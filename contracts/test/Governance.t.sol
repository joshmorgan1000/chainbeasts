// SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.19;

import "ds-test/test.sol";
import "../Governance.sol";
import "../Marketplace.sol";
import "../CreatureNFT.sol";
import "../SeasonRegistry.sol";
import "../Core.sol";

contract GovernanceHarness is Governance {
    uint256 public testTime;
    constructor(address coreToken) Governance(coreToken) {}
    function setTime(uint256 t) external { testTime = t; }
    function _now() internal view override returns (uint256) { return testTime == 0 ? block.timestamp : testTime; }
}

contract GovernanceTest is DSTest {
    GovernanceHarness gov;
    Marketplace market;
    CreatureNFT nft;
    SeasonRegistry seasons;
    Core core;

    function setUp() public {
        core = new Core();
        core.mint(address(this), 10 ether);
        gov = new GovernanceHarness(address(core));
        nft = new CreatureNFT();
        market = new Marketplace(address(nft), address(gov));
        nft.setMarketplace(address(market));
        seasons = new SeasonRegistry(address(gov));
    }

    function testExecuteProposal() public {
        // stake to gain voting power
        core.approve(address(gov), 2 ether);
        gov.stake(2 ether);

        // propose to change breed fee
        bytes memory data = abi.encodeWithSelector(market.setBreedFee.selector, 2 ether);
        uint256 id = gov.propose(address(market), data);

        // vote and advance time
        gov.vote(id, true);
        // advance time past voting period and timelock
        gov.setTime(block.timestamp + 3 days + 7 days);
        gov.execute(id);
        assertEq(market.breedFee(), 2 ether);
    }

    function testGovernanceHooks() public {
        core.approve(address(gov), 4 ether);
        gov.stake(4 ether);

        uint256 t = block.timestamp;
        uint256 period = 3 days + 7 days;

        // hook marketplace to governance
        gov.setTime(t);
        bytes memory hookMarket = abi.encodeWithSelector(market.setGovernanceHook.selector, address(gov));
        uint256 p1 = gov.propose(address(market), hookMarket);
        gov.vote(p1, true);
        t += period;
        gov.setTime(t);
        gov.execute(p1);

        // hook season registry to governance
        gov.setTime(t);
        bytes memory hookSeason = abi.encodeWithSelector(seasons.setGovernanceHook.selector, address(gov));
        uint256 p2 = gov.propose(address(seasons), hookSeason);
        gov.vote(p2, true);
        t += period;
        gov.setTime(t);
        gov.execute(p2);

        // update marketplace parameter to trigger hook
        gov.setTime(t);
        bytes memory upd = abi.encodeWithSelector(market.setBreedFee.selector, 5 ether);
        uint256 id = gov.propose(address(market), upd);
        gov.vote(id, true);
        t += period;
        gov.setTime(t);
        gov.execute(id);
        assertEq(gov.lastTarget(), address(market));
        assertEq(gov.lastSelector(), market.setBreedFee.selector);

        // update season registry parameter to trigger hook
        gov.setTime(t);
        bytes memory upd2 = abi.encodeWithSelector(seasons.setMaxNetworkBytes.selector, 99);
        uint256 id2 = gov.propose(address(seasons), upd2);
        gov.vote(id2, true);
        t += period;
        gov.setTime(t);
        gov.execute(id2);
        assertEq(gov.lastTarget(), address(seasons));
        assertEq(gov.lastSelector(), seasons.setMaxNetworkBytes.selector);
    }
}
