// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

/// @title IGovernanceHook
/// @notice Interface for DAO hooks notified when governance actions occur.
interface IGovernanceHook {
    /// Called after a governance controlled action is executed.
    /// @param target Contract that executed the action.
    /// @param selector Function selector of the action.
    /// @param data Calldata passed to the action.
    function onAction(address target, bytes4 selector, bytes calldata data) external;
}
