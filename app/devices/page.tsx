"use client";

import { useMQTT } from "@/hooks/useMQTT";
import DeviceStatus from "@/components/DeviceStatus";

export default function DevicesPage() {
  const { publish, subscribe } = useMQTT();

  return (
    <div className="min-h-screen grid-pattern">
      <div className="max-w-7xl mx-auto p-6 md:p-8">
        <header className="mb-8">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">
            Device Status & Pairing
          </h1>
          <p className="text-gray-600">
            Kelola device dengan mudah
          </p>
        </header>

        <div className="max-w-2xl">
          <DeviceStatus subscribe={subscribe} publish={publish} />
        </div>
      </div>
    </div>
  );
}
