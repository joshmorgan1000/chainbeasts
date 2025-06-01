// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../Energy.sol";

contract EnergyTest is DSTest {
    Energy energy;
    address constant BOB = address(0xB0B);

    function setUp() public {
        energy = new Energy();
    }

    function testMintAndSpend() public {
        energy.mint(address(this), 10);
        assertEq(energy.balanceOf(address(this)), 10);
        energy.transfer(BOB, 3);
        assertEq(energy.balanceOf(BOB), 3);
        energy.spend(4);
        assertEq(energy.balanceOf(address(this)), 3);
        energy.burnFrom(address(this), 3);
        assertEq(energy.balanceOf(address(this)), 0);
    }
}
