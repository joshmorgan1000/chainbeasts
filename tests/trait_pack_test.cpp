#include "neuropet/proof_system.hpp"
#include "neuropet/traits.hpp"
#include <gtest/gtest.h>

TEST(TraitPack, RoundTrip) {
    neuropet::VisualTraits t{};
    t.body_shape = 3;
    t.body_colour = {1, 2, 3};
    t.sensor_count = 2;
    t.sensor_shape[0] = 4;
    t.sensor_shape[1] = 5;
    t.sensor_colour[0] = {4, 5, 6};
    t.sensor_colour[1] = {6, 7, 8};
    t.appendage_count = 1;
    t.appendage_shape[0] = 2;
    t.appendage_colour[0] = {8, 9, 10};

    unsigned __int128 packed = neuropet::pack_traits(t);
    auto u = neuropet::unpack_traits(packed, t.sensor_count, t.appendage_count);
    EXPECT_EQ(u.body_shape, t.body_shape);
    EXPECT_EQ(u.body_colour.h, t.body_colour.h);
    EXPECT_EQ(u.body_colour.s, t.body_colour.s);
    EXPECT_EQ(u.body_colour.v, t.body_colour.v);
    for (unsigned i = 0; i < t.sensor_count; ++i) {
        EXPECT_EQ(u.sensor_shape[i], t.sensor_shape[i]);
        EXPECT_EQ(u.sensor_colour[i].h, t.sensor_colour[i].h);
        EXPECT_EQ(u.sensor_colour[i].s, t.sensor_colour[i].s);
        EXPECT_EQ(u.sensor_colour[i].v, t.sensor_colour[i].v);
    }
    for (unsigned i = 0; i < t.appendage_count; ++i) {
        EXPECT_EQ(u.appendage_shape[i], t.appendage_shape[i]);
        EXPECT_EQ(u.appendage_colour[i].h, t.appendage_colour[i].h);
        EXPECT_EQ(u.appendage_colour[i].s, t.appendage_colour[i].s);
        EXPECT_EQ(u.appendage_colour[i].v, t.appendage_colour[i].v);
    }
}

TEST(TraitPack, NameHash) {
    auto digest = neuropet::trait_name_hash("FlareDrake");
    std::string hex = neuropet::to_hex(digest.data(), digest.size());
    EXPECT_EQ(hex, "19df2a7272ef22a6e7f5f05eefd555ab5ed934a44a5d53cd31e75709ac7cb011");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
