"use client";

import { useState, useEffect } from "react";
import { DoorOpen, DoorClosed, Lock } from "lucide-react";

interface Props {
  publish: (topic: string, message: string) => void;
  subscribe: (topic: string, callback: (message: string) => void) => void;
}

export default function SmartDoor({ publish, subscribe }: Props) {
  const [isLocked, setIsLocked] = useState(true);

  useEffect(() => {
    // Subscribe to door status
    subscribe("iot/door/status", (message) => {
      setIsLocked(message === "LOCKED");
    });

    return () => {
      // Cleanup handled by parent component
    };
  }, [subscribe]);

  const manualToggle = () => {
    const newState = !isLocked;
    setIsLocked(newState);
    publish("iot/door/control", newState ? "LOCK" : "UNLOCK");
  };

  const emergencyUnlock = () => {
    setIsLocked(false);
    publish("iot/door/emergency", "UNLOCK");
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-3">
          {isLocked ? (
            <DoorClosed className="w-8 h-8 text-red-600" />
          ) : (
            <DoorOpen className="w-8 h-8 text-green-600" />
          )}
          <h2 className="text-xl font-bold text-gray-800">Smart Door Lock</h2>
        </div>
        <div className={`px-3 py-1 rounded-full text-sm font-semibold ${
          isLocked ? 'bg-red-100 text-red-700' : 'bg-green-100 text-green-700'
        }`}>
          {isLocked ? '🔒 LOCKED' : '🔓 UNLOCKED'}
        </div>
      </div>

      {/* Manual Control */}
      <div className="grid grid-cols-2 gap-3 mb-4">
        <button
          onClick={manualToggle}
          className="py-3 bg-green-500 text-white rounded-lg hover:bg-green-600 font-semibold"
        >
          Unlock Door
        </button>
        <button
          onClick={emergencyUnlock}
          className="py-3 bg-orange-500 text-white rounded-lg hover:bg-orange-600 font-semibold"
        >
          🚨 Emergency
        </button>
      </div>

      {/* IR Sensor Info */}
      <div className="bg-blue-50 p-4 rounded-lg">
        <div className="flex items-center gap-2 mb-2">
          <Lock className="w-5 h-5 text-blue-600" />
          <span className="font-semibold text-blue-800">IR Sensor Mode</span>
        </div>
        <div className="text-sm text-blue-700">
          Door automatically opens when person is detected by IR sensor.
          Use manual controls above for remote access.
        </div>
      </div>
    </div>
  );
}
