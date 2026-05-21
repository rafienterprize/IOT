"use client";

import { useEffect, useState } from "react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => (() => void) | void;
  publish: (topic: string, message: string) => void;
}

export default function SmartClothesline({ subscribe, publish }: Props) {
  const [status, setStatus] = useState("Menunggu...");
  const [autoMode, setAutoMode] = useState(false);

  useEffect(() => {
    const unsubscribe = subscribe("iot/clothesline/status", (message) => {
      setStatus(message);
      setAutoMode(message.toLowerCase().includes("auto"));
    });

    return () => {
      if (unsubscribe) unsubscribe();
    };
  }, [subscribe]);

  const toggleAuto = () => {
    const next = autoMode ? "MANUAL" : "AUTO";
    publish("iot/clothesline/command", next);
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <h3 className="text-lg font-bold text-gray-800 mb-3">Smart Clothesline</h3>
      <div className="space-y-2">
        <div className="text-sm text-gray-600">Mode:</div>
        <div className="text-xl font-semibold text-gray-800">{status}</div>
      </div>
      <button
        onClick={toggleAuto}
        className={`mt-4 px-4 py-2 rounded font-semibold ${autoMode ? "bg-blue-600 text-white" : "bg-gray-800 text-white"}`}
      >
        {autoMode ? "Matikan Auto" : "Aktifkan Auto"}
      </button>
    </div>
  );
}
