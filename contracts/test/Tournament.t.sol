// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../Tournament.sol";
import "../SeasonRegistry.sol";
import "../ProofVerifier.sol";
import "../ZkVerifier.sol";

contract TournamentTest is DSTest {
    Tournament tour;
    SeasonRegistry seasons;
    ZkVerifier zk;
    ProofVerifier verifier;

    function setUp() public {
        seasons = new SeasonRegistry(address(this));
        seasons.setTable(Tournament.SEED_KEY, abi.encode(uint256(42)));
        tour = new Tournament(address(seasons));
        zk = new ZkVerifier();
        verifier = new ProofVerifier(address(zk));
    }

    function testCreateMatchUsesSeed() public {
        uint256 id = tour.create(address(1), address(2));
        (address a, address b, uint256 seed) = tour.matches(id);
        assertEq(a, address(1));
        assertEq(b, address(2));
        assertEq(seed, 42);
    }

    function testSubmitProof() public {
        bytes memory proof = bytes("root");
        bytes32 h = sha256(proof);
        verifier.submit_proof(h, proof);
        assertTrue(verifier.proven(h));
        assertEq(verifier.attestations(h), verifier.quorum());
    }

    function testBracketFlow() public {
        address[] memory players = new address[](4);
        players[0] = address(1);
        players[1] = address(2);
        players[2] = address(3);
        players[3] = address(4);
        uint256 id = tour.createBracket{value: 1 ether}(players);
        address[] memory current = tour.bracketPlayers(id);
        assertEq(current.length, 4);

        address[] memory winners = new address[](2);
        winners[0] = address(1);
        winners[1] = address(3);
        tour.reportWinners(id, winners);
        current = tour.bracketPlayers(id);
        assertEq(current.length, 2);

        address[] memory finalW = new address[](1);
        finalW[0] = address(3);
        uint256 before = address(3).balance;
        tour.reportWinners(id, finalW);
        assertEq(tour.bracketWinner(id), address(3));
        assertEq(address(3).balance, before + 1 ether);
        assertEq(address(tour).balance, 0);
    }
}
