#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#if HARMONICS_HAS_VULKAN
#include <gpu/GlobalFunctionRegistry.hpp>
#include <variant>
#endif
#include <harmonics/gpu_backend.hpp>
#include <vector>

#if defined(NEUROPET_USE_AVX2)
#include <immintrin.h>
#elif defined(NEUROPET_USE_NEON)
#include <arm_neon.h>
#endif

namespace neuropet {

#if defined(NEUROPET_USE_AVX2) || defined(NEUROPET_USE_NEON)
constexpr std::size_t INT8_TILE = 128;
#else
constexpr std::size_t INT8_TILE = 64;
#endif

#if defined(NEUROPET_USE_AVX2)
inline void int8_matmul_avx2_tile(const int8_t* A, const int8_t* B, int8_t* C, std::size_t N,
                                  std::size_t K, std::vector<int32_t>& acc) {
    std::fill(acc.begin(), acc.end(), 0);
    for (std::size_t t = 0; t < K; ++t) {
        __m256i a16 = _mm256_set1_epi16(static_cast<int16_t>(A[t]));
        std::size_t j = 0;
        for (; j + 15 < N; j += 16) {
            __m128i b8 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(B + t * N + j));
            __m256i b16 = _mm256_cvtepi8_epi16(b8);
            __m256i prod = _mm256_mullo_epi16(a16, b16);
            __m256i prod_lo = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(prod));
            __m256i prod_hi = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(prod, 1));
            __m256i acc_lo = _mm256_loadu_si256(reinterpret_cast<__m256i*>(acc.data() + j));
            __m256i acc_hi = _mm256_loadu_si256(reinterpret_cast<__m256i*>(acc.data() + j + 8));
            acc_lo = _mm256_add_epi32(acc_lo, prod_lo);
            acc_hi = _mm256_add_epi32(acc_hi, prod_hi);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(acc.data() + j), acc_lo);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(acc.data() + j + 8), acc_hi);
        }
        for (; j < N; ++j)
            acc[j] += static_cast<int32_t>(A[t]) * static_cast<int32_t>(B[t * N + j]);
    }
    for (std::size_t j = 0; j < N; ++j)
        C[j] = static_cast<int8_t>(std::max(-128, std::min(127, acc[j])));
}
#elif defined(NEUROPET_USE_NEON)
inline void int8_matmul_neon_tile(const int8_t* A, const int8_t* B, int8_t* C, std::size_t N,
                                  std::size_t K, std::vector<int32_t>& acc) {
    std::fill(acc.begin(), acc.end(), 0);
    for (std::size_t t = 0; t < K; ++t) {
        int8x8_t a8 = vdup_n_s8(A[t]);
        std::size_t j = 0;
        for (; j + 15 < N; j += 16) {
            int8x16_t b8 = vld1q_s8(B + t * N + j);
            int16x8_t prod0 = vmull_s8(vget_low_s8(b8), a8);
            int16x8_t prod1 = vmull_s8(vget_high_s8(b8), a8);
            int32x4_t acc0 = vld1q_s32(acc.data() + j);
            int32x4_t acc1 = vld1q_s32(acc.data() + j + 4);
            int32x4_t acc2 = vld1q_s32(acc.data() + j + 8);
            int32x4_t acc3 = vld1q_s32(acc.data() + j + 12);
            acc0 = vaddq_s32(acc0, vmovl_s16(vget_low_s16(prod0)));
            acc1 = vaddq_s32(acc1, vmovl_s16(vget_high_s16(prod0)));
            acc2 = vaddq_s32(acc2, vmovl_s16(vget_low_s16(prod1)));
            acc3 = vaddq_s32(acc3, vmovl_s16(vget_high_s16(prod1)));
            vst1q_s32(acc.data() + j, acc0);
            vst1q_s32(acc.data() + j + 4, acc1);
            vst1q_s32(acc.data() + j + 8, acc2);
            vst1q_s32(acc.data() + j + 12, acc3);
        }
        for (; j < N; ++j)
            acc[j] += static_cast<int32_t>(A[t]) * static_cast<int32_t>(B[t * N + j]);
    }
    for (std::size_t j = 0; j < N; ++j)
        C[j] = static_cast<int8_t>(std::max(-128, std::min(127, acc[j])));
}
#endif

/** Simple deterministic INT8 matrix multiply.
 *  Computes C = A (MxK) * B (KxN) using saturating INT8 arithmetic.
 */
inline void int8_matmul(const std::vector<int8_t>& A, const std::vector<int8_t>& B,
                        std::vector<int8_t>& C, std::size_t M, std::size_t N, std::size_t K) {
    static thread_local std::vector<int32_t> acc;
    // Work on small tiles to improve cache locality and allow vectorized kernels
    // to reuse the same accumulation buffer across iterations.
    std::size_t tile_size = std::min<std::size_t>(INT8_TILE, N);
    if (acc.size() < tile_size)
        acc.resize(tile_size);
    C.assign(M * N, 0);
    for (std::size_t i = 0; i < M; ++i) {
        for (std::size_t jj = 0; jj < N; jj += tile_size) {
            std::size_t tile = std::min<std::size_t>(tile_size, N - jj);
            if (acc.size() < tile)
                acc.resize(tile);
#if defined(NEUROPET_USE_AVX2)
            int8_matmul_avx2_tile(A.data() + i * K, B.data() + jj, C.data() + i * N + jj, tile, K,
                                  acc);
#elif defined(NEUROPET_USE_NEON)
            int8_matmul_neon_tile(A.data() + i * K, B.data() + jj, C.data() + i * N + jj, tile, K,
                                  acc);
#else
            // Scalar fallback used when no SIMD implementation is available.
            std::fill(acc.begin(), acc.begin() + tile, 0);
            for (std::size_t t = 0; t < K; ++t) {
                for (std::size_t j = 0; j < tile; ++j)
                    acc[j] += static_cast<int32_t>(A[i * K + t]) *
                              static_cast<int32_t>(B[t * N + jj + j]);
            }
            for (std::size_t j = 0; j < tile; ++j)
                C[i * N + jj + j] = static_cast<int8_t>(std::max(-128, std::min(127, acc[j])));
#endif
        }
    }
}

/** Return true if the Harmonics GPU backend is available at runtime. */
inline bool int8_gpu_available() { return harmonics::gpu_runtime_available(); }

/**
 * Execute INT8 matmul using the Harmonics GPU backend when available.
 * Falls back to the CPU implementation otherwise.
 */
inline void int8_matmul_gpu(const std::vector<int8_t>& A, const std::vector<int8_t>& B,
                            std::vector<int8_t>& C, std::size_t M, std::size_t N, std::size_t K) {
    if (!int8_gpu_available()) {
        int8_matmul(A, B, C, M, N, K);
        return;
    }
#if HARMONICS_HAS_VULKAN
    harmonics::registerAllShaders();
    std::vector<uint8_t> a8(A.begin(), A.end());
    std::vector<uint8_t> b8(B.begin(), B.end());
    std::vector<harmonics::GPUDataVariant> params;
    params.emplace_back(a8);
    params.emplace_back(b8);
    params.emplace_back(static_cast<int32_t>(M));
    params.emplace_back(static_cast<int32_t>(N));
    params.emplace_back(static_cast<int32_t>(K));
    const auto* fn = harmonics::GPUFunctionRegistry::getInstance().get("int8_matmul");
    if (!fn) {
        int8_matmul(A, B, C, M, N, K);
        return;
    }
    harmonics::GPUDataVariant result = fn->cpuFallback(params);
    if (auto out = std::get_if<std::vector<uint8_t>>(&result)) {
        C.resize(out->size());
        for (size_t i = 0; i < out->size(); ++i)
            C[i] = static_cast<int8_t>((*out)[i]);
    } else {
        int8_matmul(A, B, C, M, N, K);
    }
#else
    int8_matmul(A, B, C, M, N, K);
#endif
}

} // namespace neuropet
