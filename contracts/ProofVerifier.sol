// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

/**
 * @title ProofVerifier
 * @notice MVP validator requiring a quorum of attestations for each checkpoint.
 */
import "./IZkVerifier.sol";

contract ProofVerifier {
    uint256 public quorum = 3;
    mapping(bytes32 => uint256) public attestations;
    mapping(bytes32 => bool) public proven;
    IZkVerifier public zkVerifier;

    event Attested(bytes32 indexed rootHash, address indexed validator, uint256 count);
    event QuorumChanged(uint256 newQuorum);
    event VerifierChanged(address newVerifier);
    event ProofSubmitted(bytes32 indexed rootHash, address indexed prover);

    constructor(address verifier) {
        zkVerifier = IZkVerifier(verifier);
    }

    /**
     * @dev Record an attestation for a checkpoint root.
     */
    function attest(bytes32 rootHash) external {
        uint256 count = ++attestations[rootHash];
        emit Attested(rootHash, msg.sender, count);
    }

    /**
     * @dev Submit a proof for the given root hash. The proof is verified using
     *      an external zk verifier contract. A valid proof marks the root as
     *      proven and satisfies the quorum.
     */
    function submit_proof(bytes32 rootHash, bytes calldata proof) external {
        require(!proven[rootHash], "already proven");
        bytes32 computed = zkVerifier.verify(proof);
        require(computed == rootHash, "invalid proof");
        proven[rootHash] = true;
        attestations[rootHash] = quorum;
        emit ProofSubmitted(rootHash, msg.sender);
    }

    /**
     * @dev Verify if the required quorum is met.
     */
    function verify(bytes32 rootHash) external view returns (bool) {
        return attestations[rootHash] >= quorum;
    }

    /**
     * @dev Set the quorum required for verification.
     */
    function setQuorum(uint256 newQuorum) external {
        quorum = newQuorum;
        emit QuorumChanged(newQuorum);
    }

    function setVerifier(address verifier) external {
        zkVerifier = IZkVerifier(verifier);
        emit VerifierChanged(verifier);
    }
}
