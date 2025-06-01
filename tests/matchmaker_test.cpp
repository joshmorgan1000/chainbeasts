#include "neuropet/battle.hpp"
#include <gtest/gtest.h>

TEST(MatchmakerTest, RunsAndRecords) {
    neuropet::BattleEngine engine;
    neuropet::MatchLedger ledger;
    neuropet::Matchmaker mm(engine, &ledger);
    neuropet::CreatureStats a{1, 1, 1, 1};
    neuropet::CreatureStats b{2, 1, 1, 1};
    mm.enqueue(a);
    mm.enqueue(b);
    auto winner = mm.try_match(123);
    ASSERT_EQ(winner.has_value(), true);
    EXPECT_EQ(ledger.results().size(), 1u);
    EXPECT_EQ(ledger.results()[0].winner, *winner);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
