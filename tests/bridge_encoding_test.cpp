#include "neuropet/eth_encoding.hpp"
#include <gtest/gtest.h>

namespace {
std::string encode_address(const std::string& addr) {
    std::string h = neuropet::strip_0x(addr);
    return std::string(64 - h.size(), '0') + h;
}
} // namespace

TEST(BridgeEncodingTest, EncodePrimitives) {
    EXPECT_EQ(neuropet::encode_uint32(1),
              "0000000000000000000000000000000000000000000000000000000000000001");
    EXPECT_EQ(neuropet::encode_int32(-1),
              "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    EXPECT_EQ(neuropet::encode_bytes32("0x12ab"),
              "00000000000000000000000000000000000000000000000000000000000012ab");
    EXPECT_EQ(neuropet::encode_uint256(42),
              "000000000000000000000000000000000000000000000000000000000000002a");
    EXPECT_EQ(neuropet::encode_bool(true),
              "0000000000000000000000000000000000000000000000000000000000000001");
    EXPECT_EQ(neuropet::encode_bool(false),
              "0000000000000000000000000000000000000000000000000000000000000000");
    EXPECT_EQ(neuropet::encode_bytes("deadbeef"),
              "0000000000000000000000000000000000000000000000000000000000000004"
              "deadbeef00000000000000000000000000000000000000000000000000000000");
}

TEST(BridgeEncodingTest, BridgeOutTx) {
    std::string tx = "0x3dc83f6e";
    tx += neuropet::encode_uint256(1);
    tx += neuropet::encode_uint256(2);
    EXPECT_EQ(tx, "0x3dc83f6e"
                  "0000000000000000000000000000000000000000000000000000000000000001"
                  "0000000000000000000000000000000000000000000000000000000000000002");
}

TEST(BridgeEncodingTest, BridgeInTx) {
    std::string proof;
    proof += neuropet::encode_uint256(1);
    proof += encode_address("0xabc");
    proof += neuropet::encode_uint256(2);
    proof += neuropet::encode_bytes("deadbeef");
    proof += neuropet::encode_bytes32("1234");

    std::string tx = "0x4b42d211";
    tx += neuropet::encode_uint256(1);
    tx += encode_address("0xabc");
    tx += neuropet::encode_uint256(1);
    tx += neuropet::encode_uint256(2);
    tx += neuropet::encode_bytes("deadbeef");
    tx += neuropet::encode_bytes32("1234");
    tx += neuropet::encode_bytes(proof);
    EXPECT_EQ(tx, "0x4b42d211"
                  "0000000000000000000000000000000000000000000000000000000000000001"
                  "0000000000000000000000000000000000000000000000000000000000000abc"
                  "0000000000000000000000000000000000000000000000000000000000000001"
                  "0000000000000000000000000000000000000000000000000000000000000002"
                  "0000000000000000000000000000000000000000000000000000000000000004"
                  "deadbeef00000000000000000000000000000000000000000000000000000000"
                  "0000000000000000000000000000000000000000000000000000000000001234"
                  "00000000000000000000000000000000000000000000000000000000000000c0"
                  "0000000000000000000000000000000000000000000000000000000000000001"
                  "0000000000000000000000000000000000000000000000000000000000000abc"
                  "0000000000000000000000000000000000000000000000000000000000000002"
                  "0000000000000000000000000000000000000000000000000000000000000004"
                  "deadbeef00000000000000000000000000000000000000000000000000000000"
                  "0000000000000000000000000000000000000000000000000000000000001234");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
