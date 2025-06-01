#include "neuropet/battle.hpp"
#include <gtest/gtest.h>

TEST(BattleItemTest, AttachmentsAffectOutcome) {
    neuropet::CreatureStats a{1, 1, 1, 1};
    neuropet::CreatureStats b{2, 2, 2, 2};
    // Item significantly boosts power, defense and stamina
    neuropet::Item boost{1, 3, 3, 3};
    a.attach_item(boost);
    neuropet::BattleEngine engine;
    // Without item B would win due to higher power and defense
    std::uint32_t winner = engine.fight(a, b, 1);
    EXPECT_EQ(winner, 1u);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
