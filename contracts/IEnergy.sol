// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

interface IEnergy {
    function mint(address to, uint256 amount) external;
    function burnFrom(address from, uint256 amount) external;
    function transfer(address to, uint256 amount) external;
    function spend(uint256 amount) external;
}
