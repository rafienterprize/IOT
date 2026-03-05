"use client";

import { useState, useEffect } from "react";
import { Lightbulb, Clock } from "lucide-react";

interface Props {
  publish: (topic: string, message: string) => void;
  subscribe: (topic: string, callback: (message: string) => void) => void;
}

export default function SmartLamp({ publish, subscribe }: Props) {
  const [isOn, setIsOn] = useState(false);
  const [timerOn, setTimerOn] = useState("");
  const [timerOff, setTimerOff] = useState("");
  const [autoMode, setAutoMode] = useState(false);

  useEffect(() => {
    const unsubscribe = subscribe("iot/lamp/status", (message) => {
      console.log("Lamp status received:", message);
      setIsOn(message === "ON");
    });
    return unsubscribe;
  }, [subscribe]);

  const toggleLamp = () => {
    const newState = !isOn;
    publish("iot/lamp/control", newState ? "ON" : "OFF");
    console.log("Lamp control sent:", newState ? "ON" : "OFF");
  };

  const setTimer = () => {
    if (timerOn && timerOff) {
      publish("iot/lamp/timer", JSON.stringify({ on: timerOn, off: timerOff }));
      setAutoMode(true);
      alert(`Timer diset: Nyala ${timerOn}, Mati ${timerOff}`);
    }
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <div className="flex items-center gap-3 mb-4">
        <Lightbulb className={`w-8 h-8 ${isOn ? 'text-yellow-400' : 'text-gray-400'}`} />
        <h2 className="text-xl font-bold text-gray-800">Smart Lamp</h2>
      </div>

      <div className="space-y-4">
        <button
          onClick={toggleLamp}
          className={`w-full py-3 rounded-lg font-semibold transition-colors ${
            isOn
              ? 'bg-yellow-400 text-gray-800 hover:bg-yellow-500'
              : 'bg-gray-300 text-gray-600 hover:bg-gray-400'
          }`}
        >
          {isOn ? 'NYALA 💡' : 'MATI'}
        </button>

        <div className="border-t pt-4">
          <div className="flex items-center gap-2 mb-3">
            <Clock className="w-5 h-5 text-gray-600" />
            <span className="font-semibold text-gray-700">Timer Otomatis</span>
          </div>
          
          <div className="space-y-2">
            <div>
              <label className="text-sm text-gray-600">Waktu Nyala</label>
              <input
                type="time"
                value={timerOn}
                onChange={(e) => setTimerOn(e.target.value)}
                className="w-full px-3 py-2 border rounded-lg"
              />
            </div>
            <div>
              <label className="text-sm text-gray-600">Waktu Mati</label>
              <input
                type="time"
                value={timerOff}
                onChange={(e) => setTimerOff(e.target.value)}
                className="w-full px-3 py-2 border rounded-lg"
              />
            </div>
            <button
              onClick={setTimer}
              className="w-full py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600"
            >
              Set Timer
            </button>
          </div>
          
          {autoMode && (
            <div className="mt-2 text-sm text-green-600">
              ✓ Mode otomatis aktif
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
