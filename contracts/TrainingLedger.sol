// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "./IEnergy.sol";
import "./ICore.sol";
import "./SeasonRegistry.sol";

interface IProofVerifier {
    function verify(bytes32 rootHash) external view returns (bool);
}

/**
 * @title TrainingLedger
 * @notice Records checkpoint submissions for creature training.
 */
contract TrainingLedger {
    IEnergy public energy;
    ICore public core;
    IProofVerifier public verifier;
    SeasonRegistry public seasons;
    uint256 public constant CORE_PER_ENERGY = 1 ether;
    uint256 public constant DEFAULT_MAX_BYTES = 64 * 1024;

    function _maxBytes() internal view returns (uint256) {
        return address(seasons) != address(0)
            ? seasons.maxNetworkBytes()
            : DEFAULT_MAX_BYTES;
    }

    struct Submission {
        uint256 creatureId;
        uint256 epochId;
        address miner;
        uint32 loss;
        uint32 energySpent;
    }

    mapping(bytes32 => Submission) private _submission;
    mapping(bytes32 => bool) public finalized;
    mapping(uint256 => uint32) public lastLoss;
    mapping(uint256 => uint256) public nextEpoch;

    event Reward(address indexed miner, uint256 amount);
    event CoreReward(address indexed miner, uint256 amount);
    event CheckpointSubmitted(
        uint256 indexed creatureId,
        uint256 indexed epochId,
        bytes32 rootHash,
        uint32 modelBytes,
        uint32 loss,
        bool lossDrop,
        uint32 energySpent,
        bytes32 ruleHash,
        address indexed submitter
    );
    event CheckpointFinalized(
        uint256 indexed creatureId,
        uint256 indexed epochId,
        bytes32 rootHash,
        address indexed miner
    );

    constructor(
        address energyAddr,
        address coreAddr,
        address verifierAddr,
        address seasonRegistry
    ) {
        energy = IEnergy(energyAddr);
        core = ICore(coreAddr);
        verifier = IProofVerifier(verifierAddr);
        seasons = SeasonRegistry(seasonRegistry);
    }

    /**
     * @dev Submit a training checkpoint.
     */
    function submitCheckpoint(
        uint256 creatureId,
        uint256 epochId,
        bytes32 rootHash,
        uint32 modelBytes,
        uint32 loss,
        bool lossDrop,
        uint32 energySpent,
        bytes32 ruleHash
    ) external {
        require(modelBytes <= _maxBytes(), "weights too large");
        if (energySpent > 0) {
            energy.burnFrom(msg.sender, energySpent);
        }
        _submission[rootHash] =
            Submission(creatureId, epochId, msg.sender, loss, energySpent);
        emit CheckpointSubmitted(
            creatureId,
            epochId,
            rootHash,
            modelBytes,
            loss,
            lossDrop,
            energySpent,
            ruleHash,
            msg.sender
        );
    }

    /**
     * @dev Mint ENERGY reward to a miner once a checkpoint is verified.
     */
    function rewardMiner(address miner, uint256 amount) external {
        energy.mint(miner, amount);
        emit Reward(miner, amount);
    }

    /**
     * @dev Finalize a checkpoint once the proof verifier confirms it.
     *      Rewards the submitting miner based on the ENERGY spent.
     */
    function finalizeCheckpoint(bytes32 rootHash) external {
        require(!finalized[rootHash], "finalized");
        require(verifier.verify(rootHash), "unverified");
        Submission memory sub = _submission[rootHash];
        address miner = sub.miner;
        require(miner != address(0), "unknown miner");
        require(sub.epochId == nextEpoch[sub.creatureId], "epoch mismatch");
        uint32 prev = lastLoss[sub.creatureId];
        require(prev == 0 || sub.loss <= prev, "loss increased");
        nextEpoch[sub.creatureId] = sub.epochId + 1;
        lastLoss[sub.creatureId] = sub.loss;
        finalized[rootHash] = true;
        energy.mint(miner, sub.energySpent);
        emit Reward(miner, sub.energySpent);
        uint256 coreReward = uint256(sub.energySpent) * CORE_PER_ENERGY;
        core.mint(miner, coreReward);
        emit CoreReward(miner, coreReward);
        emit CheckpointFinalized(sub.creatureId, sub.epochId, rootHash, miner);
    }
}
