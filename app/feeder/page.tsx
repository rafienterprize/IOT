"use client";

import { useMQTT } from "@/hooks/useMQTT";
import FishFeeder from "@/components/FishFeeder";

export default function FeederPage() {
  const { publish, subscribe } = useMQTT();

  return (
    <div className="min-h-screen grid-pattern">
      <div className="max-w-7xl mx-auto p-6 md:p-8">
        <header className="mb-8">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">
            Fish Feeder Control
          </h1>
          <p className="text-gray-600">
            Atur jadwal pemberian makan ikan otomatis
          </p>
        </header>

        <div className="max-w-2xl">
          <FishFeeder publish={publish} subscribe={subscribe} />
        </div>
      </div>
    </div>
  );
}
