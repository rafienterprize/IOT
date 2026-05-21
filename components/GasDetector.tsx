"use client";

import { useEffect, useState } from "react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => (() => void) | void;
}

export default function GasDetector({ subscribe }: Props) {
  const [level, setLevel] = useState<number | null>(null);
  const [status, setStatus] = useState("Menunggu data...");

  useEffect(() => {
    const unsubscribe = subscribe("iot/gas/level", (message) => {
      const value = parseFloat(message);
      setLevel(Number.isNaN(value) ? null : value);
      if (!Number.isNaN(value)) {
        if (value > 500) setStatus("Bahaya - segera cek!");
        else if (value > 200) setStatus("Caution");
        else setStatus("Aman");
      }
    });

    return () => {
      if (unsubscribe) unsubscribe();
    };
  }, [subscribe]);

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <h3 className="text-lg font-bold text-gray-800 mb-3">Gas Detector</h3>
      <div className="space-y-2">
        <div className="text-sm text-gray-600">Level gas:</div>
        <div className="text-3xl font-semibold text-gray-800">
          {level === null ? "-" : `${level.toFixed(1)} ppm`}
        </div>
        <div className="text-sm font-medium text-gray-700">{status}</div>
      </div>
    </div>
  );
}
