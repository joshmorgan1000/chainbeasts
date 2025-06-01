#pragma once

#include "neuropet/eth_encoding.hpp"
#include "neuropet/validator.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "neuropet/http_client.hpp"

namespace neuropet {

/** Simple client sending proofs to the on-chain ProofVerifier contract. */
class OnchainProofVerifier {
  public:
    OnchainProofVerifier(const std::string& host, unsigned short port, const std::string& contract)
        : host_{host}, port_{port}, contract_{contract} {}

    /** Submit a STARK proof to the ProofVerifier contract. */
    bool submit_proof(const StarkProof& proof) const { return send_tx(proof.root, proof.proof); }

    /** Submit a STARK proof asynchronously. */
    std::future<bool> submit_proof_async(const StarkProof& proof) const {
        return std::async(std::launch::async,
                          [this, proof]() { return send_tx(proof.root, proof.proof); });
    }

    /** Query whether a proof root has been verified on-chain. */
    bool verify(const std::string& root) const { return call_verify(root); }

    /** Asynchronously query proof verification. */
    std::future<bool> verify_async(const std::string& root) const {
        return std::async(std::launch::async, [this, root]() { return call_verify(root); });
    }

  private:
    // Encode a submit_proof transaction and send it via JSON-RPC.
    bool send_tx(const std::string& root, const std::string& proof) const {
#if defined(_WIN32) || defined(__unix__)
        std::string data = "0xb76df398";
        data += encode_bytes32(root);
        data += encode_uint256(0x40);
        data += encode_bytes(proof);

        std::ostringstream body;
        body << "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",";
        body << "\"params\":[{\"to\":\"" << contract_ << "\",\"data\":\"" << data
             << "\"}],\"id\":1}";
        if (!http_post_json(host_, port_, body.str())) {
            std::cerr << "OnchainProofVerifier: failed to submit proof" << std::endl;
            return false;
        }
        return true;
#else
        (void)root;
        (void)proof;
        return false;
#endif
    }

    // Query the on-chain contract to check if a proof root is finalized.
    bool call_verify(const std::string& root) const {
#if defined(_WIN32) || defined(__unix__)
        std::string data = "0x256aa6a0";
        data += encode_bytes32(root);

        std::ostringstream body;
        body << "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",";
        body << "\"params\":[{\"to\":\"" << contract_ << "\",\"data\":\"" << data
             << "\"},\"latest\"],\"id\":1}";
        std::string resp;
        if (!http_post_json(host_, port_, body.str(), &resp)) {
            std::cerr << "OnchainProofVerifier: verify request failed" << std::endl;
            return false;
        }
        auto pos = resp.find("\"result\"");
        if (pos == std::string::npos) {
            std::cerr << "OnchainProofVerifier: malformed response" << std::endl;
            return false;
        }
        pos = resp.find("0x", pos);
        if (pos == std::string::npos || resp.size() < pos + 66) {
            std::cerr << "OnchainProofVerifier: invalid result field" << std::endl;
            return false;
        }
        std::string hex = resp.substr(pos + 2, 64);
        return !hex.empty() && hex.back() == '1';
#else
        (void)root;
        return false;
#endif
    }

    std::string host_{};
    unsigned short port_{};
    std::string contract_{};
};

} // namespace neuropet
