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
    char c;
    while (file>>c) { 
        data.push_back(int(c));
    }
    return data; 
}

int main() {
    const long N = 20000000; // 20 millones

    std::cout << "Generando " << N << " numeros aleatorios..." << std::endl;

    std::vector<int> data;
    data.reserve(N);


    std::cout << "Iniciando sort..." << std::endl;

    // Medir tiempo
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(data.begin(), data.end());
    auto end = std::chrono::high_resolution_clock::now();

    // Tiempo en segundos
    std::chrono::duration<double> diff = end - start;

    std::cout << "Tiempo total de sort: " << diff.count() << " segundos\n";

    // Verificar que quedó ordenado
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
