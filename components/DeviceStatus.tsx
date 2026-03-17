"use client";

import { useEffect, useState } from "react";
import { Wifi, WifiOff, CheckCircle, XCircle, Clock } from "lucide-react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => (() => void) | void;
  publish: (topic: string, message: string) => void;
}

interface DeviceInfo {
  name: string;
  status: "online" | "offline" | "unknown";
  lastSeen: string;
  topic: string;
}

export default function DeviceStatus({ subscribe, publish }: Props) {
  const [devices, setDevices] = useState<DeviceInfo[]>([
    { name: "ESP32 #1 (Lamp/Gas/Feeder)", status: "unknown", lastSeen: "Never", topic: "iot/device1/status" },
    { name: "ESP32 #2 (Trash/Clothesline)", status: "unknown", lastSeen: "Never", topic: "iot/device2/status" },
    { name: "ESP32 #3 (Smart Door)", status: "unknown", lastSeen: "Never", topic: "iot/device3/status" },
    { name: "ESP32 #4 (WiFi/LCD)", status: "unknown", lastSeen: "Never", topic: "iot/device4/status" },
  ]);

  useEffect(() => {
    const unsubscribes: (() => void)[] = [];

    devices.forEach((device, index) => {
      const unsubscribe = subscribe(device.topic, (message) => {
        setDevices(prev => prev.map((d, i) => 
          i === index 
            ? { 
                ...d, 
                status: message.toLowerCase().includes("online") ? "online" : "offline",
                lastSeen: new Date().toLocaleTimeString()
              }
            : d
        ));
      });
      
      if (unsubscribe) {
        unsubscribes.push(unsubscribe);
      }
    });

    // Send ping to all devices every 30 seconds
    const pingInterval = setInterval(() => {
      devices.forEach(device => {
        publish(device.topic.replace('/status', '/ping'), 'PING');
      });
    }, 30000);

    return () => {
      unsubscribes.forEach(unsub => unsub());
      clearInterval(pingInterval);
    };
  }, [subscribe, publish]);

  const getStatusIcon = (status: DeviceInfo['status']) => {
    switch (status) {
      case "online":
        return <CheckCircle className="w-5 h-5 text-green-500" />;
      case "offline":
        return <XCircle className="w-5 h-5 text-red-500" />;
      default:
        return <Clock className="w-5 h-5 text-gray-400" />;
    }
  };

  const getStatusColor = (status: DeviceInfo['status']) => {
    switch (status) {
      case "online":
        return "text-green-600 bg-green-50";
      case "offline":
        return "text-red-600 bg-red-50";
      default:
        return "text-gray-600 bg-gray-50";
    }
  };

  const onlineCount = devices.filter(d => d.status === "online").length;
  const totalCount = devices.length;

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-lg font-bold text-gray-800">Device Status</h3>
        <div className="flex items-center gap-2">
          {onlineCount === totalCount ? (
            <Wifi className="w-5 h-5 text-green-500" />
          ) : (
            <WifiOff className="w-5 h-5 text-red-500" />
          )}
          <span className="text-sm font-medium text-gray-600">
            {onlineCount}/{totalCount} Online
          </span>
        </div>
      </div>

      <div className="space-y-3">
        {devices.map((device, index) => (
          <div key={index} className="flex items-center justify-between p-3 rounded-lg border border-gray-100">
            <div className="flex items-center gap-3">
              {getStatusIcon(device.status)}
              <div>
                <div className="font-medium text-gray-800">{device.name}</div>
                <div className="text-xs text-gray-500">Last seen: {device.lastSeen}</div>
              </div>
            </div>
            <div className={`px-2 py-1 rounded-full text-xs font-medium ${getStatusColor(device.status)}`}>
              {device.status.charAt(0).toUpperCase() + device.status.slice(1)}
            </div>
          </div>
        ))}
      </div>

      <div className="mt-4 pt-4 border-t border-gray-100">
        <button
          onClick={() => {
            devices.forEach(device => {
              publish(device.topic.replace('/status', '/ping'), 'PING');
            });
          }}
          className="w-full px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700 transition-colors"
        >
          Ping All Devices
        </button>
      </div>
    </div>
  );
}