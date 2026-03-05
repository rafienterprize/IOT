"use client";

import { useState, useEffect } from "react";
import { Monitor } from "lucide-react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => void;
}

export default function LCDDisplay({ subscribe }: Props) {
  const [lcdMessage, setLcdMessage] = useState("System Siap Digunakan");

  useEffect(() => {
    const unsubscribeLcd = subscribe("iot/door/lcd", (message) => {
      setLcdMessage(message);
    });

    return () => {
      if (unsubscribeLcd) unsubscribeLcd();
    };
  }, [subscribe]);

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <div className="flex items-center gap-3 mb-4">
        <Monitor className="w-8 h-8 text-green-600" />
        <h2 className="text-xl font-bold text-gray-800">LCD Display Monitor</h2>
      </div>

      {/* LCD Display Simulation */}
      <div className="bg-gradient-to-br from-green-900 to-green-800 text-green-300 p-6 rounded-xl font-mono border-4 border-green-700 shadow-inner">
        <div className="text-xs text-green-400 mb-2 text-center opacity-70">
          LCD Display:
        </div>
        <div className="text-xl font-bold text-center leading-relaxed tracking-wide">
          {lcdMessage}
        </div>
        <div className="mt-3 flex justify-center gap-1">
          <div className="w-2 h-2 bg-green-500 rounded-full animate-pulse"></div>
          <div className="w-2 h-2 bg-green-500 rounded-full animate-pulse" style={{ animationDelay: '0.2s' }}></div>
          <div className="w-2 h-2 bg-green-500 rounded-full animate-pulse" style={{ animationDelay: '0.4s' }}></div>
        </div>
      </div>

      <div className="mt-4 text-xs text-gray-600 text-center bg-blue-50 p-2 rounded">
        💡 Menampilkan pesan real-time dari LCD fisik ESP32
      </div>
    </div>
  );
}
