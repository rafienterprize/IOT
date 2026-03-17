"use client";

import { useEffect, useState } from "react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => (() => void) | void;
  publish: (topic: string, message: string) => void;
}

export default function FishFeeder({ subscribe, publish }: Props) {
  const [lastFed, setLastFed] = useState<string>("Belum pernah");
  const [status, setStatus] = useState<string>("Menunggu...");

  useEffect(() => {
    const unsubscribe = subscribe("iot/feeder/status", (message) => {
      setStatus(message);
      if (message.toLowerCase().includes("fed")) {
        setLastFed(new Date().toLocaleTimeString());
      }
    });

    return () => {
      if (unsubscribe) unsubscribe();
    };
  }, [subscribe]);

  const feedNow = () => {
    publish("iot/feeder/command", "FEED");
    setLastFed(new Date().toLocaleTimeString());
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <h3 className="text-lg font-bold text-gray-800 mb-3">Fish Feeder</h3>
      <div className="space-y-2">
        <div className="text-sm text-gray-600">Status:</div>
        <div className="text-xl font-semibold text-gray-800">{status}</div>
        <div className="text-sm text-gray-500">Terakhir diberi makan: {lastFed}</div>
      </div>
      <button
        onClick={feedNow}
        className="mt-4 px-4 py-2 rounded bg-emerald-600 text-white hover:bg-emerald-700"
      >
        Beri makan sekarang
      </button>
    </div>
  );
}
