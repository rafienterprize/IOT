"use client";

import { useState, useEffect } from "react";
import { CloudRain, Sun } from "lucide-react";

interface Props {
  publish: (topic: string, message: string) => void;
  subscribe: (topic: string, callback: (message: string) => void) => void;
}

export default function SmartClothesline({ publish, subscribe }: Props) {
  const [isOpen, setIsOpen] = useState(false);
  const [isRaining, setIsRaining] = useState(false);
  const [autoMode, setAutoMode] = useState(true);

  useEffect(() => {
    const unsubscribeRain = subscribe("iot/clothesline/rain", (message) => {
      const raining = message === "RAIN";
      setIsRaining(raining);
      
      if (raining && autoMode && isOpen) {
        setIsOpen(false);
        publish("iot/clothesline/control", "CLOSE");
      }
    });

    const unsubscribeStatus = subscribe("iot/clothesline/status", (message) => {
      setIsOpen(message === "OPEN");
    });

    return () => {
      if (unsubscribeRain) unsubscribeRain();
      if (unsubscribeStatus) unsubscribeStatus();
    };
  }, [subscribe, autoMode, isOpen, publish]);

  const toggle = () => {
    const newState = !isOpen;
    setIsOpen(newState);
    publish("iot/clothesline/control", newState ? "OPEN" : "CLOSE");
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <div className="flex items-center gap-3 mb-4">
        {isRaining ? (
          <CloudRain className="w-8 h-8 text-blue-500" />
        ) : (
          <Sun className="w-8 h-8 text-yellow-500" />
        )}
        <h2 className="text-xl font-bold text-gray-800">Jemuran Otomatis</h2>
      </div>

      <div className="space-y-4">
        {isRaining && (
          <div className="bg-blue-100 p-3 rounded-lg">
            <div className="flex items-center gap-2">
              <CloudRain className="w-5 h-5 text-blue-600" />
              <span className="text-sm font-semibold text-blue-700">
                ⚠️ Hujan terdeteksi!
              </span>
            </div>
          </div>
        )}

        <div className="text-center">
          <div className="text-sm text-gray-600 mb-2">Status Jemuran</div>
          <div className={`text-2xl font-bold ${isOpen ? 'text-green-600' : 'text-gray-600'}`}>
            {isOpen ? '👕 TERBUKA' : '🏠 TERTUTUP'}
          </div>
        </div>

        <button
          onClick={toggle}
          className={`w-full py-3 rounded-lg font-semibold transition-colors ${
            isOpen
              ? 'bg-red-500 text-white hover:bg-red-600'
              : 'bg-green-500 text-white hover:bg-green-600'
          }`}
        >
          {isOpen ? 'Tutup Jemuran' : 'Buka Jemuran'}
        </button>

        <div className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
          <span className="text-sm text-gray-700">Mode Otomatis</span>
          <button
            onClick={() => {
              setAutoMode(!autoMode);
              publish("iot/clothesline/auto", !autoMode ? "ON" : "OFF");
            }}
            className={`px-4 py-1 rounded-full text-sm font-semibold ${
              autoMode
                ? 'bg-green-500 text-white'
                : 'bg-gray-300 text-gray-600'
            }`}
          >
            {autoMode ? 'ON' : 'OFF'}
          </button>
        </div>

        <div className="text-xs text-gray-500 text-center">
          {autoMode ? '✓ Jemuran akan otomatis tertutup saat hujan' : 'Mode manual aktif'}
        </div>
      </div>
    </div>
  );
}
