// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../CreatureNFT.sol";
import "../Core.sol";

contract TraitLockTest is DSTest {
    CreatureNFT nft;
    Core core;

    function setUp() public {
        nft = new CreatureNFT();
        core = new Core();
        nft.setCore(address(core));
        core.mint(address(this), 1 ether);
    }

    function testLockTraits() public {
        uint256 tokenId = nft.hatch("");
        core.approve(address(nft), 1 ether);
        nft.lockTraits(tokenId, 0x1234, bytes32(uint256(1)));
        (uint256 t, bytes32 nh) = nft.traits(tokenId);
        assertEq(t, 0x1234);
        assertEq(nh, bytes32(uint256(1)));
    }
}
