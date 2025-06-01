// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

/**
 * @title Governance
 * @notice Minimal DAO using staked $CORE for voting power.
 *         Proposals execute calls on target contracts after a timelock.
 */
import "./ICore.sol";
import "./IGovernanceHook.sol";

contract Governance is IGovernanceHook {
    struct Proposal {
        address target;
        bytes data;
        uint256 voteEnd;
        uint256 yes;
        uint256 no;
        bool executed;
    }

    uint256 public constant VOTING_PERIOD = 3 days;
    uint256 public constant TIMELOCK = 7 days;
    uint256 public constant QUORUM = 1 ether;

    ICore public immutable core;

    mapping(address => uint256) public stake;
    uint256 public totalStake;

    mapping(uint256 => Proposal) public proposals;
    uint256 public nextProposalId;

    event Staked(address indexed user, uint256 amount);
    event Withdrawn(address indexed user, uint256 amount);
    event Proposed(uint256 indexed id, address indexed proposer, address target);
    event Voted(uint256 indexed id, address indexed voter, bool support, uint256 weight);
    event Executed(uint256 indexed id);
    event ActionNotified(address indexed target, bytes4 indexed selector);

    address public lastTarget;
    bytes4 public lastSelector;
    bytes public lastData;

    /** Internal helper returning current time. Split for testing. */
    function _now() internal view virtual returns (uint256) {
        return block.timestamp;
    }

    constructor(address coreToken) {
        core = ICore(coreToken);
    }

    /** Deposit CORE to gain voting power. */
    function stake(uint256 amount) external {
        require(amount > 0, "amount");
        require(core.transferFrom(msg.sender, address(this), amount));
        stake[msg.sender] += amount;
        totalStake += amount;
        emit Staked(msg.sender, amount);
    }

    /** Withdraw previously staked CORE. */
    function withdraw(uint256 amount) external {
        require(stake[msg.sender] >= amount, "balance");
        stake[msg.sender] -= amount;
        totalStake -= amount;
        require(core.transfer(msg.sender, amount));
        emit Withdrawn(msg.sender, amount);
    }

    /** Create a new proposal to call a target contract. */
    function propose(address target, bytes calldata data) external returns (uint256 id) {
        require(stake[msg.sender] > 0, "stake");
        id = nextProposalId++;
        proposals[id] = Proposal({
            target: target,
            data: data,
            voteEnd: _now() + VOTING_PERIOD,
            yes: 0,
            no: 0,
            executed: false
        });
        emit Proposed(id, msg.sender, target);
    }

    /** Vote for or against a proposal. Weight equals current stake. */
    function vote(uint256 id, bool support) external {
        Proposal storage p = proposals[id];
        require(_now() < p.voteEnd, "ended");
        uint256 weight = stake[msg.sender];
        require(weight > 0, "no stake");
        if (support) {
            p.yes += weight;
        } else {
            p.no += weight;
        }
        emit Voted(id, msg.sender, support, weight);
    }

    /** Execute a successful proposal after the timelock. */
    function execute(uint256 id) external {
        Proposal storage p = proposals[id];
        require(!p.executed, "done");
        require(_now() >= p.voteEnd + TIMELOCK, "timelock");
        require(p.yes > p.no && p.yes >= QUORUM, "failed");
        p.executed = true;
        (bool ok, ) = p.target.call(p.data);
        require(ok, "call failed");
        emit Executed(id);
    }

    /** Record governance controlled actions executed by hooked contracts. */
    function onAction(address t, bytes4 s, bytes calldata d) external override {
        lastTarget = t;
        lastSelector = s;
        lastData = d;
        emit ActionNotified(t, s);
    }
}
