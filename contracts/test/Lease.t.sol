// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../Marketplace.sol";
import "../CreatureNFT.sol";

contract LeaseUser {
    function rent(Marketplace m, uint256 tokenId) external payable {
        m.rent{value: msg.value}(tokenId);
    }
}

contract LeaseTest is DSTest {
    Marketplace market;
    CreatureNFT nft;
    LeaseUser user;

    function setUp() public {
        nft = new CreatureNFT();
        market = new Marketplace(address(nft), address(this));
        nft.setMarketplace(address(market));
        user = new LeaseUser();
    }

    function testCancelLease() public {
        uint256 tokenId = nft.hatch("");
        market.listForLease(tokenId, 1 ether, 100);
        market.cancelLease(tokenId);
        (address owner,, , ,) = market.leases(tokenId);
        assertEq(owner, address(0));
    }

    function testRentInfo() public {
        uint256 tokenId = nft.hatch("");
        market.listForLease(tokenId, 1 ether, 100);
        user.rent{value: 1 ether}(market, tokenId);
        Marketplace.Lease memory info = market.leases(tokenId);
        assertEq(info.owner, address(this));
        assertEq(info.renter, address(user));
        assertTrue(info.expiry > 0);
    }

    function testRelistAfterCancel() public {
        uint256 tokenId = nft.hatch("");
        market.listForLease(tokenId, 1 ether, 100);
        market.cancelLease(tokenId);
        market.listForLease(tokenId, 2 ether, 50);
        Marketplace.Lease memory info = market.leases(tokenId);
        assertEq(info.owner, address(this));
        assertEq(info.price, 2 ether);
        assertEq(info.duration, 50);
    }
}
