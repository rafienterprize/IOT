"use client";

import { useState, useEffect } from "react";
import { Wind, AlertTriangle } from "lucide-react";
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from "recharts";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => void;
}

export default function GasDetector({ subscribe }: Props) {
  const [gasLevel, setGasLevel] = useState(0);
  const [history, setHistory] = useState<{ time: string; value: number }[]>([]);
  const [isDangerous, setIsDangerous] = useState(false);

  useEffect(() => {
    const unsubscribe = subscribe("iot/gas/level", (message) => {
      const level = parseFloat(message);
      setGasLevel(level);
      setIsDangerous(level > 500);

      const now = new Date();
      const timeStr = `${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}`;
      
      setHistory(prev => {
        const newHistory = [...prev, { time: timeStr, value: level }];
        return newHistory.slice(-20); // Keep last 20 data points
      });
    });
    return unsubscribe;
  }, [subscribe]);

  return (
    <div className="bg-white rounded-xl shadow-lg p-6 relative">
      {/* Red Alert Light - Top Right Corner */}
      {isDangerous && (
        <div className="absolute top-4 right-4 flex items-center justify-center">
          <div className="relative w-16 h-16">
            {/* Outer glow */}
            <div className="absolute inset-0 bg-red-500 rounded-full opacity-30 animate-ping"></div>
            {/* Middle glow */}
            <div className="absolute inset-2 bg-red-500 rounded-full opacity-50 animate-pulse"></div>
            {/* Inner solid */}
            <div className="absolute inset-4 bg-red-600 rounded-full animate-pulse"></div>
          </div>
        </div>
      )}

      <div className="flex items-center gap-3 mb-4">
        <Wind className="w-8 h-8 text-blue-500" />
        <h2 className="text-xl font-bold text-gray-800">Gas Detector</h2>
      </div>

      <div className="space-y-4">
        {/* Alert Blinking */}
        {isDangerous && (
          <div className="bg-red-100 border-2 border-red-500 rounded-lg p-3 animate-pulse">
            <div className="flex items-center gap-2">
              <div className="w-3 h-3 bg-red-600 rounded-full animate-ping"></div>
              <AlertTriangle className="w-5 h-5 text-red-600" />
              <span className="text-sm font-bold text-red-700">
                ⚠️ BAHAYA! Gas Terdeteksi Tinggi!
              </span>
            </div>
          </div>
        )}

        {/* Gas Level Display */}
        <div className={`p-4 rounded-lg ${isDangerous ? 'bg-red-100 border-2 border-red-500' : 'bg-green-100'}`}>
          <div className="flex items-center justify-between">
            <span className="text-sm font-semibold text-gray-700">Level Gas</span>
            {isDangerous && (
              <div className="flex items-center gap-1">
                <div className="w-2 h-2 bg-red-600 rounded-full animate-ping"></div>
                <div className="w-2 h-2 bg-red-600 rounded-full animate-ping" style={{ animationDelay: '0.2s' }}></div>
                <div className="w-2 h-2 bg-red-600 rounded-full animate-ping" style={{ animationDelay: '0.4s' }}></div>
              </div>
            )}
          </div>
          <div className="text-4xl font-bold mt-2">
            <span className={isDangerous ? 'text-red-600' : 'text-green-600'}>
              {gasLevel.toFixed(1)} PPM
            </span>
          </div>
          <div className="text-sm mt-1 font-semibold">
            {isDangerous ? '🚨 BAHAYA! Evakuasi Area' : '✓ Aman'}
          </div>
        </div>

        {/* Chart */}
        <div className="bg-gray-50 p-4 rounded-lg">
          <div className="text-sm font-semibold text-gray-700 mb-2">Grafik Real-Time</div>
          {history.length > 0 ? (
            <div className="h-48">
              <ResponsiveContainer width="100%" height="100%">
                <LineChart data={history}>
                  <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                  <XAxis 
                    dataKey="time" 
                    tick={{ fontSize: 10, fill: '#6b7280' }}
                    stroke="#9ca3af"
                  />
                  <YAxis 
                    tick={{ fontSize: 10, fill: '#6b7280' }}
                    stroke="#9ca3af"
                    domain={[0, 'auto']}
                  />
                  <Tooltip 
                    contentStyle={{ 
                      backgroundColor: '#fff', 
                      border: '1px solid #e5e7eb',
                      borderRadius: '8px',
                      fontSize: '12px'
                    }}
                  />
                  <Line 
                    type="monotone" 
                    dataKey="value" 
                    stroke={isDangerous ? "#dc2626" : "#3b82f6"}
                    strokeWidth={2}
                    dot={{ fill: isDangerous ? "#dc2626" : "#3b82f6", r: 3 }}
                    activeDot={{ r: 5 }}
                  />
                </LineChart>
              </ResponsiveContainer>
            </div>
          ) : (
            <div className="h-48 flex items-center justify-center text-gray-400 text-sm">
              Menunggu data sensor...
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
