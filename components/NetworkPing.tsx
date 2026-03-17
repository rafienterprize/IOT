"use client";

import { useEffect, useState } from "react";
import { Wifi, WifiOff, Activity, Clock } from "lucide-react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => (() => void) | void;
  publish: (topic: string, message: string) => void;
}

interface PingData {
  timestamp: string;
  latency: number;
  status: "success" | "timeout" | "error";
}

export default function NetworkPing({ subscribe, publish }: Props) {
  const [pingHistory, setPingHistory] = useState<PingData[]>([]);
  const [currentLatency, setCurrentLatency] = useState<number | null>(null);
  const [isOnline, setIsOnline] = useState(true);
  const [isPinging, setIsPinging] = useState(false);

  useEffect(() => {
    const unsubscribe = subscribe("iot/network/ping", (message) => {
      try {
        const data = JSON.parse(message);
        const pingData: PingData = {
          timestamp: new Date().toLocaleTimeString(),
          latency: data.latency || 0,
          status: data.status || "success"
        };
        
        setPingHistory(prev => [pingData, ...prev.slice(0, 9)]); // Keep last 10
        setCurrentLatency(data.latency);
        setIsOnline(data.status === "success");
      } catch {
        // Handle plain text response
        const latency = parseFloat(message);
        if (!isNaN(latency)) {
          const pingData: PingData = {
            timestamp: new Date().toLocaleTimeString(),
            latency: latency,
            status: "success"
          };
          setPingHistory(prev => [pingData, ...prev.slice(0, 9)]);
          setCurrentLatency(latency);
          setIsOnline(true);
        }
      }
    });

    return () => {
      if (unsubscribe) unsubscribe();
    };
  }, [subscribe]);

  const sendPing = () => {
    setIsPinging(true);
    publish("iot/network/command", "PING");
    
    // Reset isPinging after 3 seconds
    setTimeout(() => setIsPinging(false), 3000);
  };

  const getLatencyColor = (latency: number) => {
    if (latency < 50) return "text-green-600";
    if (latency < 100) return "text-yellow-600";
    return "text-red-600";
  };

  const getStatusIcon = (status: string) => {
    switch (status) {
      case "success":
        return <Wifi className="w-4 h-4 text-green-500" />;
      case "timeout":
        return <Clock className="w-4 h-4 text-yellow-500" />;
      default:
        return <WifiOff className="w-4 h-4 text-red-500" />;
    }
  };

  return (
    <div className="space-y-6">
      {/* Current Status */}
      <div className="bg-white rounded-xl shadow-lg p-6">
        <div className="flex items-center justify-between mb-4">
          <h3 className="text-lg font-bold text-gray-800">Network Status</h3>
          <div className="flex items-center gap-2">
            {isOnline ? (
              <Wifi className="w-5 h-5 text-green-500" />
            ) : (
              <WifiOff className="w-5 h-5 text-red-500" />
            )}
            <span className={`text-sm font-medium ${isOnline ? 'text-green-600' : 'text-red-600'}`}>
              {isOnline ? 'Online' : 'Offline'}
            </span>
          </div>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mb-4">
          <div className="text-center p-4 bg-gray-50 rounded-lg">
            <div className="text-2xl font-bold text-gray-800">
              {currentLatency !== null ? `${currentLatency}ms` : '-'}
            </div>
            <div className="text-sm text-gray-600">Current Latency</div>
          </div>
          <div className="text-center p-4 bg-gray-50 rounded-lg">
            <div className="text-2xl font-bold text-gray-800">
              {pingHistory.length}
            </div>
            <div className="text-sm text-gray-600">Total Pings</div>
          </div>
        </div>

        <button
          onClick={sendPing}
          disabled={isPinging}
          className={`w-full px-4 py-2 rounded-lg font-semibold transition-colors ${
            isPinging 
              ? 'bg-gray-400 text-white cursor-not-allowed' 
              : 'bg-indigo-600 text-white hover:bg-indigo-700'
          }`}
        >
          {isPinging ? (
            <div className="flex items-center justify-center gap-2">
              <Activity className="w-4 h-4 animate-spin" />
              Pinging...
            </div>
          ) : (
            'Send Ping'
          )}
        </button>
      </div>

      {/* Ping History */}
      <div className="bg-white rounded-xl shadow-lg p-6">
        <h3 className="text-lg font-bold text-gray-800 mb-4">Ping History</h3>
        
        {pingHistory.length === 0 ? (
          <div className="text-center py-8 text-gray-500">
            No ping data available. Click "Send Ping" to start monitoring.
          </div>
        ) : (
          <div className="space-y-2">
            {pingHistory.map((ping, index) => (
              <div key={index} className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
                <div className="flex items-center gap-3">
                  {getStatusIcon(ping.status)}
                  <span className="text-sm text-gray-600">{ping.timestamp}</span>
                </div>
                <div className={`font-semibold ${getLatencyColor(ping.latency)}`}>
                  {ping.status === "success" ? `${ping.latency}ms` : ping.status}
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}