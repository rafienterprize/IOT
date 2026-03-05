"use client";

import { useState, useEffect } from "react";
import { Wifi, WifiOff, Settings } from "lucide-react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => void;
  publish: (topic: string, message: string) => void;
}

interface DeviceStatus {
  esp32_1: boolean;
  esp32_2: boolean;
  esp32_3: boolean;
  esp32_4: boolean;
  lastSeen1: Date | null;
  lastSeen2: Date | null;
  lastSeen3: Date | null;
  lastSeen4: Date | null;
  status1: string;
  status2: string;
  status3: string;
  status4: string;
}

export default function DeviceStatus({ subscribe, publish }: Props) {
  const [devices, setDevices] = useState<DeviceStatus>({
    esp32_1: false,
    esp32_2: false,
    esp32_3: false,
    esp32_4: false,
    lastSeen1: null,
    lastSeen2: null,
    lastSeen3: null,
    lastSeen4: null,
    status1: "Idle",
    status2: "Idle",
    status3: "Idle",
    status4: "Idle",
  });
  const [showPairing, setShowPairing] = useState(false);
  const [wifiSSID, setWifiSSID] = useState("");
  const [wifiPassword, setWifiPassword] = useState("");

  useEffect(() => {
    // Subscribe to heartbeat messages
    const unsubscribe1 = subscribe("iot/esp32_1/heartbeat", (message) => {
      if (message === "ONLINE") {
        setDevices(prev => ({ ...prev, esp32_1: true, lastSeen1: new Date() }));
      }
    });

    const unsubscribe2 = subscribe("iot/esp32_2/heartbeat", (message) => {
      if (message === "ONLINE") {
        setDevices(prev => ({ ...prev, esp32_2: true, lastSeen2: new Date() }));
      }
    });

    const unsubscribe3 = subscribe("iot/esp32_3/heartbeat", (message) => {
      if (message === "ONLINE") {
        setDevices(prev => ({ ...prev, esp32_3: true, lastSeen3: new Date() }));
      }
    });

    const unsubscribe4 = subscribe("iot/esp32_4/heartbeat", (message) => {
      if (message === "ONLINE") {
        setDevices(prev => ({ ...prev, esp32_4: true, lastSeen4: new Date() }));
      }
    });

    // Subscribe to status messages
    const unsubscribeStatus1 = subscribe("iot/esp32_1/status", (message) => {
      setDevices(prev => ({ ...prev, status1: message }));
    });

    const unsubscribeStatus2 = subscribe("iot/esp32_2/status", (message) => {
      setDevices(prev => ({ ...prev, status2: message }));
    });

    const unsubscribeStatus3 = subscribe("iot/esp32_3/status", (message) => {
      setDevices(prev => ({ ...prev, status3: message }));
    });

    const unsubscribeStatus4 = subscribe("iot/esp32_4/status", (message) => {
      setDevices(prev => ({ ...prev, status4: message }));
    });

    // Check for offline devices every 30 seconds
    const interval = setInterval(() => {
      const now = new Date();
      setDevices(prev => {
        const newState = { ...prev };
        
        if (prev.lastSeen1 && (now.getTime() - prev.lastSeen1.getTime()) > 30000) {
          newState.esp32_1 = false;
        }
        if (prev.lastSeen2 && (now.getTime() - prev.lastSeen2.getTime()) > 30000) {
          newState.esp32_2 = false;
        }
        if (prev.lastSeen3 && (now.getTime() - prev.lastSeen3.getTime()) > 30000) {
          newState.esp32_3 = false;
        }
        if (prev.lastSeen4 && (now.getTime() - prev.lastSeen4.getTime()) > 30000) {
          newState.esp32_4 = false;
        }
        
        return newState;
      });
    }, 5000);

    return () => {
      if (unsubscribe1) unsubscribe1();
      if (unsubscribe2) unsubscribe2();
      if (unsubscribe3) unsubscribe3();
      if (unsubscribe4) unsubscribe4();
      if (unsubscribeStatus1) unsubscribeStatus1();
      if (unsubscribeStatus2) unsubscribeStatus2();
      if (unsubscribeStatus3) unsubscribeStatus3();
      if (unsubscribeStatus4) unsubscribeStatus4();
      clearInterval(interval);
    };
  }, [subscribe]);

  const sendWifiConfig = (deviceId: string) => {
    if (!wifiSSID || !wifiPassword) {
      alert("Masukkan SSID dan Password WiFi!");
      return;
    }

    const config = JSON.stringify({
      ssid: wifiSSID,
      password: wifiPassword,
    });

    if (deviceId === "all") {
      // Broadcast to all ESP32s
      publish(`iot/all/wifi/config`, config);
      alert(`Konfigurasi WiFi dikirim ke SEMUA ESP32!\nSemua device akan restart dan connect ke WiFi baru.`);
    } else {
      // Send to specific ESP32
      publish(`iot/${deviceId}/wifi/config`, config);
      alert(`Konfigurasi WiFi dikirim ke ${deviceId}!`);
    }
    
    setShowPairing(false);
    setWifiSSID("");
    setWifiPassword("");
  };

  return (
    <div className="space-y-4">
      <div className="bg-white rounded-xl shadow-lg p-6">
        <h3 className="text-lg font-bold text-gray-800 mb-4 flex items-center gap-2">
          <Wifi className="w-5 h-5" />
          Status Device ESP32
        </h3>

        <div className="space-y-3">
          {/* ESP32 1 */}
          <div className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
            <div className="flex items-center gap-3">
              <div className={`w-3 h-3 rounded-full ${devices.esp32_1 ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`}></div>
              <div>
                <div className="font-semibold text-gray-800">ESP32 1</div>
                <div className="text-xs text-gray-600">
                  Lamp, Gas, Feeder
                </div>
                {devices.esp32_1 && (
                  <div className="text-xs text-blue-600 font-medium mt-1">
                    ⚡ {devices.status1}
                  </div>
                )}
              </div>
            </div>
            <div className="text-right">
              <div className={`text-sm font-semibold ${devices.esp32_1 ? 'text-green-600' : 'text-red-600'}`}>
                {devices.esp32_1 ? '🟢 Online' : '🔴 Offline'}
              </div>
              {devices.lastSeen1 && (
                <div className="text-xs text-gray-500">
                  {devices.lastSeen1.toLocaleTimeString('id-ID')}
                </div>
              )}
            </div>
          </div>

          {/* ESP32 2 */}
          <div className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
            <div className="flex items-center gap-3">
              <div className={`w-3 h-3 rounded-full ${devices.esp32_2 ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`}></div>
              <div>
                <div className="font-semibold text-gray-800">ESP32 2</div>
                <div className="text-xs text-gray-600">
                  Trash, Clothesline
                </div>
                {devices.esp32_2 && (
                  <div className="text-xs text-blue-600 font-medium mt-1">
                    ⚡ {devices.status2}
                  </div>
                )}
              </div>
            </div>
            <div className="text-right">
              <div className={`text-sm font-semibold ${devices.esp32_2 ? 'text-green-600' : 'text-red-600'}`}>
                {devices.esp32_2 ? '🟢 Online' : '🔴 Offline'}
              </div>
              {devices.lastSeen2 && (
                <div className="text-xs text-gray-500">
                  {devices.lastSeen2.toLocaleTimeString('id-ID')}
                </div>
              )}
            </div>
          </div>

          {/* ESP32 3 */}
          <div className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
            <div className="flex items-center gap-3">
              <div className={`w-3 h-3 rounded-full ${devices.esp32_3 ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`}></div>
              <div>
                <div className="font-semibold text-gray-800">ESP32 3</div>
                <div className="text-xs text-gray-600">
                  Smart Door Lock
                </div>
                {devices.esp32_3 && (
                  <div className="text-xs text-blue-600 font-medium mt-1">
                    ⚡ {devices.status3}
                  </div>
                )}
              </div>
            </div>
            <div className="text-right">
              <div className={`text-sm font-semibold ${devices.esp32_3 ? 'text-green-600' : 'text-red-600'}`}>
                {devices.esp32_3 ? '🟢 Online' : '🔴 Offline'}
              </div>
              {devices.lastSeen3 && (
                <div className="text-xs text-gray-500">
                  {devices.lastSeen3.toLocaleTimeString('id-ID')}
                </div>
              )}
            </div>
          </div>

          {/* ESP32 4 */}
          <div className="flex items-center justify-between p-3 bg-gradient-to-r from-purple-50 to-pink-50 rounded-lg border-2 border-purple-200">
            <div className="flex items-center gap-3">
              <div className={`w-3 h-3 rounded-full ${devices.esp32_4 ? 'bg-purple-500 animate-pulse' : 'bg-red-500'}`}></div>
              <div>
                <div className="font-semibold text-purple-800">ESP32 4</div>
                <div className="text-xs text-purple-600 font-medium">
                  WiFi Controller & LCD Display
                </div>
                {devices.esp32_4 && (
                  <div className="text-xs text-purple-700 font-medium mt-1">
                    ⚡ {devices.status4}
                  </div>
                )}
              </div>
            </div>
            <div className="text-right">
              <div className={`text-sm font-semibold ${devices.esp32_4 ? 'text-green-600' : 'text-red-600'}`}>
                {devices.esp32_4 ? '🟢 Online' : '🔴 Offline'}
              </div>
              {devices.lastSeen4 && (
                <div className="text-xs text-gray-500">
                  {devices.lastSeen4.toLocaleTimeString('id-ID')}
                </div>
              )}
            </div>
          </div>
        </div>

        <button
          onClick={() => setShowPairing(!showPairing)}
          className="w-full mt-4 py-2 bg-indigo-500 text-white rounded-lg hover:bg-indigo-600 flex items-center justify-center gap-2"
        >
          <Settings className="w-4 h-4" />
          WiFi Pairing Mode
        </button>
      </div>

      {/* WiFi Pairing Modal */}
      {showPairing && (
        <div className="bg-white rounded-xl shadow-lg p-6">
          <h3 className="text-lg font-bold text-gray-800 mb-4">
            Konfigurasi WiFi ESP32
          </h3>

          <div className="space-y-4">
            <div>
              <label className="text-sm font-semibold text-gray-700">WiFi SSID</label>
              <input
                type="text"
                value={wifiSSID}
                onChange={(e) => setWifiSSID(e.target.value)}
                placeholder="Nama WiFi"
                className="w-full px-4 py-2 border rounded-lg mt-1"
              />
            </div>

            <div>
              <label className="text-sm font-semibold text-gray-700">WiFi Password</label>
              <input
                type="password"
                value={wifiPassword}
                onChange={(e) => setWifiPassword(e.target.value)}
                placeholder="Password WiFi"
                className="w-full px-4 py-2 border rounded-lg mt-1"
              />
            </div>

            <div className="grid grid-cols-2 gap-3 mb-3">
              <button
                onClick={() => sendWifiConfig("esp32_1")}
                className="py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 text-sm"
              >
                ESP32 1
              </button>
              <button
                onClick={() => sendWifiConfig("esp32_2")}
                className="py-2 bg-green-500 text-white rounded-lg hover:bg-green-600 text-sm"
              >
                ESP32 2
              </button>
            </div>
            
            <div className="grid grid-cols-2 gap-3">
              <button
                onClick={() => sendWifiConfig("esp32_3")}
                className="py-2 bg-purple-500 text-white rounded-lg hover:bg-purple-600 text-sm"
              >
                ESP32 3
              </button>
              <button
                onClick={() => sendWifiConfig("esp32_4")}
                className="py-2 bg-pink-500 text-white rounded-lg hover:bg-pink-600 text-sm"
              >
                ESP32 4
              </button>
            </div>

            <button
              onClick={() => sendWifiConfig("all")}
              className="w-full py-3 bg-gradient-to-r from-orange-500 to-red-500 text-white rounded-lg hover:opacity-90 text-sm font-bold"
            >
              🔥 ALL ESP32 (1, 2, 3, 4)
            </button>

            <button
              onClick={() => setShowPairing(false)}
              className="w-full py-2 bg-gray-300 text-gray-700 rounded-lg hover:bg-gray-400"
            >
              Tutup
            </button>

            <div className="text-xs text-gray-600 bg-yellow-50 p-3 rounded">
              <strong>Catatan:</strong> Pastikan ESP32 dalam mode pairing (tekan tombol BOOT 3 detik)
            </div>
          </div>
        </div>
      )}

      {/* Notifications */}
      {!devices.esp32_1 && (
        <div className="bg-red-100 border border-red-300 rounded-lg p-4 flex items-start gap-3">
          <WifiOff className="w-5 h-5 text-red-600 mt-0.5" />
          <div>
            <div className="font-semibold text-red-800">ESP32 1 Offline</div>
            <div className="text-sm text-red-700">
              Device tidak terhubung. Cek koneksi WiFi atau power supply.
            </div>
          </div>
        </div>
      )}

      {!devices.esp32_2 && (
        <div className="bg-red-100 border border-red-300 rounded-lg p-4 flex items-start gap-3">
          <WifiOff className="w-5 h-5 text-red-600 mt-0.5" />
          <div>
            <div className="font-semibold text-red-800">ESP32 2 Offline</div>
            <div className="text-sm text-red-700">
              Device tidak terhubung. Cek koneksi WiFi atau power supply.
            </div>
          </div>
        </div>
      )}

      {!devices.esp32_3 && (
        <div className="bg-red-100 border border-red-300 rounded-lg p-4 flex items-start gap-3">
          <WifiOff className="w-5 h-5 text-red-600 mt-0.5" />
          <div>
            <div className="font-semibold text-red-800">ESP32 3 Offline</div>
            <div className="text-sm text-red-700">
              Device tidak terhubung. Cek koneksi WiFi atau power supply.
            </div>
          </div>
        </div>
      )}

      {!devices.esp32_4 && (
        <div className="bg-red-100 border border-red-300 rounded-lg p-4 flex items-start gap-3">
          <WifiOff className="w-5 h-5 text-red-600 mt-0.5" />
          <div>
            <div className="font-semibold text-red-800">ESP32 4 Offline</div>
            <div className="text-sm text-red-700">
              WiFi Controller tidak terhubung. Sistem WiFi dan LCD tidak berfungsi!
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
