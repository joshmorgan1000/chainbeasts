import MetricsChart from './MetricsChart';
import TournamentResults from './TournamentResults';

/**
 * Dashboard view showing recent training metrics and battle results.
 */
export default function AnalyticsDashboard() {
  return (
    <div>
      <h2>Analytics Dashboard</h2>
      <MetricsChart />
      <TournamentResults />
    </div>
  );
}
