// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../CreatureNFT.sol";
import "../CreatureNFTBridge.sol";
import "../TrainingLedger.sol";
import "../Energy.sol";
import "../Core.sol";
import "../ZkVerifier.sol";
import "../ProofVerifier.sol";
import "../SeasonRegistry.sol";

contract BridgePoUWTest is DSTest {
    CreatureNFT nftA;
    CreatureNFT nftB;
    CreatureNFTBridge bridgeA;
    CreatureNFTBridge bridgeB;
    Energy energy;
    Core core;
    ZkVerifier zk;
    ProofVerifier verifier;
    SeasonRegistry seasons;
    TrainingLedger training;

    function setUp() public {
        nftA = new CreatureNFT();
        nftB = new CreatureNFT();
        zk = new ZkVerifier();
        bridgeA = new CreatureNFTBridge(address(nftA), address(0));
        bridgeB = new CreatureNFTBridge(address(nftB), address(0));
        nftA.setBridge(address(bridgeA));
        nftB.setBridge(address(bridgeB));
        energy = new Energy();
        core = new Core();
        verifier = new ProofVerifier(address(zk));
        seasons = new SeasonRegistry(address(this));
        training = new TrainingLedger(address(energy), address(core), address(verifier), address(seasons));
    }

    function testBridgeAndTrain() public {
        uint256 tokenId = nftA.hatch("");
        (bytes memory w, bytes32 dna) = nftA.creatures(tokenId);
        nftA.approve(address(bridgeA), tokenId);
        bridgeA.bridgeOut(tokenId, 2);
        bytes memory proof = abi.encode(tokenId, address(this), 2, w, dna);
        bridgeB.bridgeIn(tokenId, address(this), 1, 2, w, dna, proof);
        assertEq(nftB.ownerOf(tokenId), address(this));

        bytes32 root = sha256(bytes("p0"));
        uint32 spent = 5;
        training.submitCheckpoint(tokenId, 0, root, 32, 1, false, spent, bytes32(0));
        verifier.submit_proof(root, bytes("p0"));
        training.finalizeCheckpoint(root);
        assertEq(energy.balanceOf(address(this)), spent);
        assertEq(core.balanceOf(address(this)), uint256(spent) * TrainingLedger.CORE_PER_ENERGY());

        nftB.approve(address(bridgeB), tokenId);
        bridgeB.bridgeOut(tokenId, 1);
        proof = abi.encode(tokenId, address(this), 1, w, dna);
        bridgeA.bridgeIn(tokenId, address(this), 2, 1, w, dna, proof);
        assertEq(nftA.ownerOf(tokenId), address(this));
    }
}
