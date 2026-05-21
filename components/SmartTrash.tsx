"use client";

import { useEffect, useState } from "react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => (() => void) | void;
  publish: (topic: string, message: string) => void;
}

export default function SmartTrash({ subscribe, publish }: Props) {
  const [level, setLevel] = useState<number | null>(null);
  const [status, setStatus] = useState("Menunggu...");

  useEffect(() => {
    const unsubscribe = subscribe("iot/trash/level", (message) => {
      const value = parseFloat(message);
      if (!Number.isNaN(value)) {
        setLevel(value);
        if (value > 80) {
          setStatus("Penuh - kosongkan segera");
        } else if (value > 50) {
          setStatus("Mendekati penuh");
        } else {
          setStatus("Cukup kosong");
        }
      }
    });

    return () => {
      if (unsubscribe) unsubscribe();
    };
  }, [subscribe]);

  const reset = () => {
    publish("iot/trash/command", "RESET");
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <h3 className="text-lg font-bold text-gray-800 mb-3">Smart Trash</h3>
      <div className="space-y-2">
        <div className="text-sm text-gray-600">Level sampah:</div>
        <div className="text-3xl font-semibold text-gray-800">
          {level === null ? "-" : `${level.toFixed(0)}%`}
        </div>
        <div className="text-sm font-medium text-gray-700">{status}</div>
      </div>
      <button
        onClick={reset}
        className="mt-4 px-4 py-2 rounded bg-yellow-600 text-white hover:bg-yellow-700"
      >
        Reset sensor
      </button>
    </div>
  );
}
