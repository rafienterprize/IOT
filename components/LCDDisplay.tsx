"use client";

import { useEffect, useState } from "react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => (() => void) | void;
  publish?: (topic: string, message: string) => void;
}

export default function LCDDisplay({ subscribe, publish }: Props) {
  const [text, setText] = useState("Menunggu data...");

  useEffect(() => {
    const unsubscribe = subscribe("iot/lcd/display", (message) => {
      setText(message);
    });

    return () => {
      if (unsubscribe) unsubscribe();
    };
  }, [subscribe]);

  const sendTest = () => {
    if (!publish) return;
    publish("iot/lcd/display", `Hello from dashboard @ ${new Date().toLocaleTimeString()}`);
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <h3 className="text-lg font-bold text-gray-800 mb-3">LCD Display</h3>
      <div className="min-h-[96px] p-4 bg-black text-green-300 font-mono rounded-lg">
        {text}
      </div>
      {publish ? (
        <button
          onClick={sendTest}
          className="mt-4 px-4 py-2 rounded bg-indigo-600 text-white hover:bg-indigo-700"
        >
          Kirim teks percobaan
        </button>
      ) : null}
    </div>
  );
}
