// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

interface IZkVerifier {
    function verify(bytes calldata proof) external view returns (bytes32);
}
