"use client";

import { useEffect, useState } from "react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => (() => void) | void;
  publish: (topic: string, message: string) => void;
}

export default function SmartDoor({ subscribe, publish }: Props) {
  const [locked, setLocked] = useState<boolean>(false);
  const [status, setStatus] = useState<string>("Menunggu...");

  useEffect(() => {
    const unsubscribe = subscribe("iot/door/state", (message) => {
      setStatus(message);
      setLocked(message.toLowerCase().includes("locked") || message.toLowerCase().includes("tutup"));
    });

    return () => {
      if (unsubscribe) unsubscribe();
    };
  }, [subscribe]);

  const toggleLock = () => {
    const command = locked ? "UNLOCK" : "LOCK";
    publish("iot/door/command", command);
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <h3 className="text-lg font-bold text-gray-800 mb-3">Smart Door</h3>
      <div className="flex items-center justify-between">
        <div>
          <div className="text-sm text-gray-600">Status:</div>
          <div className="text-xl font-semibold text-gray-800">{status}</div>
        </div>
        <button
          onClick={toggleLock}
          className={`px-4 py-2 rounded font-semibold ${locked ? "bg-red-600 text-white" : "bg-green-600 text-white"}`}
        >
          {locked ? "Buka" : "Kunci"}
        </button>
      </div>
    </div>
  );
}
