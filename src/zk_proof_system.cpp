#include "neuropet/zk_proof_system.hpp"
#include <cstring>
#if defined(__unix__)
#include <dlfcn.h>
#elif defined(_WIN32)
#include <windows.h>
#endif
#include <cstdio>
#include <cstdlib>

namespace {}

namespace neuropet {

const ZkProofSystem& ZkProofSystem::instance() {
    static ZkProofSystem inst;
    return inst;
}

// -------------------------------------------------------------------------
// ZkProofSystem
// -------------------------------------------------------------------------

/**
 * Construct the proof system by dynamically loading the prover library.
 *
 * The library name can be overridden via the ``ZK_PROVER_PATH`` environment
 * variable. The loader tries both the provided path and the local directory in
 * order to support running the tests without installing the library system
 * wide. If the library or required entry points cannot be found the process is
 * aborted as the proof system cannot operate without them.
 */
ZkProofSystem::ZkProofSystem() {
#if defined(__APPLE__)
    const char* path = std::getenv("ZK_PROVER_PATH");
    if (!path)
        path = "libstark_prover.dylib";
    handle_ = dlopen(path, RTLD_LAZY);
    if (!handle_ && std::strchr(path, '/') == nullptr) {
        std::string local = std::string("./") + path;
        handle_ = dlopen(local.c_str(), RTLD_LAZY);
    }
    if (handle_) {
        gen_ = reinterpret_cast<GenerateFn>(dlsym(handle_, "zk_generate_proof"));
        verify_ = reinterpret_cast<VerifyFn>(dlsym(handle_, "zk_verify_proof"));
    }
#elif defined(__unix__)
    const char* path = std::getenv("ZK_PROVER_PATH");
    if (!path)
        path = "libstark_prover.so";
    handle_ = dlopen(path, RTLD_LAZY);
    if (!handle_ && std::strchr(path, '/') == nullptr) {
        std::string local = std::string("./") + path;
        handle_ = dlopen(local.c_str(), RTLD_LAZY);
    }
    if (handle_) {
        gen_ = reinterpret_cast<GenerateFn>(dlsym(handle_, "zk_generate_proof"));
        verify_ = reinterpret_cast<VerifyFn>(dlsym(handle_, "zk_verify_proof"));
    }
#elif defined(_WIN32)
    const char* path = std::getenv("ZK_PROVER_PATH");
    if (!path)
        path = "stark_prover.dll";
    handle_ = LoadLibraryA(path);
    if (!handle_ && std::strchr(path, '\\') == nullptr) {
        std::string local = std::string(".\\") + path;
        handle_ = LoadLibraryA(local.c_str());
    }
    if (handle_) {
        gen_ = reinterpret_cast<GenerateFn>(
            GetProcAddress(static_cast<HMODULE>(handle_), "zk_generate_proof"));
        verify_ = reinterpret_cast<VerifyFn>(
            GetProcAddress(static_cast<HMODULE>(handle_), "zk_verify_proof"));
    }
#endif
    if (!gen_ || !verify_) {
        std::fprintf(
            stderr,
            "Failed to load STARK prover. Set ZK_PROVER_PATH or install the prover library\n");
        std::abort();
    }
}

ZkProofSystem::~ZkProofSystem() {
#if defined(__unix__)
    if (handle_)
        dlclose(handle_);
#elif defined(_WIN32)
    if (handle_)
        FreeLibrary(static_cast<HMODULE>(handle_));
#endif
}

/**
 * @brief Collate tensors and invoke the external prover to produce a proof.
 *
 * The tensors are flattened into a single contiguous buffer before calling into
 * the dynamically loaded prover. Loss information is appended using
 * ``compute_loss`` so that verifiers can compare it against the original
 * inputs.
 */
StarkProof ZkProofSystem::generate_proof(const std::vector<std::vector<int8_t>>& tensors) const {
    std::vector<int8_t> buf;
    for (const auto& t : tensors) {
        buf.insert(buf.end(), t.begin(), t.end());
    }
    zk_proof_raw raw{};
    gen_(buf.data(), buf.size(), &raw);
    StarkProof p{};
    p.root = std::string(raw.root);
    p.proof = std::string(raw.proof);
    p.loss = compute_loss(tensors);
    return p;
}

/**
 * @brief Verify a proof against a sequence of tensors.
 *
 * The input tensors are flattened using the same layout as in
 * ``generate_proof`` before being passed to the verifier entry point. The proof
 * strings are copied into the raw structure expected by the C API.
 */
bool ZkProofSystem::verify_proof(const std::vector<std::vector<int8_t>>& tensors,
                                 const StarkProof& proof) const {
    std::vector<int8_t> buf;
    for (const auto& t : tensors) {
        buf.insert(buf.end(), t.begin(), t.end());
    }
    zk_proof_raw raw{};
    std::snprintf(raw.root, sizeof(raw.root), "%s", proof.root.c_str());
    std::snprintf(raw.proof, sizeof(raw.proof), "%s", proof.proof.c_str());
    if (!verify_(buf.data(), buf.size(), &raw))
        return false;
    return compute_loss(tensors) == proof.loss;
}

} // namespace neuropet
