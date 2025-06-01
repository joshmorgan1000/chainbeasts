// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

/**
 * @title CreatureNFT
 * @notice ERC721 token representing a trainable creature.
 *         Each wallet may hatch a single creature whose DNA is
 *         derived from the previous block hash and wallet address.
 */
import "./SeasonRegistry.sol";
import "./ICore.sol";

contract CreatureNFT {
    event CreatureHatched(address indexed owner, uint256 indexed tokenId, bytes32 dna);

    address public bridge;
    SeasonRegistry public seasons;
    uint256 public constant DEFAULT_MAX_BYTES = 64 * 1024;

    struct Creature {
        bytes genesisWeights; // INT8 weights, optional off-chain storage
        bytes32 dna;
    }

    uint256 public nextId = 1;
    mapping(uint256 => address) private _ownerOf;
    mapping(address => bool) public hasHatched;
    mapping(uint256 => Creature) public creatures;
    mapping(uint256 => address) public getApproved;
    address public marketplace;

    ICore public core;
    uint256 public constant TRAIT_LOCK_FEE = 1 ether;

    struct Traits {
        uint256 packedTraits;
        bytes32 nameHash;
    }

    mapping(uint256 => Traits) public traits;

    event TraitsLocked(uint256 indexed tokenId, uint256 packedTraits, bytes32 nameHash);

    modifier onlyBridge() {
        require(msg.sender == bridge, "not bridge");
        _;
    }

    function setSeasonRegistry(address reg) external {
        require(address(seasons) == address(0), "already set");
        seasons = SeasonRegistry(reg);
    }

    function _maxBytes() internal view returns (uint256) {
        return address(seasons) != address(0) ? seasons.maxNetworkBytes() : DEFAULT_MAX_BYTES;
    }

    /**
     * @dev Hatch a new creature. One per wallet.
     */
    function hatch(bytes calldata genesisWeights) external returns (uint256 tokenId) {
        require(!hasHatched[msg.sender], "already hatched");
        require(genesisWeights.length <= _maxBytes(), "weights too large");
        tokenId = nextId++;
        _ownerOf[tokenId] = msg.sender;
        hasHatched[msg.sender] = true;

        bytes32 dna = keccak256(abi.encodePacked(blockhash(block.number - 1), msg.sender));
        creatures[tokenId] = Creature({genesisWeights: genesisWeights, dna: dna});
        emit CreatureHatched(msg.sender, tokenId, dna);
    }

    /**
     * @dev Return the owner of a token.
     */
    function ownerOf(uint256 tokenId) external view returns (address) {
        return _ownerOf[tokenId];
    }

    /**
     * @dev Approve an address to transfer the given token.
     */
    function approve(address to, uint256 tokenId) external {
        require(_ownerOf[tokenId] == msg.sender, "not owner");
        getApproved[tokenId] = to;
    }

    /**
     * @dev Transfer a token from one address to another.
     */
    function transferFrom(address from, address to, uint256 tokenId) external {
        require(_ownerOf[tokenId] == from, "wrong owner");
        require(msg.sender == from || msg.sender == getApproved[tokenId],
                "not authorized");
        _ownerOf[tokenId] = to;
        getApproved[tokenId] = address(0);
    }

    /**
     * @dev Set the marketplace contract allowed to mint via breeding.
     */
    function setMarketplace(address m) external {
        require(marketplace == address(0), "already set");
        marketplace = m;
    }

    function setBridge(address b) external {
        require(bridge == address(0), "already set");
        bridge = b;
    }

    function setCore(address c) external {
        require(address(core) == address(0), "already set");
        core = ICore(c);
    }

    /**
     * @dev Mint a new creature via breeding. Callable by Marketplace.
     */
    function breed(address to, bytes calldata genesisWeights)
        external
        returns (uint256 tokenId)
    {
        require(msg.sender == marketplace, "not marketplace");
        require(genesisWeights.length <= _maxBytes(), "weights too large");
        tokenId = nextId++;
        _ownerOf[tokenId] = to;
        bytes32 dna =
            keccak256(abi.encodePacked(blockhash(block.number - 1), to, tokenId));
        creatures[tokenId] = Creature({genesisWeights: genesisWeights, dna: dna});
        emit CreatureHatched(to, tokenId, dna);
    }

    function mintFromBridge(
        address to,
        uint256 tokenId,
        bytes calldata genesisWeights,
        bytes32 dna
    ) external onlyBridge {
        require(_ownerOf[tokenId] == address(0), "exists");
        require(genesisWeights.length <= _maxBytes(), "weights too large");
        if (tokenId >= nextId) nextId = tokenId + 1;
        _ownerOf[tokenId] = to;
        creatures[tokenId] = Creature({genesisWeights: genesisWeights, dna: dna});
    }

    function burnFromBridge(uint256 tokenId) external onlyBridge {
        require(_ownerOf[tokenId] == address(this), "not bridge held");
        _ownerOf[tokenId] = address(0);
        delete creatures[tokenId];
        delete getApproved[tokenId];
    }

    function lockTraits(uint256 tokenId, uint256 packedTraits, bytes32 nameHash) external {
        require(_ownerOf[tokenId] == msg.sender, "not owner");
        require(traits[tokenId].nameHash == bytes32(0), "locked");
        require(address(core) != address(0), "core unset");
        require(core.transferFrom(msg.sender, address(this), TRAIT_LOCK_FEE));
        traits[tokenId] = Traits({packedTraits: packedTraits, nameHash: nameHash});
        emit TraitsLocked(tokenId, packedTraits, nameHash);
    }
}
