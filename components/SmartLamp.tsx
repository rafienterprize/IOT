"use client";

import { useEffect, useState } from "react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => (() => void) | void;
  publish: (topic: string, message: string) => void;
}

export default function SmartLamp({ subscribe, publish }: Props) {
  const [on, setOn] = useState<boolean>(false);
  const [brightness, setBrightness] = useState<number>(0);

  useEffect(() => {
    const unsubscribe = subscribe("iot/lamp/state", (message) => {
      const parts = message.split("|");
      setOn(parts[0]?.toLowerCase() === "on");
      const parsed = parseInt(parts[1] ?? "0", 10);
      if (!Number.isNaN(parsed)) setBrightness(parsed);
    });

    return () => {
      if (unsubscribe) unsubscribe();
    };
  }, [subscribe]);

  const toggle = () => {
    const next = on ? "OFF" : "ON";
    publish("iot/lamp/command", next);
  };

  const setBright = (value: number) => {
    setBrightness(value);
    publish("iot/lamp/brightness", String(value));
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <h3 className="text-lg font-bold text-gray-800 mb-3">Smart Lamp</h3>
      <div className="flex items-center justify-between mb-4">
        <div>
          <div className="text-sm text-gray-600">Status:</div>
          <div className="text-xl font-semibold text-gray-800">{on ? "Nyala" : "Mati"}</div>
        </div>
        <button
          onClick={toggle}
          className={`px-4 py-2 rounded font-semibold ${on ? "bg-yellow-600 text-black" : "bg-gray-800 text-white"}`}
        >
          {on ? "Matikan" : "Nyalakan"}
        </button>
      </div>
      <div>
        <div className="text-sm text-gray-600 mb-2">Kecerahan: {brightness}%</div>
        <input
          type="range"
          min={0}
          max={100}
          value={brightness}
          onChange={(e) => setBright(Number(e.target.value))}
          className="w-full"
        />
      </div>
    </div>
  );
}
