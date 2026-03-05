"use client";

import { useState } from "react";
import { Lightbulb, Wind, Fish, Trash2, CloudRain, Menu, X, Home, Wifi, DoorOpen, Activity } from "lucide-react";

interface Props {
  activeSection: string;
  onSectionChange: (section: string) => void;
  isConnected: boolean;
}

export default function Sidebar({ activeSection, onSectionChange, isConnected }: Props) {
  const [isOpen, setIsOpen] = useState(true);

  const menuItems = [
    { id: "dashboard", label: "Dashboard", icon: Home },
    { id: "devices", label: "Device Status", icon: Wifi },
    { id: "ping", label: "Network Ping", icon: Activity },
    { id: "door", label: "Smart Door", icon: DoorOpen },
    { id: "lamp", label: "Smart Lamp", icon: Lightbulb },
    { id: "gas", label: "Gas Detector", icon: Wind },
    { id: "feeder", label: "Fish Feeder", icon: Fish },
    { id: "trash", label: "Smart Trash", icon: Trash2 },
    { id: "clothesline", label: "Jemuran", icon: CloudRain },
  ];

  return (
    <>
      {/* Mobile Toggle */}
      <button
        onClick={() => setIsOpen(!isOpen)}
        className="lg:hidden fixed top-4 left-4 z-50 p-2 bg-white rounded-lg shadow-lg"
      >
        {isOpen ? <X className="w-6 h-6" /> : <Menu className="w-6 h-6" />}
      </button>

      {/* Sidebar */}
      <aside
        className={`fixed top-0 left-0 h-full bg-gradient-to-b from-indigo-900 to-indigo-700 text-white transition-transform duration-300 z-40 ${
          isOpen ? "translate-x-0" : "-translate-x-full"
        } w-64 lg:translate-x-0`}
      >
        <div className="p-6">
          <div className="flex items-center gap-3 mb-8">
            <div className="w-10 h-10 bg-white rounded-lg flex items-center justify-center">
              <span className="text-2xl">🏠</span>
            </div>
            <div>
              <h1 className="text-xl font-bold">IoT Monitor</h1>
              <div className="flex items-center gap-2 mt-1">
                <div className={`w-2 h-2 rounded-full ${isConnected ? 'bg-green-400' : 'bg-red-400'}`}></div>
                <span className="text-xs opacity-80">
                  {isConnected ? 'Connected' : 'Disconnected'}
                </span>
              </div>
            </div>
          </div>

          <nav className="space-y-2">
            {menuItems.map((item) => {
              const Icon = item.icon;
              const isActive = activeSection === item.id;
              
              return (
                <button
                  key={item.id}
                  onClick={() => onSectionChange(item.id)}
                  className={`w-full flex items-center gap-3 px-4 py-3 rounded-lg transition-all ${
                    isActive
                      ? 'bg-white text-indigo-900 shadow-lg'
                      : 'hover:bg-indigo-800 text-white'
                  }`}
                >
                  <Icon className="w-5 h-5" />
                  <span className="font-medium">{item.label}</span>
                </button>
              );
            })}
          </nav>
        </div>

        <div className="absolute bottom-0 left-0 right-0 p-6 border-t border-indigo-600">
          <div className="text-xs opacity-70">
            <p>IoT Dashboard v1.0</p>
            <p className="mt-1">ESP32 Monitoring System</p>
          </div>
        </div>
      </aside>

      {/* Overlay for mobile */}
      {isOpen && (
        <div
          onClick={() => setIsOpen(false)}
          className="lg:hidden fixed inset-0 bg-black bg-opacity-50 z-30"
        />
      )}
    </>
  );
}
