#include "neuropet/int8_kernel.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    constexpr std::size_t M = 64;
    constexpr std::size_t N = 64;
    constexpr std::size_t K = 64;
    constexpr int iterations = 1000;

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
    std::cout << "INT8 matmul " << M << "x" << N << "x" << K << "\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Total time (s): " << sec << "\n";
    std::cout << "Ops/s: " << ops << std::endl;

    return 0;
}
