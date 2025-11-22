#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <chrono>

int main() {
    const long N = 20000000; // 20 millones

    std::cout << "Generando " << N << " numeros aleatorios..." << std::endl;

    std::vector<int> data;
    data.reserve(N);

    // Generar números aleatorios
    std::srand(42); // Semilla fija para reproducibilidad
    for (long i = 0; i < N; i++) {
        data.push_back(std::rand());
    }

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
