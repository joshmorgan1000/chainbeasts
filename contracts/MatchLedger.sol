// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

/**
 * @title MatchLedger
 * @notice Records deterministic battle results.
 */
contract MatchLedger {
    struct Result {
        uint256 creatureA;
        uint256 creatureB;
        uint256 winnerId;
        bytes32 battleHash;
        address reporter;
    }

    event MatchResult(
        uint256 indexed battleId,
        uint256 indexed creatureA,
        uint256 indexed creatureB,
        uint256 winnerId,
        bytes32 battleHash,
        address reporter
    );

    uint256 public nextBattleId = 1;
    mapping(uint256 => Result) public results;

    /**
     * @dev Record a battle result.
     */
    function record(
        uint256 creatureA,
        uint256 creatureB,
        uint256 winnerId,
        bytes32 battleHash
    ) external returns (uint256 battleId) {
        battleId = nextBattleId++;
        results[battleId] = Result({
            creatureA: creatureA,
            creatureB: creatureB,
            winnerId: winnerId,
            battleHash: battleHash,
            reporter: msg.sender
        });
        emit MatchResult(battleId, creatureA, creatureB, winnerId, battleHash, msg.sender);
    }
}
