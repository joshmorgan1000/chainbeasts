// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "./CreatureNFT.sol";
import "./IProofVerifier.sol";

/**
 * @title CreatureNFTBridge
 * @notice Simple lock/mint bridge for CreatureNFT tokens.
 */
contract CreatureNFTBridge {
    CreatureNFT public immutable nft;
    IProofVerifier public verifier;

    // Token locked on this chain
    mapping(uint256 => bool) public locked;
    // Token minted by this bridge (mirror)
    mapping(uint256 => bool) public mirror;

    event BridgeOut(
        uint256 indexed tokenId,
        address indexed owner,
        uint256 dstChainId,
        bytes genesisWeights,
        bytes32 dna,
        bytes32 rootHash
    );
    event BridgeIn(uint256 indexed tokenId, address indexed owner, uint256 srcChainId);

    mapping(bytes32 => bool) public processed;
    constructor(address nftAddress, address verifierAddr) {
        nft = CreatureNFT(nftAddress);
        verifier = IProofVerifier(verifierAddr);
    }

    /**
     * @dev Bridge a token out to another chain.
     *      Canonical tokens are locked while mirror tokens are burned.
     */
    function bridgeOut(uint256 tokenId, uint256 dstChainId) external {
        require(nft.ownerOf(tokenId) == msg.sender, "not owner");
        CreatureNFT.Creature memory info = nft.creatures(tokenId);
        bytes32 rootHash = keccak256(
            abi.encodePacked(tokenId, msg.sender, block.chainid, info.genesisWeights, info.dna)
        );
        if (mirror[tokenId]) {
            nft.transferFrom(msg.sender, address(this), tokenId);
            nft.burnFromBridge(tokenId);
            mirror[tokenId] = false;
        } else {
            nft.transferFrom(msg.sender, address(this), tokenId);
            locked[tokenId] = true;
        }
        emit BridgeOut(
            tokenId,
            msg.sender,
            dstChainId,
            info.genesisWeights,
            info.dna,
            rootHash
        );
    }

    /**
     * @dev Bridge a token in from another chain. If the token is locked locally
     *      it is released, otherwise a mirror token is minted.
     */
    function bridgeIn(
        uint256 tokenId,
        address to,
        uint256 srcChainId,
        uint256 dstChainId,
        bytes calldata genesisWeights,
        bytes32 dna,
        bytes calldata proof
    ) external {
        bytes32 rootHash = keccak256(
            abi.encodePacked(tokenId, to, srcChainId, genesisWeights, dna)
        );
        require(!processed[rootHash], "processed");
        if (address(verifier) != address(0)) {
            require(verifier.verify(rootHash), "unverified");
        }
        processed[rootHash] = true;
        if (locked[tokenId]) {
            locked[tokenId] = false;
            nft.transferFrom(address(this), to, tokenId);
        } else {
            nft.mintFromBridge(to, tokenId, genesisWeights, dna);
            mirror[tokenId] = true;
        }
        emit BridgeIn(tokenId, to, srcChainId);
    }
}
