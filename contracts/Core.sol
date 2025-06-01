// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

/**
 * @title Core Token
 * @notice ERC20-style token used for staking in the Governance contract.
 *         Supply is capped at 1 billion tokens.
 */
contract Core {
    string public constant name = "Core Token";
    string public constant symbol = "CORE";
    uint8 public constant decimals = 18;
    uint256 public constant CAP = 1_000_000_000 ether;

    address public admin;
    uint256 public totalSupply;
    mapping(address => uint256) public balanceOf;
    mapping(address => mapping(address => uint256)) public allowance;

    event Transfer(address indexed from, address indexed to, uint256 amount);
    event Approval(address indexed owner, address indexed spender, uint256 amount);
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

    /** Mint new CORE tokens up to the cap. */
    function mint(address to, uint256 amount) external onlyAdmin {
        require(totalSupply + amount <= CAP, "cap");
        balanceOf[to] += amount;
        totalSupply += amount;
        emit Minted(to, amount);
        emit Transfer(address(0), to, amount);
    }

    /** Burn CORE from an address. */
    function burnFrom(address from, uint256 amount) external onlyAdmin {
        require(balanceOf[from] >= amount, "balance");
        balanceOf[from] -= amount;
        totalSupply -= amount;
        emit Burned(from, amount);
        emit Transfer(from, address(0), amount);
    }

    function approve(address spender, uint256 amount) external returns (bool) {
        allowance[msg.sender][spender] = amount;
        emit Approval(msg.sender, spender, amount);
        return true;
    }

    function transfer(address to, uint256 amount) external returns (bool) {
        require(balanceOf[msg.sender] >= amount, "balance");
        balanceOf[msg.sender] -= amount;
        balanceOf[to] += amount;
        emit Transfer(msg.sender, to, amount);
        return true;
    }

    function transferFrom(address from, address to, uint256 amount) external returns (bool) {
        require(balanceOf[from] >= amount, "balance");
        uint256 allowed = allowance[from][msg.sender];
        require(allowed >= amount, "allowance");
        allowance[from][msg.sender] = allowed - amount;
        balanceOf[from] -= amount;
        balanceOf[to] += amount;
        emit Transfer(from, to, amount);
        return true;
    }
}
