#include "mpi.h"
#include<stdio.h>
#include <stdlib.h>
#include "math.h"
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
// #define SIZE 1000000
// #define SIZE 18

/*
    Divides the array given into two partitions
        - Lower than pivot
        - Higher than pivot
    and returns the Pivot index in the array
*/
int partition(int *arr, int low, int high){
    int pivot = arr[high];
    int i = (low - 1);
    int j,temp;
    for (j=low;j<=high-1;j++){
	if(arr[j] < pivot){
	     i++;
             temp=arr[i];  
             arr[i]=arr[j];
             arr[j]=temp;	
	}
    }
    temp=arr[i+1];  
    arr[i+1]=arr[high];
    arr[high]=temp; 
    return (i+1);
}

/*
    Hoare Partition - Starting pivot is the middle point
    Divides the array given into two partitions
        - Lower than pivot
        - Higher than pivot
    and returns the Pivot index in the array
*/
int hoare_partition(int *arr, int low, int high){
    int middle = floor((low+high)/2);
    int pivot = arr[middle];
    int j,temp;
    // move pivot to the end
    temp=arr[middle];  
    arr[middle]=arr[high];
    arr[high]=temp;

    int i = (low - 1);
    for (j=low;j<=high-1;j++){
        if(arr[j] < pivot){
            i++;
            temp=arr[i];  
            arr[i]=arr[j];
            arr[j]=temp;	
        }
    }
    // move pivot back
    temp=arr[i+1];  
    arr[i+1]=arr[high];
    arr[high]=temp; 

    return (i+1);
}

/*
    Simple sequential Quicksort Algorithm
*/
void quicksort(int *number,int first,int last){
    if(first<last){
        int pivot_index = partition(number, first, last);
        quicksort(number,first,pivot_index-1);
        quicksort(number,pivot_index+1,last);
    }
}

/*
    Functions that handles the sharing of subarrays to the right clusters
*/
int quicksort_recursive(int* arr, int arrSize, int currProcRank, int maxRank, int rankIndex) {
    MPI_Status status;

    // Calculate the rank of the Cluster which I'll send the other half
    int shareProc = currProcRank + pow(2, rankIndex);
    // Move to lower layer in the tree
    rankIndex++;

    // If no Cluster is available, sort sequentially by yourself and return
    if (shareProc > maxRank) {
        MPI_Barrier(MPI_COMM_WORLD);
	    quicksort(arr, 0, arrSize-1 );
        return 0;
    }
    // Divide array in two parts with the pivot in between
    int j = 0;
    int pivotIndex;
    pivotIndex = hoare_partition(arr, j, arrSize-1 );

    // Send partition based on size(always send the smaller part), 
    // Sort the remaining partitions,
    // Receive sorted partition
    if (pivotIndex <= arrSize - pivotIndex) {
        MPI_Send(arr, pivotIndex , MPI_INT, shareProc, pivotIndex, MPI_COMM_WORLD);
	    quicksort_recursive((arr + pivotIndex+1), (arrSize - pivotIndex-1 ), currProcRank, maxRank, rankIndex); 
        MPI_Recv(arr, pivotIndex , MPI_INT, shareProc, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
    else {
        MPI_Send((arr + pivotIndex+1), arrSize - pivotIndex-1, MPI_INT, shareProc, pivotIndex + 1, MPI_COMM_WORLD);
        quicksort_recursive(arr, (pivotIndex), currProcRank, maxRank, rankIndex);
        MPI_Recv((arr + pivotIndex+1), arrSize - pivotIndex-1, MPI_INT, shareProc, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
}

int* readData(const char* path, int outSize) {
    FILE* file = fopen(path, "r");
    if (!file) {
        printf("Error al abrir %s\n", path);
        return NULL;
    }

    int* arr = malloc(sizeof(int) * outSize);  
    if (!arr) {
        printf("Error al asignar memoria\n");
        fclose(file);
        return NULL;
    }
    char c;
    int i = 0;

    printf("El array tiene:\n");
    while (i < outSize && fscanf(file, " %c", &c) == 1) {
        arr[i] = (int)c;  
        printf("%d ", arr[i]);
        i++;
    }

    printf("\nLeídos: %d/%d\n", i, outSize);

    fclose(file);
    return arr;
}

void ensure_outdir_for_size(int nprocs) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p ../tiempos/quicksort_mpi/%d", nprocs);
    system(cmd);
}

int file_exists(const char* path) {
    return access(path, F_OK) == 0;
}

bool check_flag(int argc, char *argv[], const char* flag) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], flag) == 0) {
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    double start_timer =MPI_Wtime();
    double ini_start =MPI_Wtime();
    int SIZE = atoi(argv[1]);
    int N = SIZE;
    char* N_path = argv[2];
    bool CHECK = check_flag(argc, argv, "--check");

    // int unsorted_array[SIZE]; 
    int *unsorted_array = NULL;
    int array_size = SIZE;
    int size, rank;
    // Start Parallel Execution
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank==0){
        unsorted_array = readData(N_path, SIZE);
	}
    double ini_end =MPI_Wtime();
    double inicializacion =ini_end - ini_start; 

    double tiempo_computo = 0.0;
    double tiempo_comunicacion = 0.0;

    // Calculate in which layer of the tree each Cluster belongs
    int rankPower = 0;
    while (pow(2, rankPower) <= rank){
        rankPower++;
    }
    // Wait for all clusters to reach this point 
    MPI_Barrier(MPI_COMM_WORLD);
    double finish_timer;
    if (rank == 0) {
	    // start_timer = MPI_Wtime();
        // Cluster Zero(Master) starts the Execution and
        // always runs recursively and keeps the left bigger half
        quicksort_recursive(unsorted_array, array_size, rank, size - 1, rankPower);    
    }else{ 
        // All other Clusters wait for their subarray to arrive,
        // they sort it and they send it back.
        MPI_Status status;
        int subarray_size;
        
        double t0 = MPI_Wtime();
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        double t1 = MPI_Wtime();
        
        // Capturing size of the array to receive
        t0 = MPI_Wtime();
        MPI_Get_count(&status, MPI_INT, &subarray_size);
        t1 = MPI_Wtime();
        tiempo_comunicacion += (t1 - t0);

	    int source_process = status.MPI_SOURCE;     
        // int subarray[subarray_size];
        int *subarray = (int*) malloc(subarray_size * sizeof(int));
        
        t0 = MPI_Wtime();
        MPI_Recv(subarray, subarray_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        t1 = MPI_Wtime();
        tiempo_comunicacion += (t1 - t0);
        
        quicksort_recursive(subarray, subarray_size, rank, size - 1, rankPower);
        
        t0 = MPI_Wtime();
        MPI_Send(subarray, subarray_size, MPI_INT, source_process, 0, MPI_COMM_WORLD);
        t1 = MPI_Wtime();
        tiempo_comunicacion += (t1 - t0);

    };

    if (rank == 0) {
        finish_timer = MPI_Wtime();
        double tiempo_total = finish_timer - start_timer; 
        double tiempo_computo = tiempo_total - (tiempo_comunicacion + inicializacion);

        ensure_outdir_for_size(size);
        char outfile[256];
        snprintf(outfile, sizeof(outfile), "../tiempos/quicksort_mpi/%d/tiempos.csv", size);


        int exists = file_exists(outfile);
        FILE *fout = fopen(outfile, "a");
        if (!fout) {
            fprintf(stderr, "No se pudo abrir %s para append: %s\n", outfile, strerror(errno));
        } else {
            if (!exists) {
                fprintf(fout, "Step,Computo,Comunicacion,N\n");
            }
            // 0: computo separado, 1: total, 2: inicializacion
            fprintf(fout, "0,%.8f,%.8f,%d\n", tiempo_computo, tiempo_comunicacion, SIZE);; // Medición de tiempos separadp
            fprintf(fout, "1,%.8f,0.00000000,%d\n", tiempo_total, SIZE);// Tiempo total, la columna de comunicación no cuenta
            fprintf(fout, "2,%.8f,0.00000000,%d\n", inicializacion, SIZE);// Tiempo de inicialización, la columna de comunicación no cuenta
            fclose(fout);
            printf("Archivo de tiempos guardado en: %s\n", outfile);
        }
    }
    
    if (CHECK){
        if(rank==0){
            finish_timer = MPI_Wtime();
            printf("Total time for %d Clusters : %2.2f sec \n",size, finish_timer-start_timer);

            // --- VALIDATION CHECK ---
            printf("Checking.. \n");
            bool error = false;
            int i=0;
            for(i=0;i<SIZE-1;i++) { 
                if (unsorted_array[i] > unsorted_array[i+1]){
                    error = true;
                    printf("error in i=%d \n", i);
                }
            }
            if(error)
                printf("Error..Not sorted correctly\n");
            else
                printf("Correct!\n");        
        }
    }
       
    MPI_Finalize();
    // End of Parallel Execution
    return 0;
}