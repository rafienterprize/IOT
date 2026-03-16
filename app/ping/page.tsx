"use client";

import { useMQTT } from "@/hooks/useMQTT";
import NetworkPing from "@/components/NetworkPing";

export default function PingPage() {
  const { publish, subscribe } = useMQTT();

  return (
    <div className="min-h-screen grid-pattern">
      <div className="max-w-7xl mx-auto p-6 md:p-8">
        <header className="mb-8">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">
            Network Latency Monitor
          </h1>
          <p className="text-gray-600">
            Monitor latensi koneksi jaringan
          </p>
        </header>

        <div>
          <NetworkPing subscribe={subscribe} publish={publish} />
        </div>
      </div>
    </div>
  );
}
