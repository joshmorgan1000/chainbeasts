#pragma once

#include "neuropet/eth_encoding.hpp"
#include "neuropet/http_client.hpp"
#include <sstream>
#include <string>
#include <vector>

namespace neuropet {

/** Lightweight client for the CurriculumDuel contract. */
class OnchainCurriculumDuel {
  public:
    OnchainCurriculumDuel(const std::string& host, unsigned short port, const std::string& contract)
        : host_{host}, port_{port}, contract_{contract} {}

    /**
     * Query the revealed dataset bytes for a duel.
     *
     * @param duel_id Duel identifier.
     * @param out_hex Hex string of the dataset (no 0x prefix).
     * @return True on success.
     */
    bool get_dataset(std::uint64_t duel_id, std::string& out_hex) const {
#if defined(_WIN32) || defined(__unix__)
        // selector = keccak256("datasets(uint256)")[0:4]
        std::string data = "0xf968825d";
        data += encode_uint256(duel_id);

        std::ostringstream body;
        body << "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",";
        body << "\"params\":[{\"to\":\"" << contract_ << "\",\"data\":\"" << data
             << "\"},\"latest\"],\"id\":1}";
        std::string resp;
        if (!http_post_json(host_, port_, body.str(), &resp))
            return false;
        auto pos = resp.find("\"result\"");
        if (pos == std::string::npos)
            return false;
        pos = resp.find("0x", pos);
        if (pos == std::string::npos)
            return false;
        pos += 2;
        auto end = resp.find('"', pos);
        if (end == std::string::npos)
            return false;
        out_hex = resp.substr(pos, end - pos);
        return true;
#else
        (void)duel_id;
        (void)out_hex;
        return false;
#endif
    }

  private:
    std::string host_{};
    unsigned short port_{};
    std::string contract_{};
};

/** Decode a hex string into raw bytes. */
inline std::vector<std::uint8_t> decode_hex(const std::string& hex) {
    std::string h = strip_0x(hex);
    std::vector<std::uint8_t> out(h.size() / 2);
    for (std::size_t i = 0; i + 1 < h.size(); i += 2)
        out[i / 2] = static_cast<std::uint8_t>(std::stoi(h.substr(i, 2), nullptr, 16));
    return out;
}

} // namespace neuropet
