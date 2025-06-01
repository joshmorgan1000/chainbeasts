// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

/**
 * @title Energy
 * @notice Transferable reward token minted for PoUW miners.
 *         Balances can be spent or burned by authorised contracts when
 *         creatures train or participate in battles.
 */
contract Energy {
    address public admin;
    uint256 public totalSupply;
    mapping(address => uint256) public balanceOf;

    event Transfer(address indexed from, address indexed to, uint256 amount);

    event Minted(address indexed to, uint256 amount);
    event Burned(address indexed from, uint256 amount);
    event AdminUpdated(address newAdmin);

    constructor() {
        admin = msg.sender;
    }

    modifier onlyAdmin() {
        require(msg.sender == admin, "not admin");
        _;
    }

    function setAdmin(address newAdmin) external onlyAdmin {
        admin = newAdmin;
        emit AdminUpdated(newAdmin);
    }

    /** Mint new ENERGY to an address. */
    function mint(address to, uint256 amount) external onlyAdmin {
        balanceOf[to] += amount;
        totalSupply += amount;
        emit Minted(to, amount);
        emit Transfer(address(0), to, amount);
    }

    /** Burn ENERGY from a user's balance. */
    function burnFrom(address from, uint256 amount) external onlyAdmin {
        require(balanceOf[from] >= amount, "balance");
        balanceOf[from] -= amount;
        totalSupply -= amount;
        emit Burned(from, amount);
        emit Transfer(from, address(0), amount);
    }

    /** Spend caller's ENERGY balance. */
    function spend(uint256 amount) external {
        require(balanceOf[msg.sender] >= amount, "balance");
        balanceOf[msg.sender] -= amount;
        totalSupply -= amount;
        emit Burned(msg.sender, amount);
        emit Transfer(msg.sender, address(0), amount);
    }

    /** Transfer ENERGY to another address. */
    function transfer(address to, uint256 amount) external {
        require(balanceOf[msg.sender] >= amount, "balance");
        balanceOf[msg.sender] -= amount;
        balanceOf[to] += amount;
        emit Transfer(msg.sender, to, amount);
    }
}
