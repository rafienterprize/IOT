"use client";

import { useState, useEffect } from "react";
import Sidebar from "@/components/Sidebar";
import SmartLamp from "@/components/SmartLamp";
import GasDetector from "@/components/GasDetector";
import FishFeeder from "@/components/FishFeeder";
import SmartTrash from "@/components/SmartTrash";
import SmartClothesline from "@/components/SmartClothesline";
import DeviceStatus from "@/components/DeviceStatus";
import SmartDoor from "@/components/SmartDoor";
import LCDDisplay from "@/components/LCDDisplay";
import IntroAnimation from "@/components/IntroAnimation";
import NetworkPing from "@/components/NetworkPing";
import { useMQTT } from "@/hooks/useMQTT";

export default function Home() {
  const { isConnected, publish, subscribe } = useMQTT();
  const [activeSection, setActiveSection] = useState("dashboard");
  const [showIntro, setShowIntro] = useState(true);
  const [mounted, setMounted] = useState(false);

  useEffect(() => {
    setMounted(true);
  }, []);

  if (!mounted) {
    return null;
  }

  if (showIntro) {
    return <IntroAnimation onComplete={() => setShowIntro(false)} />;
  }

  const renderContent = () => {
    switch (activeSection) {
      case "dashboard":
        return (
          <>
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
          </>
        );
      case "lamp":
        return (
          <div className="max-w-2xl mx-auto">
            <SmartLamp publish={publish} subscribe={subscribe} />
          </div>
        );
      case "gas":
        return (
          <div className="max-w-4xl mx-auto">
            <GasDetector subscribe={subscribe} />
          </div>
        );
      case "feeder":
        return (
          <div className="max-w-2xl mx-auto">
            <FishFeeder publish={publish} subscribe={subscribe} />
          </div>
        );
      case "trash":
        return (
          <div className="max-w-2xl mx-auto">
            <SmartTrash subscribe={subscribe} publish={publish} />
          </div>
        );
      case "clothesline":
        return (
          <div className="max-w-2xl mx-auto">
            <SmartClothesline publish={publish} subscribe={subscribe} />
          </div>
        );
      case "devices":
        return (
          <div className="max-w-2xl mx-auto">
            <DeviceStatus subscribe={subscribe} publish={publish} />
          </div>
        );
      case "ping":
        return (
          <div>
            <NetworkPing subscribe={subscribe} publish={publish} />
          </div>
        );
      case "door":
        return (
          <div className="max-w-2xl mx-auto">
            <SmartDoor publish={publish} subscribe={subscribe} />
          </div>
        );
      default:
        return null;
    }
  };

  const getSectionTitle = () => {
    const titles: Record<string, string> = {
      dashboard: "Dashboard Overview",
      devices: "Device Status & Pairing",
      ping: "Network Latency Monitor",
      door: "Smart Door Lock Control",
      lamp: "Smart Lamp Control",
      gas: "Gas Detector Monitoring",
      feeder: "Fish Feeder Control",
      trash: "Smart Trash Bin Status",
      clothesline: "Smart Clothesline Control",
    };
    return titles[activeSection] || "Dashboard";
  };

  return (
    <div className="min-h-screen grid-pattern">
      <Sidebar
        activeSection={activeSection}
        onSectionChange={setActiveSection}
        isConnected={isConnected}
      />

      <main className="lg:ml-64 min-h-screen p-6 md:p-8">
        <header className="mb-8">
          <h1 className="text-3xl font-bold text-gray-800 mb-2">
            {getSectionTitle()}
          </h1>
          <p className="text-gray-600">
            {activeSection === "dashboard"
              ? "Smart Home Monitoring and Control by XI SIJA 1"
              : "Kelola device dengan mudah"}
          </p>
        </header>

        {renderContent()}
      </main>
    </div>
  );
}
