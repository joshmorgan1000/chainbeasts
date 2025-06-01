// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

/**
 * @title FashionDuel
 * @notice Minimal commit-reveal voting protocol for style battles.
 */
contract FashionDuel {
    enum VoteSide {
        None,
        Challenger,
        Opponent
    }

    struct Duel {
        address challenger;
        address opponent;
        uint256 judgeBlock;
        uint256 commitEnd;
        uint256 revealEnd;
        bool resolved;
    }

    uint256 public nextId = 1;
    mapping(uint256 => Duel) public duels;
    mapping(uint256 => mapping(address => bytes32)) public commitments;
    mapping(uint256 => mapping(address => VoteSide)) public reveals;
    mapping(uint256 => uint256) public weightChallenger;
    mapping(uint256 => uint256) public weightOpponent;

    event DuelInitiated(
        uint256 indexed duelId,
        address indexed challenger,
        address indexed opponent,
        uint256 judgeBlock,
        uint256 commitEnd,
        uint256 revealEnd
    );
    event Committed(uint256 indexed duelId, address indexed voter);
    event Revealed(uint256 indexed duelId, address indexed voter, VoteSide side);
    event Tally(uint256 indexed duelId, VoteSide winner);

    /**
     * @dev Start a duel and define the commit/reveal windows.
     */
    function initiate(
        address opponent,
        uint256 judgeBlock,
        uint256 commitWindow,
        uint256 revealWindow
    ) external returns (uint256 duelId) {
        require(judgeBlock > block.number, "judge in past");
        duelId = nextId++;
        Duel storage d = duels[duelId];
        d.challenger = msg.sender;
        d.opponent = opponent;
        d.judgeBlock = judgeBlock;
        d.commitEnd = judgeBlock + commitWindow;
        d.revealEnd = d.commitEnd + revealWindow;
        emit DuelInitiated(
            duelId,
            msg.sender,
            opponent,
            judgeBlock,
            d.commitEnd,
            d.revealEnd
        );
    }

    /**
     * @dev Submit a hashed vote during the commit window.
     */
    function commit(uint256 duelId, bytes32 commitment) external {
        Duel storage d = duels[duelId];
        require(block.number >= d.judgeBlock, "too early");
        require(block.number < d.commitEnd, "commit over");
        commitments[duelId][msg.sender] = commitment;
        emit Committed(duelId, msg.sender);
    }

    function encodeVote(VoteSide side, bytes32 salt) public pure returns (bytes32) {
        return keccak256(abi.encodePacked(uint8(side), salt));
    }

    /**
     * @dev Reveal a vote with the original salt during the reveal window.
     */
    function reveal(
        uint256 duelId,
        VoteSide side,
        bytes32 salt
    ) external {
        Duel storage d = duels[duelId];
        require(block.number >= d.commitEnd, "not reveal phase");
        require(block.number < d.revealEnd, "reveal over");
        require(
            commitments[duelId][msg.sender] == encodeVote(side, salt),
            "commit mismatch"
        );
        require(reveals[duelId][msg.sender] == VoteSide.None, "already revealed");
        reveals[duelId][msg.sender] = side;
        if (side == VoteSide.Challenger) {
            weightChallenger[duelId] += 1;
        } else if (side == VoteSide.Opponent) {
            weightOpponent[duelId] += 1;
        }
        emit Revealed(duelId, msg.sender, side);
    }

    /**
     * @dev Tally votes after the reveal window and emit the winner.
     */
    function tally(uint256 duelId) external {
        Duel storage d = duels[duelId];
        require(block.number >= d.revealEnd, "too early");
        require(!d.resolved, "already tallied");
        d.resolved = true;
        VoteSide winner =
            weightChallenger[duelId] >= weightOpponent[duelId]
                ? VoteSide.Challenger
                : VoteSide.Opponent;
        emit Tally(duelId, winner);
    }
}

