// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../Marketplace.sol";
import "../CreatureNFT.sol";

contract AttachmentTest is DSTest {
    Marketplace market;
    CreatureNFT nft;

    function setUp() public {
        nft = new CreatureNFT();
        market = new Marketplace(address(nft), address(this));
        nft.setMarketplace(address(market));
    }

    function testAttachDetach() public {
        uint256 tokenId = nft.hatch("");
        market.registerItem(1);
        market.attachItem(tokenId, 1);
        assertEq(market.itemAttachedTo(1), tokenId);
        market.detachItem(tokenId, 1);
        assertEq(market.itemAttachedTo(1), 0);
    }
}
