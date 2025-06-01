// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "./CreatureNFT.sol";
import "./IGovernanceHook.sol";

/**
 * @title Marketplace
 * @notice Simple on-chain marketplace to trade, breed and lease creatures.
 */
contract Marketplace {
    struct Listing {
        address seller;
        uint256 price;
    }

    CreatureNFT public immutable nft;
    address public governance;
    address public governanceHook;
    uint256 public breedFee = 1 ether;
    mapping(uint256 => Listing) public listings;

    struct Lease {
        address owner;
        uint256 price;
        uint256 duration;
        address renter;
        uint256 expiry;
    }

    mapping(uint256 => Lease) public leases;

    mapping(uint256 => address) public itemOwner;
    mapping(uint256 => uint256) public itemAttachedTo;

    event ItemRegistered(uint256 indexed itemId, address indexed owner);
    event ItemAttached(uint256 indexed creatureId, uint256 indexed itemId);
    event ItemDetached(uint256 indexed creatureId, uint256 indexed itemId);

    event Listed(uint256 indexed tokenId, address indexed seller, uint256 price);
    event Cancelled(uint256 indexed tokenId);
    event Purchased(uint256 indexed tokenId, address indexed buyer, uint256 price);
    event Bred(uint256 indexed parentA, uint256 indexed parentB, uint256 offspring);
    event BreedFeeUpdated(uint256 newFee);
    event PriceUpdated(uint256 indexed tokenId, uint256 newPrice);
    event LeaseListed(uint256 indexed tokenId, address indexed owner, uint256 price, uint256 duration);
    event LeaseCancelled(uint256 indexed tokenId);
    event Rented(uint256 indexed tokenId, address indexed renter, uint256 expiry);
    event LeaseEnded(uint256 indexed tokenId);
    event GovernanceUpdated(address newGovernance);
    event GovernanceHookUpdated(address newHook);

    constructor(address nftAddress, address governanceAddr) {
        nft = CreatureNFT(nftAddress);
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

    /** Update the price of an existing listing. */
    function updatePrice(uint256 tokenId, uint256 newPrice) external {
        Listing storage l = listings[tokenId];
        require(l.seller == msg.sender, "not seller");
        l.price = newPrice;
        emit PriceUpdated(tokenId, newPrice);
    }

    /** Set the breeding fee charged for new creatures. */
    function setBreedFee(uint256 fee) external onlyGovernance {
        breedFee = fee;
        emit BreedFeeUpdated(fee);
        _notifyHook(Marketplace.setBreedFee.selector, abi.encode(fee));
    }

    function setGovernance(address newGovernance) external onlyGovernance {
        governance = newGovernance;
        emit GovernanceUpdated(newGovernance);
        _notifyHook(Marketplace.setGovernance.selector, abi.encode(newGovernance));
    }

    /** Register a new item to the caller. */
    function registerItem(uint256 itemId) external {
        require(itemOwner[itemId] == address(0), "exists");
        itemOwner[itemId] = msg.sender;
        emit ItemRegistered(itemId, msg.sender);
    }

    /** Attach an owned item to a creature. */
    function attachItem(uint256 creatureId, uint256 itemId) external {
        require(nft.ownerOf(creatureId) == msg.sender, "not owner");
        require(itemOwner[itemId] == msg.sender, "not item owner");
        require(itemAttachedTo[itemId] == 0, "attached");
        itemAttachedTo[itemId] = creatureId;
        emit ItemAttached(creatureId, itemId);
    }

    /** Detach an item from a creature. */
    function detachItem(uint256 creatureId, uint256 itemId) external {
        require(nft.ownerOf(creatureId) == msg.sender, "not owner");
        require(itemAttachedTo[itemId] == creatureId, "not attached");
        itemAttachedTo[itemId] = 0;
        emit ItemDetached(creatureId, itemId);
    }

    /** List a creature for sale. Caller must own the token and approve this contract. */
    function list(uint256 tokenId, uint256 price) external {
        require(nft.ownerOf(tokenId) == msg.sender, "not owner");
        listings[tokenId] = Listing({seller: msg.sender, price: price});
        emit Listed(tokenId, msg.sender, price);
    }

    /** Cancel an existing listing. */
    function cancel(uint256 tokenId) external {
        Listing memory l = listings[tokenId];
        require(l.seller == msg.sender, "not seller");
        delete listings[tokenId];
        emit Cancelled(tokenId);
    }

    /** Purchase a listed creature. */
    function buy(uint256 tokenId) external payable {
        Listing memory l = listings[tokenId];
        require(l.price > 0, "not listed");
        require(msg.value >= l.price, "price not met");
        delete listings[tokenId];
        nft.transferFrom(l.seller, msg.sender, tokenId);
        payable(l.seller).transfer(l.price);
        if (msg.value > l.price) payable(msg.sender).transfer(msg.value - l.price);
        emit Purchased(tokenId, msg.sender, l.price);
    }

    /** Breed two creatures owned by the caller, paying the breed fee. */
    function breed(
        uint256 parentA,
        uint256 parentB,
        bytes calldata weights
    ) external payable returns (uint256 tokenId) {
        require(nft.ownerOf(parentA) == msg.sender, "not owner A");
        require(nft.ownerOf(parentB) == msg.sender, "not owner B");
        require(msg.value >= breedFee, "fee");
        tokenId = nft.breed(msg.sender, weights);
        emit Bred(parentA, parentB, tokenId);
        if (msg.value > breedFee) payable(msg.sender).transfer(msg.value - breedFee);
    }

    /** List a creature for lease. Caller must own the token. */
    function listForLease(
        uint256 tokenId,
        uint256 price,
        uint256 duration
    ) external {
        require(nft.ownerOf(tokenId) == msg.sender, "not owner");
        Lease storage l = leases[tokenId];
        require(
            l.owner == address(0) || (l.renter == address(0) && block.timestamp > l.expiry),
            "already listed"
        );
        leases[tokenId] = Lease({owner: msg.sender, price: price, duration: duration, renter: address(0), expiry: 0});
        emit LeaseListed(tokenId, msg.sender, price, duration);
    }

    /** Cancel a lease listing. */
    function cancelLease(uint256 tokenId) external {
        Lease memory l = leases[tokenId];
        require(l.owner == msg.sender, "not owner");
        require(l.renter == address(0) || block.timestamp > l.expiry, "active");
        delete leases[tokenId];
        emit LeaseCancelled(tokenId);
    }

    /** Rent a creature for the specified duration. */
    function rent(uint256 tokenId) external payable {
        Lease storage l = leases[tokenId];
        require(l.owner != address(0), "not listed");
        if (l.renter != address(0) && block.timestamp > l.expiry) {
            emit LeaseEnded(tokenId);
            l.renter = address(0);
        }
        require(l.renter == address(0), "already rented");
        require(msg.value >= l.price, "price not met");
        l.renter = msg.sender;
        l.expiry = block.timestamp + l.duration;
        payable(l.owner).transfer(l.price);
        if (msg.value > l.price) payable(msg.sender).transfer(msg.value - l.price);
        emit Rented(tokenId, msg.sender, l.expiry);
    }

    /** Return lease info, clearing expired rentals in view. */
    function getLease(uint256 tokenId) external view returns (Lease memory info) {
        info = leases[tokenId];
        if (info.renter != address(0) && block.timestamp > info.expiry) {
            info.renter = address(0);
            info.expiry = 0;
        }
    }

    function _notifyHook(bytes4 selector, bytes memory data) internal {
        if (governanceHook != address(0)) {
            try IGovernanceHook(governanceHook).onAction(address(this), selector, data) {
            } catch {}
        }
    }
}
