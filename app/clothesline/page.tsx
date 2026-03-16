"use client";

import { useMQTT } from "@/hooks/useMQTT";
import SmartClothesline from "@/components/SmartClothesline";

export default function ClotheslinePage() {
  const { publish, subscribe } = useMQTT();

  return (
    <div className="min-h-screen grid-pattern">
      <div className="max-w-7xl mx-auto p-6 md:p-8">
        <header className="mb-8">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">
            Smart Clothesline Control
          </h1>
          <p className="text-gray-600">
            Kontrol jemuran otomatis sesuai cuaca
          </p>
        </header>

        <div className="max-w-2xl">
          <SmartClothesline publish={publish} subscribe={subscribe} />
        </div>
      </div>
    </div>
  );
}
