import { useState, useEffect } from "react";

function App() {
  const [data, setData] = useState(null);

  useEffect(() => {
    const fetchStats = async () => {
      try {
        const res = await fetch("http://localhost:9090");
        const json = await res.json();
        setData(json);
      } catch (e) {
        console.error("Cannot connect to load balancer");
      }
    };

    fetchStats();
    const interval = setInterval(fetchStats, 1000);
    return () => clearInterval(interval);
  }, []);

  if (!data) return (
    <div style={styles.loading}>
      Connecting to Load Balancer...
    </div>
  );

  return (
    <div style={styles.container}>
      <h1 style={styles.title}>Load Balancer Monitor</h1>
      
      <div style={styles.card}>
        <div style={styles.row}>
          <span style={styles.label}>Algorithm</span>
          <span style={styles.badge}>{data.algorithm}</span>
        </div>
        <div style={styles.row}>
          <span style={styles.label}>Status</span>
          <span style={{...styles.badge, background: "#22c55e"}}>RUNNING</span>
        </div>
      </div>

      <h2 style={styles.subtitle}>Backend Servers</h2>

      {data.servers.map((s, i) => (
        <div key={i} style={{
          ...styles.serverCard,
          borderLeft: `4px solid ${s.alive ? "#22c55e" : "#ef4444"}`
        }}>
          <div style={styles.serverRow}>
            <span style={styles.serverName}>
              {s.alive ? "🟢" : "🔴"} {s.host}:{s.port}
            </span>
            <span style={styles.connections}>
              {s.connections} active connections
            </span>
          </div>
          <div style={styles.statusText}>
            {s.alive ? "ONLINE" : "OFFLINE"}
          </div>
        </div>
      ))}

      <div style={styles.footer}>
        Auto-refreshing every 1 second
      </div>
    </div>
  );
}

const styles = {
  container: {
    maxWidth: "600px",
    margin: "40px auto",
    fontFamily: "'Segoe UI', sans-serif",
    padding: "0 20px",
    background: "#0f172a",
    minHeight: "100vh",
    color: "#f1f5f9"
  },
  title: {
    fontSize: "28px",
    fontWeight: "700",
    color: "#38bdf8",
    marginBottom: "24px",
    paddingTop: "40px"
  },
  subtitle: {
    fontSize: "18px",
    color: "#94a3b8",
    margin: "24px 0 12px"
  },
  card: {
    background: "#1e293b",
    borderRadius: "12px",
    padding: "20px",
    marginBottom: "16px"
  },
  row: {
    display: "flex",
    justifyContent: "space-between",
    alignItems: "center",
    marginBottom: "12px"
  },
  label: {
    color: "#94a3b8",
    fontSize: "14px"
  },
  badge: {
    background: "#38bdf8",
    color: "#0f172a",
    padding: "4px 12px",
    borderRadius: "20px",
    fontSize: "13px",
    fontWeight: "600"
  },
  serverCard: {
    background: "#1e293b",
    borderRadius: "12px",
    padding: "16px 20px",
    marginBottom: "12px"
  },
  serverRow: {
    display: "flex",
    justifyContent: "space-between",
    alignItems: "center"
  },
  serverName: {
    fontSize: "16px",
    fontWeight: "600"
  },
  connections: {
    color: "#94a3b8",
    fontSize: "13px"
  },
  statusText: {
    marginTop: "8px",
    fontSize: "12px",
    color: "#64748b"
  },
  loading: {
    color: "#38bdf8",
    fontFamily: "sans-serif",
    textAlign: "center",
    marginTop: "100px",
    fontSize: "18px"
  },
  footer: {
    textAlign: "center",
    color: "#475569",
    fontSize: "12px",
    marginTop: "32px",
    paddingBottom: "40px"
  }
};

export default App;