// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "./IZkVerifier.sol";

/**
 * @title ZkVerifier
 * @notice Simple Keccak256-based proof verifier used for STARK stub integration.
 */
contract ZkVerifier is IZkVerifier {
    /// @inheritdoc IZkVerifier
    function verify(bytes calldata proof) external pure override returns (bytes32) {
        return keccak256(proof);
    }
}
