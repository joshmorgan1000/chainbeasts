#include "neuropet/onchain_verifier.hpp"
#include "neuropet/validator.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0]
                  << " <tensor.bin> [--submit host:port contract]" << std::endl;
        return 1;
    }

    std::string file = argv[1];
    std::ifstream in(file, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open " << file << std::endl;
        return 1;
    }

    std::vector<int8_t> tensor((std::istreambuf_iterator<char>(in)),
                               std::istreambuf_iterator<char>());
    std::vector<std::vector<int8_t>> tensors{tensor};

    neuropet::Validator validator;
    auto proof = validator.generate_stark_proof(tensors);
    std::cout << "Root:  " << proof.root << std::endl;
    std::cout << "Proof: " << proof.proof << std::endl;
    std::cout << "Loss:  " << proof.loss << std::endl;

    if (argc == 5 && std::string(argv[2]) == "--submit") {
        std::string hostport = argv[3];
        auto pos = hostport.find(':');
        std::string host = hostport.substr(0, pos);
        unsigned short port =
            static_cast<unsigned short>(std::stoi(hostport.substr(pos + 1)));
        std::string contract = argv[4];
        neuropet::OnchainProofVerifier verifier(host, port, contract);
        if (verifier.submit_proof(proof))
            std::cout << "Proof submitted" << std::endl;
        else
            std::cerr << "Failed to submit proof" << std::endl;
    }
    return 0;
}
