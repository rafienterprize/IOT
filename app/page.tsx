"use client";

import { useMQTT } from "@/hooks/useMQTT";
import DeviceStatus from "@/components/DeviceStatus";
import LCDDisplay from "@/components/LCDDisplay";  // <-- fixed
import SmartDoor from "@/components/SmartDoor";
import SmartLamp from "@/components/SmartLamp";
import GasDetector from "@/components/GasDetector";
import FishFeeder from "@/components/FishFeeder";
import SmartTrash from "@/components/SmartTrash";
import SmartClothesline from "@/components/SmartClothesline";

export default function Home() {
  const { publish, subscribe } = useMQTT();

  return (
    <div className="min-h-screen grid-pattern">
      <div className="max-w-7xl mx-auto p-6 md:p-8">
        <header className="mb-8">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">
            Dashboard Overview
          </h1>
          <p className="text-gray-600">
            Smart Home Monitoring and Control by XI SIJA 1
          </p>
        </header>

        <div className="mb-6">
          <DeviceStatus subscribe={subscribe} publish={publish} />
        </div>
        <div className="mb-6">
          <LCDDisplay subscribe={subscribe} />
        </div>
        <div className="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-3 gap-6">
          <SmartDoor publish={publish} subscribe={subscribe} />
          <SmartLamp publish={publish} subscribe={subscribe} />
          <GasDetector subscribe={subscribe} />
          <FishFeeder publish={publish} subscribe={subscribe} />
          <SmartTrash subscribe={subscribe} publish={publish} />
          <SmartClothesline publish={publish} subscribe={subscribe} />
        </div>
      </div>
    </div>
  );
}
