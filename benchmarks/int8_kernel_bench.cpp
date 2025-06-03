#include "neuropet/int8_kernel.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    std::size_t M = 64;
    std::size_t N = 64;
    std::size_t K = 64;
    int iterations = 1000;

    if (argc > 1)
        M = static_cast<std::size_t>(std::atoi(argv[1]));
    if (argc > 2)
        N = static_cast<std::size_t>(std::atoi(argv[2]));
    if (argc > 3)
        K = static_cast<std::size_t>(std::atoi(argv[3]));
    if (argc > 4)
        iterations = std::atoi(argv[4]);

    std::vector<int8_t> A(M * K);
    std::vector<int8_t> B(K * N);
    std::vector<int8_t> C(M * N);

    for (std::size_t i = 0; i < A.size(); ++i)
        A[i] = static_cast<int8_t>(i % 7 - 3);
    for (std::size_t i = 0; i < B.size(); ++i)
        B[i] = static_cast<int8_t>((i * 3) % 5 - 2);

    neuropet::int8_matmul(A, B, C, M, N, K); // warm up

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
        neuropet::int8_matmul(A, B, C, M, N, K);
    auto end = std::chrono::high_resolution_clock::now();

    double sec = std::chrono::duration<double>(end - start).count();
    double ops = static_cast<double>(M) * N * K * iterations / sec;
    double ms_per_iter = (sec / iterations) * 1000.0;
    std::cout << "INT8 matmul " << M << "x" << N << "x" << K << "\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Total time (s): " << sec << "\n";
    std::cout << "Ops/s: " << ops << "\n";
    std::cout << "Time/iter (ms): " << ms_per_iter << std::endl;

    return 0;
}
