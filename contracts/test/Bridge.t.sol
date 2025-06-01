// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../CreatureNFT.sol";
import "../CreatureNFTBridge.sol";

contract BridgeTest is DSTest {
    CreatureNFT nftA;
    CreatureNFT nftB;
    CreatureNFT nftC;
    CreatureNFTBridge bridgeA;
    CreatureNFTBridge bridgeB;
    CreatureNFTBridge bridgeC;

    function setUp() public {
        nftA = new CreatureNFT();
        nftB = new CreatureNFT();
        nftC = new CreatureNFT();
        bridgeA = new CreatureNFTBridge(address(nftA));
        bridgeB = new CreatureNFTBridge(address(nftB));
        bridgeC = new CreatureNFTBridge(address(nftC));
        nftA.setBridge(address(bridgeA));
        nftB.setBridge(address(bridgeB));
        nftC.setBridge(address(bridgeC));
    }

    function testBridgeCycle() public {
        uint256 tokenId = nftA.hatch("");
        (bytes memory weights, bytes32 dna) = nftA.creatures(tokenId);
        nftA.approve(address(bridgeA), tokenId);
        bridgeA.bridgeOut(tokenId, 2);
        // token locked on chain A
        assertEq(nftA.ownerOf(tokenId), address(bridgeA));

        // mint on chain B
        bridgeB.bridgeIn(tokenId, address(this), 1, weights, dna);
        assertEq(nftB.ownerOf(tokenId), address(this));

        // move back to chain A
        nftB.approve(address(bridgeB), tokenId);
        bridgeB.bridgeOut(tokenId, 1);
        bridgeA.bridgeIn(tokenId, address(this), 2, weights, dna);
        assertEq(nftA.ownerOf(tokenId), address(this));
    }

    function testBridgeAcrossChains() public {
        uint256 tokenId = nftA.hatch("");
        (bytes memory weights, bytes32 dna) = nftA.creatures(tokenId);
        nftA.approve(address(bridgeA), tokenId);
        bridgeA.bridgeOut(tokenId, 2);
        assertTrue(bridgeA.locked(tokenId));

        bridgeB.bridgeIn(tokenId, address(this), 1, weights, dna);
        assertTrue(bridgeB.mirror(tokenId));
        assertEq(nftB.ownerOf(tokenId), address(this));

        nftB.approve(address(bridgeB), tokenId);
        bridgeB.bridgeOut(tokenId, 3);
        assertTrue(!bridgeB.mirror(tokenId));

        bridgeC.bridgeIn(tokenId, address(this), 2, weights, dna);
        assertTrue(bridgeC.mirror(tokenId));
        assertEq(nftC.ownerOf(tokenId), address(this));
        assertTrue(bridgeA.locked(tokenId));
    }
}
