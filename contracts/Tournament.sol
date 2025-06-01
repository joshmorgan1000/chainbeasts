// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "./SeasonRegistry.sol";

/**
 * @title Tournament
 * @notice Minimal tournament manager using SeasonRegistry constants.
 */
contract Tournament {
    SeasonRegistry public immutable seasons;
    bytes32 public constant SEED_KEY = keccak256("tournament_seed");

    struct MatchInfo {
        address playerA;
        address playerB;
        uint256 seed;
    }

    uint256 public nextId = 1;
    mapping(uint256 => MatchInfo) public matches;

    event MatchCreated(
        uint256 indexed matchId,
        address indexed playerA,
        address indexed playerB,
        uint256 seed
    );

    struct Bracket {
        address[] players;
        uint256 prizePool;
        bool finished;
        address winner;
    }

    uint256 public nextBracketId;
    mapping(uint256 => Bracket) private _brackets;

    event BracketCreated(uint256 indexed bracketId, uint256 playerCount, uint256 prize);
    event BracketAdvanced(uint256 indexed bracketId, uint256 remaining);
    event PrizePaid(uint256 indexed bracketId, address indexed winner, uint256 amount);

    constructor(address seasonRegistry) {
        seasons = SeasonRegistry(seasonRegistry);
    }

    /**
     * @dev Create a match seeded from the SeasonRegistry table.
     */
    function create(address a, address b) external returns (uint256 id) {
        bytes memory data = seasons.getTable(SEED_KEY);
        require(data.length >= 32, "seed missing");
        uint256 seed;
        assembly {
            seed := mload(add(data, 32))
        }
        id = nextId++;
        matches[id] = MatchInfo(a, b, seed);
        emit MatchCreated(id, a, b, seed);
    }

    /**
     * @dev Create a new tournament bracket with the given players and prize.
     */
    function createBracket(address[] calldata players)
        external
        payable
        returns (uint256 id)
    {
        require(players.length > 1, "players");
        id = nextBracketId++;
        Bracket storage b = _brackets[id];
        b.prizePool = msg.value;
        for (uint256 i = 0; i < players.length; ++i) {
            b.players.push(players[i]);
        }
        emit BracketCreated(id, players.length, msg.value);
    }

    /**
     * @dev Advance a bracket by providing the winners of the current round.
     */
    function reportWinners(uint256 id, address[] calldata winners) external {
        Bracket storage b = _brackets[id];
        require(!b.finished, "finished");
        require(winners.length > 0, "empty");
        for (uint256 i = 0; i < winners.length; ++i) {
            bool found = false;
            for (uint256 j = 0; j < b.players.length && !found; ++j) {
                if (b.players[j] == winners[i]) found = true;
            }
            require(found, "invalid winner");
        }
        delete b.players;
        for (uint256 i = 0; i < winners.length; ++i) {
            b.players.push(winners[i]);
        }
        emit BracketAdvanced(id, winners.length);
        if (winners.length == 1) {
            b.finished = true;
            b.winner = winners[0];
            uint256 prize = b.prizePool;
            b.prizePool = 0;
            if (prize > 0) {
                payable(winners[0]).transfer(prize);
                emit PrizePaid(id, winners[0], prize);
            }
        }
    }

    /** Return players remaining in a bracket. */
    function bracketPlayers(uint256 id) external view returns (address[] memory) {
        return _brackets[id].players;
    }

    /** Return the winner of a finished bracket. */
    function bracketWinner(uint256 id) external view returns (address) {
        return _brackets[id].winner;
    }
}
