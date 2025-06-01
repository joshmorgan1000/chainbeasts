// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";

import "../CreatureNFT.sol";
import "../Marketplace.sol";
import "../Matchmaker.sol";
import "../MatchLedger.sol";
import "../TrainingLedger.sol";
import "../Energy.sol";
import "../SeasonRegistry.sol";
import "../ProofVerifier.sol";
import "../ZkVerifier.sol";
import "../IGovernanceHook.sol";

contract User {
    function buy(Marketplace m, uint256 tokenId) external payable {
        m.buy{value: msg.value}(tokenId);
    }

    function rent(Marketplace m, uint256 tokenId) external payable {
        m.rent{value: msg.value}(tokenId);
    }
}

contract Hook is IGovernanceHook {
    address public target;
    bytes4 public selector;
    bytes public data;

    function onAction(address t, bytes4 s, bytes calldata d) external override {
        target = t;
        selector = s;
        data = d;
    }
}

contract ComprehensiveTest is DSTest {
    CreatureNFT nft;
    Marketplace market;
    MatchLedger ledger;
    Matchmaker matchmaker;
    TrainingLedger training;
    Energy energy;
    Core core;
    SeasonRegistry seasons;
    ZkVerifier zk;
    ProofVerifier verifier;
    User user;
    Hook hook;

    function setUp() public {
        nft = new CreatureNFT();
        market = new Marketplace(address(nft), address(this));
        nft.setMarketplace(address(market));
        ledger = new MatchLedger();
        matchmaker = new Matchmaker(address(ledger));
        energy = new Energy();
        core = new Core();
        zk = new ZkVerifier();
        verifier = new ProofVerifier(address(zk));
        seasons = new SeasonRegistry(address(this));
        training = new TrainingLedger(address(energy), address(core), address(verifier), address(seasons));
        nft.setSeasonRegistry(address(seasons));
        user = new User();
        hook = new Hook();
        market.setGovernanceHook(address(hook));
        seasons.setGovernanceHook(address(hook));
    }

    function testHatchOnce() public {
        uint256 id = nft.hatch("");
        assertEq(id, 1);
        bool success;
        try nft.hatch("") returns (uint256) {
            success = true;
        } catch {
            success = false;
        }
        assertTrue(!success);
    }

    function testListAndBuy() public {
        uint256 tokenId = nft.hatch("");
        nft.approve(address(market), tokenId);
        market.list(tokenId, 1 ether);
        user.buy{value: 1 ether}(market, tokenId);
        assertEq(nft.ownerOf(tokenId), address(user));
        (address seller, uint256 price) = market.listings(tokenId);
        assertEq(seller, address(0));
        assertEq(price, 0);
    }

    function testUpdateAndCancel() public {
        uint256 tokenId = nft.hatch("");
        nft.approve(address(market), tokenId);
        market.list(tokenId, 1 ether);
        market.updatePrice(tokenId, 2 ether);
        (, uint256 price) = market.listings(tokenId);
        assertEq(price, 2 ether);
        market.cancel(tokenId);
        (address seller,) = market.listings(tokenId);
        assertEq(seller, address(0));
    }

    function testBreed() public {
        uint256 a = nft.hatch("");
        uint256 b = nft.hatch("");
        nft.approve(address(market), a);
        nft.approve(address(market), b);
        uint256 fee = market.breedFee();
        uint256 c = market.breed{value: fee}(a, b, "");
        assertEq(c, 3);
        assertEq(nft.ownerOf(c), address(this));
    }

    function testLeaseAndRent() public {
        uint256 tokenId = nft.hatch("");
        market.listForLease(tokenId, 1 ether, 100);
        user.rent{value: 1 ether}(market, tokenId);
        Marketplace.Lease memory info = market.leases(tokenId);
        assertEq(info.renter, address(user));
        assertTrue(info.expiry > 0);
    }

    function testGovernanceAndFees() public {
        market.setBreedFee(2 ether);
        assertEq(market.breedFee(), 2 ether);
        market.setGovernance(address(user));
        assertEq(market.governance(), address(user));
    }

    function testMarketplaceHook() public {
        market.setBreedFee(3 ether);
        assertEq(hook.target(), address(market));
        assertEq(hook.selector(), market.setBreedFee.selector);
    }

    function testMatchmaker() public {
        matchmaker.enqueue(1, 2, 1, 1);
        matchmaker.enqueue(2, 1, 1, 1);
        MatchLedger.Result memory r = ledger.results(1);
        assertTrue(r.creatureA == 1 || r.creatureA == 2);
        assertTrue(r.creatureB == 1 || r.creatureB == 2);
        assertTrue(r.winnerId == 1 || r.winnerId == 2);
    }

    function testProofVerifier() public {
        bytes memory proof = bytes("root");
        bytes32 h = sha256(proof);
        verifier.submit_proof(h, proof);
        assertTrue(verifier.proven(h));
        assertEq(verifier.attestations(h), verifier.quorum());
        assertTrue(verifier.verify(h));
    }

    function testTrainingLedger() public {
        bytes32 root = sha256(bytes("proof"));
        uint32 spent = 5;
        training.submitCheckpoint(1, 0, root, 32, 1, false, spent, bytes32(0));
        verifier.submit_proof(root, bytes("proof"));
        training.finalizeCheckpoint(root);
        assertEq(energy.balanceOf(address(this)), spent);
        assertEq(
            core.balanceOf(address(this)),
            uint256(spent) * TrainingLedger.CORE_PER_ENERGY()
        );
    }

    function testEpochOrderEnforced() public {
        bytes32 r0 = sha256(bytes("r0"));
        training.submitCheckpoint(1, 0, r0, 32, 1, false, 0, bytes32(0));
        verifier.submit_proof(r0, bytes("r0"));
        training.finalizeCheckpoint(r0);

        bytes32 r2 = sha256(bytes("r2"));
        training.submitCheckpoint(1, 2, r2, 32, 1, false, 0, bytes32(0));
        verifier.submit_proof(r2, bytes("r2"));
        bool success;
        try training.finalizeCheckpoint(r2) {
            success = true;
        } catch {
            success = false;
        }
        assertTrue(!success);
    }

    function testSeasonRegistry() public {
        bytes32 key = keccak256("k");
        seasons.setTable(key, hex"deadbeef");
        bytes memory data = seasons.getTable(key);
        assertEq(data, hex"deadbeef");
        seasons.setGovernance(address(user));
    }

    function testSeasonHook() public {
        bytes32 key = keccak256("h");
        seasons.setTable(key, hex"01");
        assertEq(hook.target(), address(seasons));
        assertEq(hook.selector(), seasons.setTable.selector);
    }

    function testWeightLimit() public {
        seasons.setMaxNetworkBytes(1);
        bool success;
        try nft.hatch(hex"0102") returns (uint256) {
            success = true;
        } catch {
            success = false;
        }
        assertTrue(!success);
    }
}

