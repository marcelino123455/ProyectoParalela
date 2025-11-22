#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <numeric>

// Configuration
// const long TOTAL_NUMBERS = 70000000; // 70 Million
const long TOTAL_NUMBERS = 20000000; // 70 Million

void parallel_quicksort(std::vector<int>& local_data, int rank, int size) {
    // 1. Initial local sort (Hyperquicksort optimization)
    std::sort(local_data.begin(), local_data.end());

    // Calculate number of recursion steps (dimensions of the hypercube)
    int steps = log2(size);

    for (int i = 0; i < steps; i++) {
        // Determine process groups and partners based on bit manipulation
        // In step i, we split based on the (steps - 1 - i)-th bit
        int mask = 1 << (steps - 1 - i);
        int partner = rank ^ mask;
        
        // 2. Pivot Selection
        // For simplicity, we pick the median of one processor in the group to be the pivot
        // The 'root' of the current sub-group broadcasts the pivot.
        // A sub-group is defined by processes sharing the same bits above 'mask'
        
        int pivot = 0;
        // The lowest rank in the pair group picks the pivot
        int group_root = rank & ~(mask | (mask - 1)); 
        
        // To balance load better, we could pick median of medians, 
        // but here we broadcast the median of the group_root's current data.
        if (rank == group_root) {
            if (!local_data.empty()) {
                pivot = local_data[local_data.size() / 2];
            } else {
                pivot = 0; // Edge case handling
            }
        }
        
        // Broadcast pivot to all in the current sub-communicator logic
        // Since we don't create actual sub-comms, we must rely on tags or careful explicit sends.
        // However, standard approach: simpler to just Bcast within the pair logic? 
        // Actually, all procs in the "color" group need the SAME pivot.
        // We will use MPI_Bcast with a constructed communicator for clarity or standard sends.
        // Optimization: Just have every pair agree on a pivot. 
        // BETTER: One proc broadcasts to the whole world? No, that ruins recursion.
        // STANDARD HYPERCUBE:
        // We need a pivot common to the specific sub-cube.
        
        // Let's use a simpler Global Broadcast strategy for the Pivot of the specific group.
        // We can use MPI_Comm_split to create the subgroups physically.
        
        MPI_Comm sub_comm;
        int color = rank / (1 << (steps - i)); // Group ID
        MPI_Comm_split(MPI_COMM_WORLD, color, rank, &sub_comm);
        
        int sub_rank;
        MPI_Comm_rank(sub_comm, &sub_rank);
        
        if (sub_rank == 0) {
             if (!local_data.empty()) pivot = local_data[local_data.size() / 2];
        }
        MPI_Bcast(&pivot, 1, MPI_INT, 0, sub_comm);
        MPI_Comm_free(&sub_comm);

        // 3. Partition Data
        // Find split point in sorted array
        auto split_it = std::lower_bound(local_data.begin(), local_data.end(), pivot);
        
        std::vector<int> keep_data;
        std::vector<int> send_data;

        // Determine if I am in the "Low" group or "High" group relative to my partner
        if ((rank & mask) == 0) {
            // I am "Low": Keep smaller values, send larger values
            // Since local_data is sorted:
            // [begin ... split_it) are < pivot (Keep)
            // [split_it ... end) are >= pivot (Send)
            
            keep_data.assign(local_data.begin(), split_it);
            send_data.assign(split_it, local_data.end());
        } else {
            // I am "High": Keep larger values, send smaller values
            
            send_data.assign(local_data.begin(), split_it);
            keep_data.assign(split_it, local_data.end());
        }

        // 4. Exchange Data
        int send_count = send_data.size();
        int recv_count = 0;

        // Exchange sizes first
        MPI_Sendrecv(&send_count, 1, MPI_INT, partner, 0,
                     &recv_count, 1, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> recv_data(recv_count);

        // Exchange actual data
        MPI_Sendrecv(send_data.data(), send_count, MPI_INT, partner, 1,
                     recv_data.data(), recv_count, MPI_INT, partner, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 5. Merge Data
        // Since we kept a sorted chunk and received a sorted chunk (Hyperquicksort property),
        // we can merge them efficiently.
        local_data.clear();
        local_data.resize(keep_data.size() + recv_data.size());
        std::merge(keep_data.begin(), keep_data.end(),
                   recv_data.begin(), recv_data.end(),
                   local_data.begin());
                   
        // local_data is now sorted for the next step
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check for power of 2
    if ((size & (size - 1)) != 0) {
        if (rank == 0) std::cerr << "Error: Number of processes must be a power of 2." << std::endl;
        MPI_Finalize();
        return 1;
    }

    std::vector<int> local_data;
    double start_time;

    // --- 1. Data Generation ---
    if (rank == 0) {
        std::cout << "Generating " << TOTAL_NUMBERS << " random numbers..." << std::endl;
        std::vector<int> all_data(TOTAL_NUMBERS);
        srand(42);
        for (auto& x : all_data) x = rand();

        start_time = MPI_Wtime();

        // Handle initial scatter (splitting variable sizes if N % P != 0)
        int base_count = TOTAL_NUMBERS / size;
        int remainder = TOTAL_NUMBERS % size;

        std::vector<int> send_counts(size);
        std::vector<int> displs(size);
        int current_disp = 0;
        
        for (int i = 0; i < size; i++) {
            send_counts[i] = base_count + (i < remainder ? 1 : 0);
            displs[i] = current_disp;
            current_disp += send_counts[i];
        }

        local_data.resize(send_counts[0]);
        
        // Scatterv is needed because chunks might not be identical size
        MPI_Scatterv(all_data.data(), send_counts.data(), displs.data(), MPI_INT,
                     local_data.data(), send_counts[0], MPI_INT, 0, MPI_COMM_WORLD);
    } 
    else {
        int base_count = TOTAL_NUMBERS / size;
        int remainder = TOTAL_NUMBERS % size;
        int my_count = base_count + (rank < remainder ? 1 : 0);
        local_data.resize(my_count);
        
        MPI_Scatterv(NULL, NULL, NULL, MPI_INT,
                     local_data.data(), my_count, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // --- 2. Parallel Sorting ---
    parallel_quicksort(local_data, rank, size);

    // --- 3. Gathering Results ---
    // Since final array sizes vary after sorting, we need GatherV
    int local_count = local_data.size();
    std::vector<int> recv_counts(size);
    
    MPI_Gather(&local_count, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> final_data;
    std::vector<int> displs(size);

    if (rank == 0) {
        final_data.resize(TOTAL_NUMBERS);
        int current_disp = 0;
        for (int i = 0; i < size; i++) {
            displs[i] = current_disp;
            current_disp += recv_counts[i];
        }
    }

    MPI_Gatherv(local_data.data(), local_count, MPI_INT,
                final_data.data(), recv_counts.data(), displs.data(), MPI_INT,
                0, MPI_COMM_WORLD);

    // --- 4. Verification ---
    if (rank == 0) {
        double end_time = MPI_Wtime();
        std::cout << "Sorting completed in " << (end_time - start_time) << " seconds." << std::endl;
        
        bool sorted = true;
        long count = final_data.size();
        if (count != TOTAL_NUMBERS) {
            std::cout << "Error: Count mismatch! Expected " << TOTAL_NUMBERS << ", got " << count << std::endl;
            sorted = false;
        } else {
            std::cout<<"Existe un total de "<< count<<" datos\n"; 
            for (long i = 1; i < count; i++) {
                if (final_data[i-1] > final_data[i]) {
                    sorted = false;
                    std::cout << "Error: Data not sorted at index " << i << std::endl;
                    break;
                }
            }
        }

        if (sorted) {
            std::cout << "SUCCESS: " <<TOTAL_NUMBERS<<" Million numbers sorted correctly." << std::endl;
        } else {
            std::cout << "FAILURE: Sorting failed." << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}