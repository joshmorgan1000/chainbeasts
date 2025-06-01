#include "neuropet/battle.hpp"
#include <gtest/gtest.h>

TEST(BattleEngineTest, MovementWallsAndHazards) {
    neuropet::BattleEngine engine;
    engine.set_tile(1, 0, neuropet::BattleEngine::Tile::HAZARD);
    neuropet::CreatureStats a{1, 1, 1, 1};
    neuropet::CreatureStats b{2, 1, 1, 1};
    EXPECT_EQ(engine.fight(a, b, 0), 2u);

    engine.clear_board();
    engine.set_tile(1, 0, neuropet::BattleEngine::Tile::WALL);
    EXPECT_EQ(engine.fight(a, b, 0), 1u);
}

TEST(BattleEngineTest, BlockingReducesDamage) {
    neuropet::BattleEngine engine;
    neuropet::CreatureStats a{1, 1, 2, 0};
    neuropet::CreatureStats b{2, 2, 1, 1};
    EXPECT_EQ(engine.fight(a, b, 0), 1u);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
