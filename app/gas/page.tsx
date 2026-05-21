"use client";

import { useMQTT } from "@/hooks/useMQTT";
import GasDetector from "@/components/GasDetector";

export default function GasPage() {
  const { subscribe } = useMQTT();

  return (
    <div className="min-h-screen grid-pattern">
      <div className="max-w-7xl mx-auto p-6 md:p-8">
        <header className="mb-8">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">
            Gas Detector Monitoring
          </h1>
          <p className="text-gray-600">
            Monitor tingkat gas secara real-time
          </p>
        </header>

        <div className="max-w-4xl">
          <GasDetector subscribe={subscribe} />
        </div>
      </div>
    </div>
  );
}
