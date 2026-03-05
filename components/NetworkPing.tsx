"use client";

import { useState, useEffect } from "react";
import { Activity, Wifi, TrendingUp } from "lucide-react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => void;
  publish: (topic: string, message: string) => void;
}

interface PingData {
  esp32_1: number | null;
  esp32_2: number | null;
  esp32_3: number | null;
  esp32_4: number | null;
  lastPing1: Date | null;
  lastPing2: Date | null;
  lastPing3: Date | null;
  lastPing4: Date | null;
}

interface PingHistory {
  esp32_1: number[];
  esp32_2: number[];
  esp32_3: number[];
  esp32_4: number[];
}

export default function NetworkPing({ subscribe, publish }: Props) {
  const [pingData, setPingData] = useState<PingData>({
    esp32_1: null,
    esp32_2: null,
    esp32_3: null,
    esp32_4: null,
    lastPing1: null,
    lastPing2: null,
    lastPing3: null,
    lastPing4: null,
  });

  const [pingSent, setPingSent] = useState({
    esp32_1: 0,
    esp32_2: 0,
    esp32_3: 0,
    esp32_4: 0,
  });

  const [pingHistory, setPingHistory] = useState<PingHistory>({
    esp32_1: [],
    esp32_2: [],
    esp32_3: [],
    esp32_4: [],
  });

  const MAX_HISTORY = 20; // Keep last 20 pings

  useEffect(() => {
    // Subscribe to ping responses
    const unsubscribe1 = subscribe("iot/esp32_1/pong", () => {
      const now = Date.now();
      const latency = now - pingSent.esp32_1;
      setPingData(prev => ({
        ...prev,
        esp32_1: latency,
        lastPing1: new Date(),
      }));
      setPingHistory(prev => ({
        ...prev,
        esp32_1: [...prev.esp32_1, latency].slice(-MAX_HISTORY),
      }));
    });

    const unsubscribe2 = subscribe("iot/esp32_2/pong", () => {
      const now = Date.now();
      const latency = now - pingSent.esp32_2;
      setPingData(prev => ({
        ...prev,
        esp32_2: latency,
        lastPing2: new Date(),
      }));
      setPingHistory(prev => ({
        ...prev,
        esp32_2: [...prev.esp32_2, latency].slice(-MAX_HISTORY),
      }));
    });

    const unsubscribe3 = subscribe("iot/esp32_3/pong", () => {
      const now = Date.now();
      const latency = now - pingSent.esp32_3;
      setPingData(prev => ({
        ...prev,
        esp32_3: latency,
        lastPing3: new Date(),
      }));
      setPingHistory(prev => ({
        ...prev,
        esp32_3: [...prev.esp32_3, latency].slice(-MAX_HISTORY),
      }));
    });

    const unsubscribe4 = subscribe("iot/esp32_4/pong", () => {
      const now = Date.now();
      const latency = now - pingSent.esp32_4;
      setPingData(prev => ({
        ...prev,
        esp32_4: latency,
        lastPing4: new Date(),
      }));
      setPingHistory(prev => ({
        ...prev,
        esp32_4: [...prev.esp32_4, latency].slice(-MAX_HISTORY),
      }));
    });

    return () => {
      if (unsubscribe1) unsubscribe1();
      if (unsubscribe2) unsubscribe2();
      if (unsubscribe3) unsubscribe3();
      if (unsubscribe4) unsubscribe4();
    };
  }, [subscribe, pingSent]);

  const sendPing = (deviceId: string) => {
    const now = Date.now();
    setPingSent(prev => ({ ...prev, [deviceId]: now }));
    publish(`iot/${deviceId}/ping`, "PING");
  };

  const pingAll = () => {
    sendPing("esp32_1");
    setTimeout(() => sendPing("esp32_2"), 100);
    setTimeout(() => sendPing("esp32_3"), 200);
    setTimeout(() => sendPing("esp32_4"), 300);
  };

  const getPingColor = (ping: number | null) => {
    if (ping === null) return "text-gray-400";
    if (ping < 100) return "text-green-600";
    if (ping < 300) return "text-yellow-600";
    return "text-red-600";
  };

  const getPingStatus = (ping: number | null) => {
    if (ping === null) return "No data";
    if (ping < 100) return "Excellent";
    if (ping < 300) return "Good";
    return "Poor";
  };

  const getAverage = (history: number[]) => {
    if (history.length === 0) return 0;
    const sum = history.reduce((a, b) => a + b, 0);
    return Math.round(sum / history.length);
  };

  const renderChart = (history: number[], color: string, label: string) => {
    if (history.length === 0) {
      return (
        <div className="h-32 flex items-center justify-center text-gray-400 text-sm">
          No data yet. Click Ping to start.
        </div>
      );
    }

    const maxValue = Math.max(...history, 400);
    const points = history.map((value, index) => {
      const x = (index / (MAX_HISTORY - 1)) * 100;
      const y = 100 - (value / maxValue) * 100;
      return `${x},${y}`;
    }).join(' ');

    return (
      <div className="relative h-32">
        <svg className="w-full h-full" viewBox="0 0 100 100" preserveAspectRatio="none">
          {/* Grid lines */}
          <line x1="0" y1="25" x2="100" y2="25" stroke="#e5e7eb" strokeWidth="0.5" />
          <line x1="0" y1="50" x2="100" y2="50" stroke="#e5e7eb" strokeWidth="0.5" />
          <line x1="0" y1="75" x2="100" y2="75" stroke="#e5e7eb" strokeWidth="0.5" />
          
          {/* Area under line */}
          <polygon
            points={`0,100 ${points} 100,100`}
            fill={color}
            opacity="0.2"
          />
          
          {/* Line */}
          <polyline
            points={points}
            fill="none"
            stroke={color}
            strokeWidth="2"
            strokeLinecap="round"
            strokeLinejoin="round"
          />
          
          {/* Points */}
          {history.map((value, index) => {
            const x = (index / (MAX_HISTORY - 1)) * 100;
            const y = 100 - (value / maxValue) * 100;
            return (
              <circle
                key={index}
                cx={x}
                cy={y}
                r="1.5"
                fill={color}
              />
            );
          })}
        </svg>
        
        {/* Y-axis labels */}
        <div className="absolute left-0 top-0 text-xs text-gray-400">0ms</div>
        <div className="absolute left-0 bottom-0 text-xs text-gray-400">{maxValue}ms</div>
        
        {/* Stats */}
        <div className="absolute right-0 top-0 text-xs bg-white px-2 py-1 rounded shadow">
          <div className="text-gray-600">Avg: <span className="font-semibold">{getAverage(history)}ms</span></div>
          <div className="text-gray-600">Min: <span className="font-semibold">{Math.min(...history)}ms</span></div>
          <div className="text-gray-600">Max: <span className="font-semibold">{Math.max(...history)}ms</span></div>
        </div>
      </div>
    );
  };

  return (
    <div className="space-y-4">
      <div className="bg-white rounded-xl shadow-lg p-6">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-3">
            <Activity className="w-6 h-6 text-indigo-600" />
            <h3 className="text-lg font-bold text-gray-800">Network Latency</h3>
          </div>
          <button
            onClick={pingAll}
            className="px-4 py-2 bg-indigo-500 text-white rounded-lg hover:bg-indigo-600 flex items-center gap-2 text-sm font-semibold"
          >
            <Wifi className="w-4 h-4" />
            Ping All
          </button>
        </div>

        <div className="space-y-3">
          {/* ESP32 1 */}
          <div className="bg-gradient-to-r from-blue-50 to-blue-100 rounded-lg p-4">
            <div className="flex items-center justify-between mb-2">
              <div>
                <div className="font-semibold text-gray-800">ESP32 1</div>
                <div className="text-xs text-gray-600">Lamp, Gas, Feeder</div>
              </div>
              <button
                onClick={() => sendPing("esp32_1")}
                className="px-3 py-1 bg-blue-500 text-white rounded text-xs hover:bg-blue-600"
              >
                Ping
              </button>
            </div>
            <div className="flex items-center justify-between">
              <div className={`text-3xl font-bold ${getPingColor(pingData.esp32_1)}`}>
                {pingData.esp32_1 !== null ? `${pingData.esp32_1}ms` : "---"}
              </div>
              <div className="text-right">
                <div className={`text-sm font-semibold ${getPingColor(pingData.esp32_1)}`}>
                  {getPingStatus(pingData.esp32_1)}
                </div>
                {pingData.lastPing1 && (
                  <div className="text-xs text-gray-500">
                    {pingData.lastPing1.toLocaleTimeString('id-ID')}
                  </div>
                )}
              </div>
            </div>
          </div>

          {/* ESP32 2 */}
          <div className="bg-gradient-to-r from-green-50 to-green-100 rounded-lg p-4">
            <div className="flex items-center justify-between mb-2">
              <div>
                <div className="font-semibold text-gray-800">ESP32 2</div>
                <div className="text-xs text-gray-600">Trash, Clothesline</div>
              </div>
              <button
                onClick={() => sendPing("esp32_2")}
                className="px-3 py-1 bg-green-500 text-white rounded text-xs hover:bg-green-600"
              >
                Ping
              </button>
            </div>
            <div className="flex items-center justify-between">
              <div className={`text-3xl font-bold ${getPingColor(pingData.esp32_2)}`}>
                {pingData.esp32_2 !== null ? `${pingData.esp32_2}ms` : "---"}
              </div>
              <div className="text-right">
                <div className={`text-sm font-semibold ${getPingColor(pingData.esp32_2)}`}>
                  {getPingStatus(pingData.esp32_2)}
                </div>
                {pingData.lastPing2 && (
                  <div className="text-xs text-gray-500">
                    {pingData.lastPing2.toLocaleTimeString('id-ID')}
                  </div>
                )}
              </div>
            </div>
          </div>

          {/* ESP32 3 */}
          <div className="bg-gradient-to-r from-purple-50 to-purple-100 rounded-lg p-4">
            <div className="flex items-center justify-between mb-2">
              <div>
                <div className="font-semibold text-gray-800">ESP32 3</div>
                <div className="text-xs text-gray-600">Smart Door Lock</div>
              </div>
              <button
                onClick={() => sendPing("esp32_3")}
                className="px-3 py-1 bg-purple-500 text-white rounded text-xs hover:bg-purple-600"
              >
                Ping
              </button>
            </div>
            <div className="flex items-center justify-between">
              <div className={`text-3xl font-bold ${getPingColor(pingData.esp32_3)}`}>
                {pingData.esp32_3 !== null ? `${pingData.esp32_3}ms` : "---"}
              </div>
              <div className="text-right">
                <div className={`text-sm font-semibold ${getPingColor(pingData.esp32_3)}`}>
                  {getPingStatus(pingData.esp32_3)}
                </div>
                {pingData.lastPing3 && (
                  <div className="text-xs text-gray-500">
                    {pingData.lastPing3.toLocaleTimeString('id-ID')}
                  </div>
                )}
              </div>
            </div>
          </div>

          {/* ESP32 4 */}
          <div className="bg-gradient-to-r from-pink-50 to-pink-100 rounded-lg p-4">
            <div className="flex items-center justify-between mb-2">
              <div>
                <div className="font-semibold text-gray-800">ESP32 4</div>
                <div className="text-xs text-gray-600">WiFi Controller & LCD</div>
              </div>
              <button
                onClick={() => sendPing("esp32_4")}
                className="px-3 py-1 bg-pink-500 text-white rounded text-xs hover:bg-pink-600"
              >
                Ping
              </button>
            </div>
            <div className="flex items-center justify-between">
              <div className={`text-3xl font-bold ${getPingColor(pingData.esp32_4)}`}>
                {pingData.esp32_4 !== null ? `${pingData.esp32_4}ms` : "---"}
              </div>
              <div className="text-right">
                <div className={`text-sm font-semibold ${getPingColor(pingData.esp32_4)}`}>
                  {getPingStatus(pingData.esp32_4)}
                </div>
                {pingData.lastPing4 && (
                  <div className="text-xs text-gray-500">
                    {pingData.lastPing4.toLocaleTimeString('id-ID')}
                  </div>
                )}
              </div>
            </div>
          </div>
        </div>

        <div className="mt-4 bg-gray-50 p-3 rounded-lg">
          <div className="text-xs text-gray-600">
            <div className="font-semibold mb-1">Latency Guide:</div>
            <div className="flex items-center gap-4">
              <span className="text-green-600">● &lt;100ms: Excellent</span>
              <span className="text-yellow-600">● 100-300ms: Good</span>
              <span className="text-red-600">● &gt;300ms: Poor</span>
            </div>
          </div>
        </div>
      </div>

      {/* Ping History Charts */}
      <div className="bg-white rounded-xl shadow-lg p-6">
        <div className="flex items-center gap-3 mb-4">
          <TrendingUp className="w-6 h-6 text-indigo-600" />
          <h3 className="text-lg font-bold text-gray-800">Ping History (Last {MAX_HISTORY} pings)</h3>
        </div>

        <div className="space-y-6">
          {/* ESP32 1 Chart */}
          <div>
            <div className="flex items-center justify-between mb-2">
              <div className="font-semibold text-gray-800">ESP32 1 - Lamp, Gas, Feeder</div>
              <div className="text-sm text-gray-600">{pingHistory.esp32_1.length} pings</div>
            </div>
            {renderChart(pingHistory.esp32_1, "#3b82f6", "ESP32 1")}
          </div>

          {/* ESP32 2 Chart */}
          <div>
            <div className="flex items-center justify-between mb-2">
              <div className="font-semibold text-gray-800">ESP32 2 - Trash, Clothesline</div>
              <div className="text-sm text-gray-600">{pingHistory.esp32_2.length} pings</div>
            </div>
            {renderChart(pingHistory.esp32_2, "#10b981", "ESP32 2")}
          </div>

          {/* ESP32 3 Chart */}
          <div>
            <div className="flex items-center justify-between mb-2">
              <div className="font-semibold text-gray-800">ESP32 3 - Smart Door Lock</div>
              <div className="text-sm text-gray-600">{pingHistory.esp32_3.length} pings</div>
            </div>
            {renderChart(pingHistory.esp32_3, "#8b5cf6", "ESP32 3")}
          </div>

          {/* ESP32 4 Chart */}
          <div>
            <div className="flex items-center justify-between mb-2">
              <div className="font-semibold text-gray-800">ESP32 4 - WiFi Controller & LCD</div>
              <div className="text-sm text-gray-600">{pingHistory.esp32_4.length} pings</div>
            </div>
            {renderChart(pingHistory.esp32_4, "#ec4899", "ESP32 4")}
          </div>
        </div>
      </div>
    </div>
  );
}
