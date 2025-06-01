// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../ProofVerifier.sol";
import "../ZkVerifier.sol";

contract ProofVerifierTest is DSTest {
    ProofVerifier verifier;
    ZkVerifier zk;

    function setUp() public {
        zk = new ZkVerifier();
        verifier = new ProofVerifier(address(zk));
    }

    function testSubmitAndVerify() public {
        bytes memory proof = bytes("root");
        bytes32 root = zk.verify(proof);
        verifier.submit_proof(root, proof);
        assertTrue(verifier.proven(root));
        assertEq(verifier.attestations(root), verifier.quorum());
        assertTrue(verifier.verify(root));
    }
}
