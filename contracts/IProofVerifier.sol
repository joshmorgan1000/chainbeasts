// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

interface IProofVerifier {
    function verify(bytes32 rootHash) external view returns (bool);
}
