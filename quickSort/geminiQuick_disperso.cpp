#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <numeric>
#include <fstream>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

double t_comm = 0.0; 
double get_Time_start = 0.0; 
double get_Time_end = 0.0; 

double t_comp= 0.0; 


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
        get_Time_start = MPI_Wtime(); 
        MPI_Comm_split(MPI_COMM_WORLD, color, rank, &sub_comm);
        get_Time_end = MPI_Wtime(); 
        t_comm+= (get_Time_end -get_Time_start); 

        int sub_rank;
        MPI_Comm_rank(sub_comm, &sub_rank);
        
        if (sub_rank == 0) {
             if (!local_data.empty()) pivot = local_data[local_data.size() / 2];
        }
        get_Time_start = MPI_Wtime(); 
        MPI_Bcast(&pivot, 1, MPI_INT, 0, sub_comm);
        MPI_Comm_free(&sub_comm);
        get_Time_end = MPI_Wtime(); 
        t_comm+= (get_Time_end -get_Time_start); 
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
        
        get_Time_start = MPI_Wtime(); 
        MPI_Sendrecv(&send_count, 1, MPI_INT, partner, 0,
                     &recv_count, 1, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        get_Time_end = MPI_Wtime(); 
        t_comm+= (get_Time_end-get_Time_start); 
        
        std::vector<int> recv_data(recv_count);

        // Exchange actual data
        get_Time_start = MPI_Wtime(); 
        MPI_Sendrecv(send_data.data(), send_count, MPI_INT, partner, 1,
                     recv_data.data(), recv_count, MPI_INT, partner, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        get_Time_end = MPI_Wtime(); 
        t_comm+= (get_Time_end -get_Time_start); 
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

vector<int> readData(fs::path path){
    vector<int> data; 
    ifstream file(path);
    int c;
    while (file>>c) { 
        data.push_back(c);
    }
    return data; 
}

vector<fs::path> listdir(const fs::path& path, bool TESTING){
    vector<fs::path> files;
    fs::path N_18 = path/"18"; 
    if(TESTING){
        vector<fs::path> files_presentacion{N_18};
        return files_presentacion;   
    }
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path() !=N_18 ){
            files.push_back(entry.path()); 
        } 
    }
    sort(files.begin(), files.end());
    

    
    return files; 
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

    // Primera fila Computo y Comunicación
    // Segunda fila es tiempo total que se pondrá al vector de tiempos computo


    std::vector<int> local_data;
    double start_time;

    // fs::path DIR = "../data";
    int TOTAL_NUMBERS = std::stol(argv[1]);
    // --- 1. Reading Data ---
    if (rank == 0) {

        std::cout << "Starting to read " << TOTAL_NUMBERS << std::endl;
        
        fs::path name_file = "../data_dispersa"; 
        fs::path chars_file= "nums.txt"; 
        fs::path path_data = argv[1];
        fs::path path_final = name_file/path_data/chars_file; 
        vector<int> all_data  = readData(path_final);
        cout<<"Path: "<<path_final<<endl; 

        // std::vector<int> all_data(TOTAL_NUMBERS);
        // srand(42);
        // for (auto& x : all_data) x = rand();
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

        get_Time_start = MPI_Wtime();
        MPI_Scatterv(all_data.data(), send_counts.data(), displs.data(), MPI_INT,
                     local_data.data(), send_counts[0], MPI_INT, 0, MPI_COMM_WORLD);
        get_Time_end = MPI_Wtime(); 
        t_comm+= (get_Time_end -get_Time_start);     
    } 
    else {
        int base_count = TOTAL_NUMBERS / size;
        int remainder = TOTAL_NUMBERS % size;
        int my_count = base_count + (rank < remainder ? 1 : 0);
        local_data.resize(my_count);
        get_Time_start = MPI_Wtime();
        MPI_Scatterv(NULL, NULL, NULL, MPI_INT,
                     local_data.data(), my_count, MPI_INT, 0, MPI_COMM_WORLD);
        get_Time_end = MPI_Wtime(); 
        t_comm+= (get_Time_end - get_Time_start) ;
        
        }

    // --- 2. Parallel Sorting ---
    parallel_quicksort(local_data, rank, size);

    // --- 3. Gathering Results ---
    // Since final array sizes vary after sorting, we need GatherV
    int local_count = local_data.size();
    std::vector<int> recv_counts(size);
    
    get_Time_start = MPI_Wtime();
    MPI_Gather(&local_count, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    get_Time_end = MPI_Wtime(); 
    t_comm+= (get_Time_end -get_Time_start); 

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

    get_Time_start = MPI_Wtime(); 
    MPI_Gatherv(local_data.data(), local_count, MPI_INT,
                final_data.data(), recv_counts.data(), displs.data(), MPI_INT,
                0, MPI_COMM_WORLD);
    get_Time_end = MPI_Wtime();     
    t_comm+= (get_Time_end -get_Time_start); 

    // --- 4. Verification ---
    if (rank == 0) {
        double end_time = MPI_Wtime();
        // Save resultados
        fs::path outdir = "../tiempos/quickSort_disperso/" + to_string(size);
        fs::create_directories(outdir);
        fs::path outfile = outdir / "tiempos.csv";

        bool existe = fs::exists(outfile);
        ofstream fout(outfile, ios::app); 
        fout << fixed << setprecision(8);
        
        if (!existe) {
            fout << "Step,Computo,Comunicacion,N\n";
        }
        // for (int i = 0; i < 7; i++) {
            cout<< "Tcomm = "<<t_comm<<endl; 
            fout << 0 << "," << (end_time - start_time) -t_comm<< "," << t_comm <<"," <<argv[1]<< "\n";
            fout << 1 << "," << (end_time - start_time) << "," << 0 <<"," <<argv[1]<< "\n";
        // }
        fout.close();
            cout << "Archivo de tiempos guardado en: " << outfile << endl;





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