#include <iostream>
#include <mpi.h>
#include<iostream>
#include <fstream>
#include<vector>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <iomanip>
using namespace std;
namespace fs = filesystem;

// Configuraciones
bool check_flag(int argc, char *argv[], const string& flag) {
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == flag) {
            return true;
        }
    }
    return false;
}

ostream& operator<<(ostream& os, const vector<vector<int>>& mat) {
    for (int i = 0; i < mat.size(); ++i) {
        for (int j = 0; j < mat[i].size(); ++j) {

            if (i==0 or i ==1){
                if (mat[i][j] ==0){
                    os<<'0'<<' '; 
                }else{
                    os << char(mat[i][j]) << ' ';
                }
            }else{
                os << mat[i][j] << ' ';
            }
        }
        os << '\n';
    }
    return os;
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

void debug(int rank, vector<vector<int>> M, int p, string msg){
    for (int i = 0; i < p; i++) {
        if (rank == i and i ==0) {
            cout<<msg<<endl; 
        }
        if (rank == i) {
            cout<<"Rank "<<rank<<endl;
            cout<<M;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

void master_msg(int rank, string msg, bool Debugging){
    if (rank == 0 and !Debugging) {
        cout<<msg<<endl; 
    }
}


int find_i_min( vector<int>& allDataIndex) {
    int max = -1; 
    int max_i; 
    for (int i = 0; i < allDataIndex.size(); i++) {
        if(allDataIndex[i] > max) {
            max = allDataIndex[i]; 
            max_i = i; 
        }
    }
    // Ya tenemos el maximo
    int min = max;
    int min_i = -1; 
    for (int i = 0; i < allDataIndex.size(); i++) {
        if(allDataIndex[i] < min and allDataIndex[i] != -1 ) {
            min = allDataIndex[i]; 
            min_i = i; 
        }
    }
    if (min_i == -1){
        return max_i; 
    }
    allDataIndex[min_i] = -1; // ya lo usamos
    return min_i; 
}


int find_i(int k,vector<int>& allData,  vector<int>& allDataIndex){
    int min = find_i_min(allDataIndex); 
    if(min == -1){
        cerr<<"CRTICAL FAIL\n";
    } else{
        return min; 
    }
    return 999; 
}

bool are_equal(vector<int>& a, vector<int>& b){
    for (int i =0; i<a.size(); i++){
        if (a[i]!=b[i])
        {       
            return false; 
        }
    }
    return true; 
}

bool checkOrder(vector<int>& allData, vector<int>& allDataIndex) {
    vector<int> charsSorted = allData;
    
    vector<int> chars(allData.size());
    for (int i = 0; i < chars.size(); i++) {
        chars[i] = allData[find_i(i+1, allData, allDataIndex)]; 
    }
    // cout<<"Index Checking\n"; 
    // for (int i = 0; i < allDataIndex.size(); i++) {
    //     cout<<allDataIndex[i]<<", "; 
    // }

    // cout<<"\nOrder Checking\n"; 
    // for (int i = 0; i < chars.size(); i++) {
    //     cout<<(char)chars[i]<<", "; 
    // }
    // vector<int> charsSorted(allData.size());
    // copy(allData.begin(), allData.end(), charsSorted.begin());
    sort(charsSorted.begin(), charsSorted.end());  
    
    return are_equal(charsSorted, chars);
}



int main(int argc,char *argv[]){
    int rank, p;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    bool TESTING = check_flag(argc, argv, "--testing");
    bool CHECK = check_flag(argc, argv, "--check");

    vector<double> tiempos_computo(7, 0.0);
    vector<double> tiempos_comunicacion(7, 0.0);

    fs::path DIR = "../data_dispersa";
    // No paralelizable solo por automatizacion de experimentos
    for (auto path_data: listdir(DIR,TESTING)){
        fs::path name_file = "nums.txt"; 
        vector<int> data  = readData(path_data/name_file);
        if (rank ==0){
            cout<<"Trabajando con el directorio: "<<path_data;
            cout<<" con de N = "<<data.size()<<endl; 
        }
        int N = data.size(); 
        int raiz_p = sqrt(p); 
        int N_p = N/p; 

        // PASO 0: Llegar al input como en el algortimo 
        // Repartir la data solo a la primera fial de la cuadrícula de 
        // procesos
        master_msg(rank, "PASO 0: Input", TESTING);
        double t0c_start = MPI_Wtime();
        vector<vector<int>> M(4, vector<int>(N/raiz_p, 0)); 
        /*
            0 1 2
            3 4 5
            6 7 8

            0 3 6 <-
            1 4 7
            2 5 8
        */
        // Repartir la data:
        int row_0 = rank % raiz_p;
        MPI_Comm row0_comm;
        if (row_0 == 0) {
            MPI_Comm_split(MPI_COMM_WORLD, 0, rank, &row0_comm);
        } else {
            MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, rank, &row0_comm);
        }
        double t0c_end = MPI_Wtime();
        double t0comm_start = MPI_Wtime();
        if (row_0 == 0) {
            int row0_rank;
            MPI_Comm_rank(row0_comm, &row0_rank);
            MPI_Scatter(data.data(), N/raiz_p, MPI_INT, M[0].data(), N/raiz_p, MPI_INT, 0, row0_comm);
        } 

        double t0comm_end = MPI_Wtime();
        tiempos_comunicacion[0] = t0comm_end - t0comm_start;
        tiempos_computo[0] = t0c_end - t0c_start; 
        if (TESTING) debug(rank, M, p, "STEP 0"); 

        // PASO 1: Gossip
        master_msg(rank, "PASO 1: Gossip", TESTING);  
        double t1c_start = MPI_Wtime();
        // Crear los subdominios por columnas
        int row = rank%raiz_p;
        int col = rank/raiz_p;
        MPI_Comm col_comm;
        MPI_Comm_split(MPI_COMM_WORLD, col, row, &col_comm);
        int col_rank, col_size;
        MPI_Comm_rank(col_comm, &col_rank);
        MPI_Comm_size(col_comm, &col_size);
        double t1c_end = MPI_Wtime();
        
        // Gossip con brodcast
        // v2: Ahora solo es un simple brodcast
        double t1comm_start = MPI_Wtime();
        MPI_Bcast(M[0].data(), N/raiz_p, MPI_INT,0, col_comm);
        double t1comm_end = MPI_Wtime();
        if (TESTING) debug(rank, M, p,"STEP 1"); 

        tiempos_comunicacion[1] = t1comm_end - t1comm_start;
        tiempos_computo[1] =  t1c_end - t1c_start;

        // PASO 2: Broadcast
        master_msg(rank, "PASO 2: Broadcast", TESTING);  
        
        double t2c_start = MPI_Wtime();
        // Crear los subdominios por filas
        MPI_Comm row_comm;
        MPI_Comm_split(MPI_COMM_WORLD, row, col, &row_comm);
        int row_rank, row_size;
        MPI_Comm_rank(row_comm, &row_rank);
        MPI_Comm_size(row_comm, &row_size);
        
        //Copiar la fila i esima de 0 a 1 en diagonal
        if (row_rank == col_rank){
            for (int j = 0; j<N/raiz_p ; j++){
                M[1][j] = M[0][j]; 
            }
        }
        double t2c_end = MPI_Wtime();

        double t2comm_start = MPI_Wtime();
        MPI_Bcast(M[1].data(), N/raiz_p, MPI_INT, row, row_comm);
        double t2comm_end = MPI_Wtime(); 

        if (TESTING) debug(rank, M, p,"STEP 2"); 
        tiempos_comunicacion[2] = t2comm_end - t2comm_start;
        tiempos_computo[2] = t2c_end- t2c_start;


        // PASO 3: Sort de las filas 0
        master_msg(rank, "PASO 3: Sort de las filas 0", TESTING);  
        double t3c_start = MPI_Wtime();
        sort(M[0].begin(), M[0].end());
        double t3c_end = MPI_Wtime();

        tiempos_computo[3] = t3c_end - t3c_start;
        tiempos_comunicacion[3] = 0.0;
        if (TESTING) debug(rank, M, p,"STEP 3: Sort de las filas 0"); 

        // PASO 4: Local ranking de la fila 1 con respecto a la fila 0
        master_msg(rank, "PASO 4: Local ranking", TESTING);
        double t4c_start = MPI_Wtime();
        int add; 
        for (int j = 0; j < M[1].size(); ++j) {
            auto it = lower_bound(M[0].begin(), M[0].end(), M[1][j]);
            if (*it == M[1][j]){
                add = 1; 
            }else{
                add =0; 
            }
            int idx = distance(M[0].begin(), it);
            M[2][j] = idx+add;
        }
        double t4c_end = MPI_Wtime();
        tiempos_computo[4] = t4c_end - t4c_start;
        tiempos_comunicacion[4] = 0.0;
        if (TESTING) debug(rank, M, p, "STEP 4: Local Ranking");

        // PASO 5: Reducción
        master_msg(rank, "PASO 5:Reduccion", TESTING); 
        double t5comm_start = MPI_Wtime();
        MPI_Reduce(M[2].data(), M[3].data(), N/raiz_p, MPI_INT, MPI_SUM, row,row_comm );
        double t5comm_end = MPI_Wtime();
        if (TESTING) debug(rank, M, p, "STEP 5: Reduccion");
        tiempos_comunicacion[5] = t5comm_end - t5comm_start;
        tiempos_computo[5] = 0.0;

        // CHECK:
        if (CHECK){
            master_msg(rank, "CHECK: Verificar orden", TESTING);  
            vector<int> allData(N, 0);
            vector<int> allDataIndex(N, 0);

            double t6comm_start = MPI_Wtime();
            if (row_rank == col_rank and rank !=0){
                MPI_Send(M[1].data(), N/raiz_p, MPI_INT, 0, 99, MPI_COMM_WORLD ); 
                MPI_Send(M[3].data(), N/raiz_p, MPI_INT, 0, 26, MPI_COMM_WORLD ); 
                
            }
            MPI_Status status; 
            if (rank ==0){
                copy(M[1].begin(), M[1].end(), allData.begin());
                copy(M[3].begin(), M[3].end(), allDataIndex.begin());
                for (int k = 1; k < raiz_p; k++) {
                    int src = k * (raiz_p + 1);
                    int offset = k* (N / raiz_p);
                    MPI_Recv(allData.data()+offset ,N/raiz_p,MPI_INT, src, 99, MPI_COMM_WORLD, &status);

                    MPI_Recv(allDataIndex.data()+offset ,N/raiz_p,MPI_INT, src, 26, MPI_COMM_WORLD, &status);
                }
                master_msg(rank, "CHECK - RECV ok", TESTING);  

                if (TESTING){
                    cout<<"La data es la siguiente: "<<endl; 
                    for(auto char_val: allData){
                        cout<<char(char_val)<<", "; 
                    }
                    cout<<"Se tiene los siguientes indices: "<<endl; 
                    for(auto char_val: allDataIndex){
                        cout<<(char_val)<<", "; 
                    }
                }
                bool is_ordered = checkOrder(allData, allDataIndex); 
                master_msg(rank, "CHECK - Ordered ok", TESTING);  

                if (is_ordered){
                    cout<<u8"\nCHECK: 🦖 :) All oki don worry be happy"<<endl; 
                } else{
                    cerr<<u8"\nCHECK: 😞 :( Terrible lloremos"<<endl; 
                }
            }
            double t6comm_end = MPI_Wtime();
            tiempos_comunicacion[6] = t6comm_end - t6comm_start;
            tiempos_computo[6] = 0.0;
        } else{
            tiempos_comunicacion[6] = 0.0;
            tiempos_computo[6] = 0.0;
            
        }

        // Guardados de tiempos
        if (rank == 0) {
            fs::path outdir = "../tiempos/v3/" + to_string(p);
            fs::create_directories(outdir);
            fs::path outfile = outdir / "tiempos.csv";

            bool existe = fs::exists(outfile);
            ofstream fout(outfile, ios::app); 
            fout << fixed << setprecision(8);
            if (!existe) {
                fout << "Step,Computo,Comunicacion,N\n";
            }
            for (int i = 0; i < 7; i++) {
                fout << i << "," << tiempos_computo[i] << "," << tiempos_comunicacion[i]<<"," <<N<< "\n";
            }
            fout.close();
            cout << "Archivo de tiempos guardado en: " << outfile << endl;
        }


    }
    MPI_Finalize(); 


}

