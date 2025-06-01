#include "neuropet/battle_animation.hpp"
#include <gtest/gtest.h>

TEST(BattleAnimationTest, RecordsDeterministicFrames) {
    neuropet::CreatureStats a{1, 1, 1, 2};
    neuropet::CreatureStats b{2, 1, 1, 2};
    neuropet::BattleAnimator anim;
    anim.run(a, b, 42);
    EXPECT_EQ(anim.winner(), 2u);
    ASSERT_EQ(anim.frames().size(), 1u);
    const auto& f = anim.frames()[0];
    EXPECT_EQ(f.attacker_is_a, false);
    EXPECT_EQ(f.hp_a, 0);
    EXPECT_EQ(f.hp_b, 1);
    EXPECT_EQ(f.stamina_a, 2);
    EXPECT_EQ(f.stamina_b, 1);

    std::uint32_t replay_winner = neuropet::BattleAnimator::replay(a, b, anim.frames());
    EXPECT_EQ(replay_winner, anim.winner());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
