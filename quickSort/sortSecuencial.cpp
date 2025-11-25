#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <chrono>

#include <fstream>

#include <filesystem>
using namespace std;

namespace fs = filesystem;

vector<int> readData(fs::path path){
    vector<int> data; 
    ifstream file(path);
    int c;
    while (file>>c) { 
        data.push_back(c);
    }
    return data; 
}

int main(int argc, char** argv) {
    int N = std::stol(argv[1]);

    std::cout << "Leyendo " << N << " numeros..." << std::endl;
    fs::path name_file = "../data_dispersa"; 
    fs::path chars_file= "nums.txt"; 
    fs::path path_data = argv[1];
    fs::path path_final = name_file/path_data/chars_file; 
    std::vector<int> data = readData(path_final); 

    std::cout << "Iniciando sort..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    std::sort(data.begin(), data.end());
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = end - start;
    std::cout << "Tiempo total de sort: " << diff.count() << " segundos\n";


    // Guardar resultados

    fs::path outdir = "../tiempos/quickSec";
    fs::create_directories(outdir);
    fs::path outfile = outdir / "tiempos.csv";

    bool existe = fs::exists(outfile);
    ofstream fout(outfile, ios::app); 
    fout << fixed << setprecision(8);

    if (!existe) {
        fout << "Tiempo,N\n";
    }
    fout << diff.count()<< "," <<argv[1]<< "\n";
    fout.close();
        cout << "Archivo de tiempos guardado en: " << outfile << endl;



    bool sorted = true;
    for (long i = 1; i < N; i++) {
        if (data[i-1] > data[i]) {
            sorted = false;
            break;
        }
    }
    if (sorted)
        std::cout << "SUCCESS: correctamente ordenado.\n";
    else
        std::cout << "ERROR: Arreglo NO ordenado.\n";
    return 0;
}
