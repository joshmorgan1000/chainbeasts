// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../CreatureNFT.sol";
import "../CreatureNFTBridge.sol";
import "../Tournament.sol";
import "../SeasonRegistry.sol";

contract BridgeTournamentTest is DSTest {
    CreatureNFT nftA;
    CreatureNFT nftB;
    CreatureNFTBridge bridgeA;
    CreatureNFTBridge bridgeB;
    SeasonRegistry seasons;
    Tournament tour;

    function setUp() public {
        nftA = new CreatureNFT();
        nftB = new CreatureNFT();
        bridgeA = new CreatureNFTBridge(address(nftA));
        bridgeB = new CreatureNFTBridge(address(nftB));
        nftA.setBridge(address(bridgeA));
        nftB.setBridge(address(bridgeB));
        seasons = new SeasonRegistry(address(this));
        seasons.setTable(Tournament.SEED_KEY, abi.encode(uint256(7)));
        tour = new Tournament(address(seasons));
    }

    function testBridgeAndTournamentFlow() public {
        uint256 tokenId = nftA.hatch("");
        (bytes memory w, bytes32 dna) = nftA.creatures(tokenId);
        nftA.approve(address(bridgeA), tokenId);
        bridgeA.bridgeOut(tokenId, 2);
        bridgeB.bridgeIn(tokenId, address(this), 1, w, dna);
        assertEq(nftB.ownerOf(tokenId), address(this));

        address[] memory players = new address[](2);
        players[0] = address(1);
        players[1] = address(2);
        uint256 id = tour.createBracket{value: 1 ether}(players);
        address[] memory current = tour.bracketPlayers(id);
        assertEq(current.length, 2);
        address[] memory winners = new address[](1);
        winners[0] = address(2);
        tour.reportWinners(id, winners);
        assertEq(tour.bracketWinner(id), address(2));
    }
}
