"use client";

import { useMQTT } from "@/hooks/useMQTT";
import SmartDoor from "@/components/SmartDoor";

export default function DoorPage() {
  const { publish, subscribe } = useMQTT();

  return (
    <div className="min-h-screen grid-pattern">
      <div className="max-w-7xl mx-auto p-6 md:p-8">
        <header className="mb-8">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">
            Smart Door Lock Control
          </h1>
          <p className="text-gray-600">
            Kontrol pintu otomatis dengan keamanan
          </p>
        </header>

        <div className="max-w-2xl">
          <SmartDoor publish={publish} subscribe={subscribe} />
        </div>
      </div>
    </div>
  );
}
