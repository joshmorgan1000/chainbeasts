// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "./MatchLedger.sol";

/**
 * @title Matchmaker
 * @notice Deterministic on-chain battle engine queuing creatures
 *         and recording results to MatchLedger.
 */
contract Matchmaker {
    struct Stats {
        int32 power;
        int32 defense;
        int32 stamina;
        uint32 id;
    }

    struct Request {
        address owner;
        Stats stats;
    }

    MatchLedger public immutable ledger;
    Request[] public queue;
    uint256 public head;

    event Enqueued(uint256 indexed creatureId, address indexed owner);
    event Matched(
        uint256 indexed battleId,
        uint256 indexed creatureA,
        uint256 indexed creatureB,
        uint256 winner
    );

    constructor(address ledgerAddress) {
        ledger = MatchLedger(ledgerAddress);
    }

    /** Join the matchmaking queue with creature stats. */
    function enqueue(
        uint32 creatureId,
        int32 power,
        int32 defense,
        int32 stamina
    ) external {
        queue.push(Request(msg.sender, Stats(power, defense, stamina, creatureId)));
        emit Enqueued(creatureId, msg.sender);
        if (queue.length - head >= 2) {
            _runMatch();
        }
    }

    /** Number of creatures waiting to be matched. */
    function pending() external view returns (uint256) {
        return queue.length - head;
    }

    function _runMatch() internal {
        Request memory a = queue[head++];
        Request memory b = queue[head++];
        uint32 winner = _fight(a.stats, b.stats, 0);
        uint256 loser = winner == a.stats.id ? b.stats.id : a.stats.id;
        bytes32 battleHash = keccak256(
            abi.encodePacked(
                a.stats.id,
                b.stats.id,
                a.stats.power,
                a.stats.defense,
                a.stats.stamina,
                b.stats.power,
                b.stats.defense,
                b.stats.stamina
            )
        );
        uint256 battleId = ledger.record(a.stats.id, b.stats.id, winner, battleHash);
        emit Matched(battleId, a.stats.id, b.stats.id, winner);
    }

    function _fight(Stats memory a, Stats memory b, uint32 seed) internal pure returns (uint32) {
        uint32 state = seed != 0 ? seed : (a.id ^ b.id);
        int256 hpA = a.defense;
        int256 hpB = b.defense;
        int256 staA = a.stamina;
        int256 staB = b.stamina;
        state = _rng(state);
        bool turn = (state & 1) != 0;
        while (hpA > 0 && hpB > 0 && (staA > 0 || staB > 0)) {
            if (turn && staA > 0) {
                hpB -= a.power;
                staA -= 1;
            } else if (!turn && staB > 0) {
                hpA -= b.power;
                staB -= 1;
            }
            state = _rng(state);
            turn = (state & 1) != 0;
        }
        return uint32(hpA >= hpB ? a.id : b.id);
    }

    function _rng(uint32 state) private pure returns (uint32) {
        unchecked {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
        }
        return state;
    }
}
