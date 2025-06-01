// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "./CreatureNFT.sol";

/**
 * @title CreatureNFTBridge
 * @notice Simple lock/mint bridge for CreatureNFT tokens.
 */
contract CreatureNFTBridge {
    CreatureNFT public immutable nft;

    // Token locked on this chain
    mapping(uint256 => bool) public locked;
    // Token minted by this bridge (mirror)
    mapping(uint256 => bool) public mirror;

    event BridgeOut(uint256 indexed tokenId, address indexed owner, uint256 dstChainId, bytes genesisWeights, bytes32 dna);
    event BridgeIn(uint256 indexed tokenId, address indexed owner, uint256 srcChainId);

    constructor(address nftAddress) {
        nft = CreatureNFT(nftAddress);
    }

    /**
     * @dev Bridge a token out to another chain.
     *      Canonical tokens are locked while mirror tokens are burned.
     */
    function bridgeOut(uint256 tokenId, uint256 dstChainId) external {
        require(nft.ownerOf(tokenId) == msg.sender, "not owner");
        CreatureNFT.Creature memory info = nft.creatures(tokenId);
        if (mirror[tokenId]) {
            nft.transferFrom(msg.sender, address(this), tokenId);
            nft.burnFromBridge(tokenId);
            mirror[tokenId] = false;
        } else {
            nft.transferFrom(msg.sender, address(this), tokenId);
            locked[tokenId] = true;
        }
        emit BridgeOut(tokenId, msg.sender, dstChainId, info.genesisWeights, info.dna);
    }

    /**
     * @dev Bridge a token in from another chain. If the token is locked locally
     *      it is released, otherwise a mirror token is minted.
     */
    function bridgeIn(
        uint256 tokenId,
        address to,
        uint256 srcChainId,
        bytes calldata genesisWeights,
        bytes32 dna
    ) external {
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
