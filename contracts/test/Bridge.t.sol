// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../CreatureNFT.sol";
import "../CreatureNFTBridge.sol";
import "../ZkVerifier.sol";
import "../IProofVerifier.sol";

contract FakeVerifier is IProofVerifier {
    bool result;
    constructor(bool r) {
        result = r;
    }
    function verify(bytes32) external view override returns (bool) {
        return result;
    }
}

contract BridgeTest is DSTest {
    CreatureNFT nftA;
    CreatureNFT nftB;
    CreatureNFT nftC;
    CreatureNFT nftD;
    CreatureNFTBridge bridgeA;
    CreatureNFTBridge bridgeB;
    CreatureNFTBridge bridgeC;
    CreatureNFTBridge bridgeD;
    FakeVerifier verifier;

    function setUp() public {
        nftA = new CreatureNFT();
        nftB = new CreatureNFT();
        nftC = new CreatureNFT();
        nftD = new CreatureNFT();
        verifier = new FakeVerifier(false);
        bridgeA = new CreatureNFTBridge(address(nftA), address(0));
        bridgeB = new CreatureNFTBridge(address(nftB), address(0));
        bridgeC = new CreatureNFTBridge(address(nftC), address(0));
        bridgeD = new CreatureNFTBridge(address(nftD), address(verifier));
        nftA.setBridge(address(bridgeA));
        nftB.setBridge(address(bridgeB));
        nftC.setBridge(address(bridgeC));
        nftD.setBridge(address(bridgeD));
    }

    function testBridgeCycle() public {
        uint256 tokenId = nftA.hatch("");
        (bytes memory weights, bytes32 dna) = nftA.creatures(tokenId);
        nftA.approve(address(bridgeA), tokenId);
        bridgeA.bridgeOut(tokenId, 2);
        // token locked on chain A
        assertEq(nftA.ownerOf(tokenId), address(bridgeA));

        // mint on chain B
        bytes memory proof = abi.encode(tokenId, address(this), 2, weights, dna);
        bridgeB.bridgeIn(tokenId, address(this), 1, 2, weights, dna, proof);
        assertEq(nftB.ownerOf(tokenId), address(this));

        // move back to chain A
        nftB.approve(address(bridgeB), tokenId);
        bridgeB.bridgeOut(tokenId, 1);
        proof = abi.encode(tokenId, address(this), 1, weights, dna);
        bridgeA.bridgeIn(tokenId, address(this), 2, 1, weights, dna, proof);
        assertEq(nftA.ownerOf(tokenId), address(this));
    }

    function testBridgeAcrossChains() public {
        uint256 tokenId = nftA.hatch("");
        (bytes memory weights, bytes32 dna) = nftA.creatures(tokenId);
        nftA.approve(address(bridgeA), tokenId);
        bridgeA.bridgeOut(tokenId, 2);
        assertTrue(bridgeA.locked(tokenId));

        bytes memory proof = abi.encode(tokenId, address(this), 2, weights, dna);
        bridgeB.bridgeIn(tokenId, address(this), 1, 2, weights, dna, proof);
        assertTrue(bridgeB.mirror(tokenId));
        assertEq(nftB.ownerOf(tokenId), address(this));

        nftB.approve(address(bridgeB), tokenId);
        bridgeB.bridgeOut(tokenId, 3);
        assertTrue(!bridgeB.mirror(tokenId));

        proof = abi.encode(tokenId, address(this), 3, weights, dna);
        bridgeC.bridgeIn(tokenId, address(this), 2, 3, weights, dna, proof);
        assertTrue(bridgeC.mirror(tokenId));
        assertEq(nftC.ownerOf(tokenId), address(this));
        assertTrue(bridgeA.locked(tokenId));
    }

    function testBridgeInTwice() public {
        uint256 tokenId = nftA.hatch("");
        (bytes memory w, bytes32 dna) = nftA.creatures(tokenId);
        nftA.approve(address(bridgeA), tokenId);
        bridgeA.bridgeOut(tokenId, 2);
        bytes memory proof = abi.encode(tokenId, address(this), 2, w, dna);
        bridgeB.bridgeIn(tokenId, address(this), 1, 2, w, dna, proof);
        bool success = true;
        try bridgeB.bridgeIn(tokenId, address(this), 1, 2, w, dna, proof) {
            success = false;
        } catch Error(string memory reason) {
            assertEq(reason, "processed");
        }
        assertTrue(success);
    }

    function testBridgeInRequiresVerifiedProof() public {
        uint256 tokenId = nftA.hatch("");
        (bytes memory w, bytes32 dna) = nftA.creatures(tokenId);
        nftA.approve(address(bridgeA), tokenId);
        bridgeA.bridgeOut(tokenId, 4);
        bytes memory proof = abi.encode(tokenId, address(this), 4, w, dna);
        bool threw = false;
        try bridgeD.bridgeIn(tokenId, address(this), 1, 4, w, dna, proof) {
            threw = false;
        } catch Error(string memory reason) {
            threw = true;
            assertEq(reason, "unverified");
        }
        assertTrue(threw);
    }
}
