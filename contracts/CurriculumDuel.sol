// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

/**
 * @title CurriculumDuel
 * @notice Minimal commit-reveal contract for per-battle datasets.
 *         This is a prototype used by the client to test private curriculums.
 */
contract CurriculumDuel {
    struct Duel {
        address challenger;
        address opponent;
        uint256 battleBlock;
        bytes32 datasetCommit;
        bytes32 secret;
        bool revealed;
        bool forfeited;
    }

    uint256 public nextId = 1;
    mapping(uint256 => Duel) public duels;
    mapping(uint256 => bytes) public datasets;

    uint256 public constant REVEAL_WINDOW = 3;

    event DuelScheduled(
        uint256 indexed duelId,
        address indexed challenger,
        address indexed opponent,
        uint256 battleBlock,
        bytes32 datasetCommit
    );
    event DatasetRevealed(uint256 indexed duelId);
    event Forfeited(uint256 indexed duelId);

    /**
     * @dev Start a duel with a dataset commitment. The signature must be over
     *      keccak256(datasetCommit || secret || battleBlock) and match the
     *      caller address.
     */
    function challenge(
        address opponent,
        uint256 battleBlock,
        bytes32 datasetCommit,
        bytes32 secret,
        uint8 v,
        bytes32 r,
        bytes32 s
    ) external returns (uint256 duelId) {
        require(battleBlock > block.number, "battle in past");
        bytes32 msgHash = keccak256(abi.encodePacked(datasetCommit, secret, battleBlock));
        require(ecrecover(msgHash, v, r, s) == msg.sender, "bad sig");
        duelId = nextId++;
        duels[duelId] = Duel(msg.sender, opponent, battleBlock, datasetCommit, secret, false, false);
        emit DuelScheduled(duelId, msg.sender, opponent, battleBlock, datasetCommit);
    }

    /**
     * @dev Reveal the dataset within the allowed window. The provided secret
     *      must match the initial commitment.
     */
    function revealDataset(
        uint256 duelId,
        bytes calldata data,
        bytes32 secret
    ) external {
        Duel storage d = duels[duelId];
        require(block.number >= d.battleBlock, "too early");
        require(block.number < d.battleBlock + REVEAL_WINDOW, "reveal over");
        require(!d.revealed && !d.forfeited, "resolved");
        require(secret == d.secret, "secret mismatch");
        require(keccak256(data) == d.datasetCommit, "commit mismatch");
        datasets[duelId] = data;
        d.revealed = true;
        emit DatasetRevealed(duelId);
    }

    /**
     * @dev Claim victory if the challenger fails to reveal in time.
     */
    function claimForfeit(uint256 duelId) external {
        Duel storage d = duels[duelId];
        require(block.number >= d.battleBlock + REVEAL_WINDOW, "too early");
        require(!d.revealed && !d.forfeited, "resolved");
        d.forfeited = true;
        emit Forfeited(duelId);
    }
}
