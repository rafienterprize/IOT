"use client";

import { useMQTT } from "@/hooks/useMQTT";
import SmartLamp from "@/components/SmartLamp";

export default function LampPage() {
  const { publish, subscribe } = useMQTT();

  return (
    <div className="min-h-screen grid-pattern">
      <div className="max-w-7xl mx-auto p-6 md:p-8">
        <header className="mb-8">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">
            Smart Lamp Control
          </h1>
          <p className="text-gray-600">
            Kelola lampu pintar dengan mudah
          </p>
        </header>

        <div className="max-w-2xl">
          <SmartLamp publish={publish} subscribe={subscribe} />
        </div>
      </div>
    </div>
  );
}
