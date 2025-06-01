import TrainerView from './TrainerView';
import MarketplaceView from './MarketplaceView';
import ValidatorView from './ValidatorView';
import BattleNotifications from './BattleNotifications';
import MarketplaceNotifications from './MarketplaceNotifications';
import AnalyticsDashboard from './AnalyticsDashboard';
import PluginMarketplaceView from './PluginMarketplaceView';
import PoUWChainView from './PoUWChainView';
import BridgeView from './BridgeView';
import TournamentManageView from './TournamentManageView';
import CombatSimulatorView from './CombatSimulatorView';
import CustomizationView from './CustomizationView';

export default function App() {
  return (
    <div className="container">
      <CombatSimulatorView />
      <TrainerView />
      <CustomizationView />
      <MarketplaceView />
      <BridgeView />
      <TournamentManageView />
      <PluginMarketplaceView />
      <PoUWChainView />
      <ValidatorView />
      <BattleNotifications />
      <MarketplaceNotifications />
      <AnalyticsDashboard />
    </div>
  );
}
