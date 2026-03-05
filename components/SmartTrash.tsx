"use client";

import { useState, useEffect } from "react";
import { Trash2, AlertCircle, Recycle } from "lucide-react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => void;
  publish: (topic: string, message: string) => void;
}

interface TrashBin {
  type: string;
  level: number;
  color: string;
  icon: string;
}

export default function SmartTrash({ subscribe, publish }: Props) {
  const [bins, setBins] = useState<TrashBin[]>([
    { type: "Organik", level: 0, color: "green", icon: "🍃" },
    { type: "Anorganik", level: 0, color: "blue", icon: "♻️" },
    { type: "Metal", level: 0, color: "gray", icon: "🔩" },
  ]);
  
  const [currentRotation, setCurrentRotation] = useState(0);
  const [lastDetection, setLastDetection] = useState("");
  const [isProcessing, setIsProcessing] = useState(false);
  const [totalItems, setTotalItems] = useState({ organik: 0, anorganik: 0, metal: 0 });

  useEffect(() => {
    // Subscribe to bin levels
    const unsubscribeOrganik = subscribe("iot/trash/organik/level", (message) => {
      const level = parseInt(message);
      setBins(prev => prev.map(bin => 
        bin.type === "Organik" ? { ...bin, level } : bin
      ));
    });

    const unsubscribeAnorganik = subscribe("iot/trash/anorganik/level", (message) => {
      const level = parseInt(message);
      setBins(prev => prev.map(bin => 
        bin.type === "Anorganik" ? { ...bin, level } : bin
      ));
    });

    const unsubscribeMetal = subscribe("iot/trash/metal/level", (message) => {
      const level = parseInt(message);
      setBins(prev => prev.map(bin => 
        bin.type === "Metal" ? { ...bin, level } : bin
      ));
    });

    // Subscribe to detection events
    const unsubscribeDetection = subscribe("iot/trash/detection", (message) => {
      const data = JSON.parse(message);
      setLastDetection(data.type);
      setCurrentRotation(data.rotation);
      setIsProcessing(true);
      
      // Update item count
      setTotalItems(prev => ({
        ...prev,
        [data.type.toLowerCase()]: prev[data.type.toLowerCase() as keyof typeof prev] + 1
      }));

      setTimeout(() => setIsProcessing(false), 3000);
    });

    // Subscribe to rotation status
    const unsubscribeRotation = subscribe("iot/trash/rotation", (message) => {
      setCurrentRotation(parseInt(message));
    });

    return () => {
      if (unsubscribeOrganik) unsubscribeOrganik();
      if (unsubscribeAnorganik) unsubscribeAnorganik();
      if (unsubscribeMetal) unsubscribeMetal();
      if (unsubscribeDetection) unsubscribeDetection();
      if (unsubscribeRotation) unsubscribeRotation();
    };
  }, [subscribe]);

  const manualRotate = (binType: string) => {
    let rotation = 0;
    if (binType === "Organik") rotation = 0;
    else if (binType === "Anorganik") rotation = 120;
    else if (binType === "Metal") rotation = 240;
    
    publish("iot/trash/rotate", rotation.toString());
    setCurrentRotation(rotation);
  };

  const resetSystem = () => {
    publish("iot/trash/reset", "RESET");
    setCurrentRotation(0);
    setLastDetection("");
  };

  const getBinColor = (type: string) => {
    if (type === "Organik") return "bg-green-500";
    if (type === "Anorganik") return "bg-blue-500";
    return "bg-gray-500";
  };

  const getBinBorderColor = (type: string) => {
    if (type === "Organik") return "border-green-500";
    if (type === "Anorganik") return "border-blue-500";
    return "border-gray-500";
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-3">
          <Recycle className="w-8 h-8 text-green-600" />
          <h2 className="text-xl font-bold text-gray-800">Smart Trash Sorting</h2>
        </div>
        {isProcessing && (
          <div className="flex items-center gap-2 bg-blue-100 px-3 py-1 rounded-full">
            <div className="w-2 h-2 bg-blue-500 rounded-full animate-pulse"></div>
            <span className="text-xs font-semibold text-blue-700">Memproses...</span>
          </div>
        )}
      </div>

      {/* Detection Status */}
      {lastDetection && (
        <div className="mb-4 p-3 bg-gradient-to-r from-green-50 to-blue-50 rounded-lg border border-green-200">
          <div className="flex items-center gap-2">
            <AlertCircle className="w-5 h-5 text-green-600" />
            <span className="text-sm font-semibold text-gray-700">
              Terdeteksi: <span className="text-green-700">{lastDetection}</span> → Rotasi {currentRotation}°
            </span>
          </div>
        </div>
      )}

      {/* Trash Bins */}
      <div className="grid grid-cols-3 gap-3 mb-4">
        {bins.map((bin) => {
          const isFull = bin.level >= 80;
          const isActive = 
            (bin.type === "Organik" && currentRotation === 0) ||
            (bin.type === "Anorganik" && currentRotation === 120) ||
            (bin.type === "Metal" && currentRotation === 240);

          return (
            <div
              key={bin.type}
              className={`p-4 rounded-lg border-2 transition-all ${
                isActive ? `${getBinBorderColor(bin.type)} bg-opacity-10` : 'border-gray-200'
              } ${isFull ? 'bg-red-50' : 'bg-gray-50'}`}
            >
              <div className="text-center mb-2">
                <div className="text-3xl mb-1">{bin.icon}</div>
                <div className="font-semibold text-gray-800 text-sm">{bin.type}</div>
              </div>

              <div className="relative mb-2">
                <div className="w-full bg-gray-200 rounded-full h-3 overflow-hidden">
                  <div
                    className={`h-full ${getBinColor(bin.type)} transition-all duration-500`}
                    style={{ width: `${bin.level}%` }}
                  ></div>
                </div>
                <div className="text-center text-xs font-semibold text-gray-600 mt-1">
                  {bin.level}%
                </div>
              </div>

              {isFull && (
                <div className="text-xs text-red-600 font-semibold text-center">
                  ⚠️ Penuh!
                </div>
              )}

              <button
                onClick={() => manualRotate(bin.type)}
                className="w-full mt-2 py-1 text-xs bg-gray-200 hover:bg-gray-300 rounded transition-colors"
              >
                Rotasi
              </button>
            </div>
          );
        })}
      </div>

      {/* Statistics */}
      <div className="grid grid-cols-3 gap-2 mb-4 text-center text-xs">
        <div className="p-2 bg-green-50 rounded">
          <div className="font-bold text-green-700 text-lg">{totalItems.organik}</div>
          <div className="text-gray-600">Organik</div>
        </div>
        <div className="p-2 bg-blue-50 rounded">
          <div className="font-bold text-blue-700 text-lg">{totalItems.anorganik}</div>
          <div className="text-gray-600">Anorganik</div>
        </div>
        <div className="p-2 bg-gray-50 rounded">
          <div className="font-bold text-gray-700 text-lg">{totalItems.metal}</div>
          <div className="text-gray-600">Metal</div>
        </div>
      </div>

      {/* Control Buttons */}
      <div className="flex gap-2">
        <button
          onClick={resetSystem}
          className="flex-1 py-2 bg-orange-500 text-white rounded-lg hover:bg-orange-600 text-sm font-semibold"
        >
          Reset Posisi
        </button>
      </div>

      {/* Info */}
      <div className="mt-4 text-xs text-gray-600 bg-blue-50 p-3 rounded">
        <strong>Sistem Auto-Sorting:</strong> Sensor warna & metal detector akan mendeteksi jenis sampah, 
        lalu servo memutar bin ke posisi yang sesuai (0°, 120°, 240°).
      </div>
    </div>
  );
}
