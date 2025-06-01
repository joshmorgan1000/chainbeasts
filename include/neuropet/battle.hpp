#pragma once

#include <cstdint>
#include <cstdlib>
#include <optional>
#include <vector>

#include "neuropet/item.hpp"
#include "neuropet/match_ledger.hpp"

namespace neuropet {

/** Creature stats used for deterministic battles. */
struct CreatureStats {
    std::uint32_t id{0};
    int power{0};
    int defense{0};
    int stamina{0};
    std::vector<Item> items{};

    void attach_item(const Item& item) { items.push_back(item); }

    void detach_item(std::uint32_t item_id) {
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->id == item_id) {
                items.erase(it);
                break;
            }
        }
    }

    int total_power() const {
        int v = power;
        for (const auto& i : items)
            v += i.power;
        return v;
    }

    int total_defense() const {
        int v = defense;
        for (const auto& i : items)
            v += i.defense;
        return v;
    }

    int total_stamina() const {
        int v = stamina;
        for (const auto& i : items)
            v += i.stamina;
        return v;
    }
};

/** Deterministic battlefield engine on an 8×8 grid. */
class BattleEngine {
  public:
    enum class Tile { EMPTY, WALL, HAZARD };

    struct Position {
        int x{0};
        int y{0};
    };

    BattleEngine() { clear_board(); }

    /** Clear the entire board to empty tiles. */
    void clear_board() {
        for (auto& row : board_)
            for (auto& t : row)
                t = Tile::EMPTY;
    }

    /** Set the tile type at the given coordinates. */
    void set_tile(int x, int y, Tile t) {
        if (x >= 0 && x < 8 && y >= 0 && y < 8)
            board_[x][y] = t;
    }

    /** Return the tile type at the given coordinates. */
    Tile tile_at(int x, int y) const {
        if (x >= 0 && x < 8 && y >= 0 && y < 8)
            return board_[x][y];
        return Tile::WALL; // treat out-of-bounds as wall
    }

    /**
     * Run a battle between two creatures and return the winner's id.
     * The starting turn is derived from `seed = creatureA_id XOR creatureB_id`.
     * Creatures begin in opposite corners and move toward each other until they
     * are adjacent, then attack until one runs out of health or stamina.
     */
    std::uint32_t fight(const CreatureStats& a, const CreatureStats& b,
                        std::uint32_t seed = 0) const {
        Position posA{0, 0};
        Position posB{7, 7};
        int hpA = a.total_defense();
        int hpB = b.total_defense();
        int staA = a.total_stamina();
        int staB = b.total_stamina();
        unsigned s = seed ? seed : (a.id ^ b.id);
        bool a_turn = (s & 1u) == 0;

        if (board_[posA.x][posA.y] == Tile::HAZARD)
            --hpA;
        if (board_[posB.x][posB.y] == Tile::HAZARD)
            --hpB;

        auto step_towards = [&](Position& from, const Position& to, int& hp) {
            auto try_move = [&](int nx, int ny) {
                if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8)
                    return false;
                if (board_[nx][ny] == Tile::WALL)
                    return false;
                from.x = nx;
                from.y = ny;
                if (board_[nx][ny] == Tile::HAZARD)
                    --hp;
                return true;
            };

            int dx = 0;
            int dy = 0;
            if (from.x < to.x)
                dx = 1;
            else if (from.x > to.x)
                dx = -1;
            else if (from.y < to.y)
                dy = 1;
            else if (from.y > to.y)
                dy = -1;

            if (!try_move(from.x + dx, from.y + dy)) {
                if (dx != 0 && from.y != to.y)
                    try_move(from.x, from.y + (from.y < to.y ? 1 : -1));
                else if (dy != 0 && from.x != to.x)
                    try_move(from.x + (from.x < to.x ? 1 : -1), from.y);
            }
        };

        int blockA = 0;
        int blockB = 0;

        auto distance = [](const Position& p1, const Position& p2) {
            return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
        };

        while (hpA > 0 && hpB > 0 && (staA > 0 || staB > 0)) {
            if (a_turn) {
                if (distance(posA, posB) <= 1) {
                    if (staA > 0) {
                        int dmg = a.total_power();
                        if (blockB > 0) {
                            dmg -= blockB;
                            if (dmg < 0)
                                dmg = 0;
                            blockB = 0;
                        }
                        hpB -= dmg;
                        --staA;
                    } else {
                        ++blockA;
                    }
                } else {
                    step_towards(posA, posB, hpA);
                }
            } else {
                if (distance(posA, posB) <= 1) {
                    if (staB > 0) {
                        int dmg = b.total_power();
                        if (blockA > 0) {
                            dmg -= blockA;
                            if (dmg < 0)
                                dmg = 0;
                            blockA = 0;
                        }
                        hpA -= dmg;
                        --staB;
                    } else {
                        ++blockB;
                    }
                } else {
                    step_towards(posB, posA, hpB);
                }
            }
            a_turn = !a_turn;
        }
        return (hpA >= hpB) ? a.id : b.id;
    }

  private:
    Tile board_[8][8]{};
};

/** Simple FIFO matchmaker running synchronous battles. */
class Matchmaker {
  public:
    explicit Matchmaker(const BattleEngine& engine = BattleEngine(), MatchLedger* ledger = nullptr)
        : engine_(engine), ledger_(ledger) {}

    void set_ledger(MatchLedger* ledger) { ledger_ = ledger; }

    /** Add a creature to the matchmaking queue. */
    void enqueue(const CreatureStats& creature) { queue_.push_back(creature); }

    /**
     * Attempt to run a match if two or more creatures are waiting.
     * @return winner id or empty if no match occurred.
     */
    std::optional<std::uint32_t> try_match(std::uint32_t seed = 0) {
        if (queue_.size() < 2)
            return std::nullopt;
        CreatureStats a = queue_.front();
        queue_.erase(queue_.begin());
        CreatureStats b = queue_.front();
        queue_.erase(queue_.begin());
        std::uint32_t winner = engine_.fight(a, b, seed);
        if (ledger_) {
            std::uint32_t loser = winner == a.id ? b.id : a.id;
            ledger_->record_result(winner, loser);
        }
        return winner;
    }

    /** Number of creatures waiting to be matched. */
    std::size_t pending() const { return queue_.size(); }

  private:
    std::vector<CreatureStats> queue_{};
    BattleEngine engine_{};
    MatchLedger* ledger_{};
};

} // namespace neuropet
