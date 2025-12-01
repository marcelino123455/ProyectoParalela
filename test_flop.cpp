#include <chrono>
#include <iostream>

int main() {
    const long N = 1e9;
    double a = 1.0, b = 2.0, c = 0.0;

    auto start = std::chrono::high_resolution_clock::now();
    for (long i = 0; i < N; i++) {
        c = a + b;
    }
    auto end = std::chrono::high_resolution_clock::now();

    double seconds = std::chrono::duration<double>(end - start).count();

    std::cout << "Tiempo total: " << seconds << " s\n";
    std::cout << "Tiempo por FLOP: " << seconds / N << " s\n";
}
