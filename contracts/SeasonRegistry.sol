// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "./IGovernanceHook.sol";

/**
 * @title SeasonRegistry
 * @notice Stores season constants used by the deterministic engine.
 */
contract SeasonRegistry {
    address public governance;
    address public governanceHook;
    mapping(bytes32 => bytes) public tables;
    uint256 public maxNetworkBytes = 64 * 1024;

    event TableUpdated(bytes32 indexed key);
    event GovernanceUpdated(address newGovernance);
    event GovernanceHookUpdated(address newHook);
    event MaxNetworkBytesUpdated(uint256 newLimit);

    constructor(address governanceAddr) {
        governance = governanceAddr == address(0) ? msg.sender : governanceAddr;
    }

    modifier onlyGovernance() {
        require(msg.sender == governance, "not gov");
        _;
    }

    function setGovernanceHook(address hook) external onlyGovernance {
        governanceHook = hook;
        emit GovernanceHookUpdated(hook);
    }

    /**
     * @dev Set a constant table, callable by governance.
     */
    function setTable(bytes32 key, bytes calldata data) external onlyGovernance {
        tables[key] = data;
        emit TableUpdated(key);
        _notifyHook(SeasonRegistry.setTable.selector, abi.encode(key, data));
    }

    function setGovernance(address newGovernance) external onlyGovernance {
        governance = newGovernance;
        emit GovernanceUpdated(newGovernance);
        _notifyHook(SeasonRegistry.setGovernance.selector, abi.encode(newGovernance));
    }

    function setMaxNetworkBytes(uint256 limit) external onlyGovernance {
        maxNetworkBytes = limit;
        emit MaxNetworkBytesUpdated(limit);
        _notifyHook(SeasonRegistry.setMaxNetworkBytes.selector, abi.encode(limit));
    }

    /**
     * @dev Return a constant table by key.
     */
    function getTable(bytes32 key) external view returns (bytes memory) {
        return tables[key];
    }

    function _notifyHook(bytes4 selector, bytes memory data) internal {
        if (governanceHook != address(0)) {
            try IGovernanceHook(governanceHook).onAction(address(this), selector, data) {
            } catch {}
        }
    }
}
