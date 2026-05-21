"use client";

import { useState, useEffect } from "react";
import { Car, Shield, Radio, UserPlus, Trash2 } from "lucide-react";

interface Props {
  subscribe: (topic: string, callback: (message: string) => void) => void;
  publish: (topic: string, message: string) => void;
}

interface RegisteredCard {
  uid: string;
  name: string;
  addedAt: string;
}

export default function SmartGate({ subscribe, publish }: Props) {
  const [gateStatus, setGateStatus] = useState("CLOSED");
  const [carDetected, setCarDetected] = useState(false);
  const [registeredCards, setRegisteredCards] = useState<RegisteredCard[]>([]);
  const [showRegister, setShowRegister] = useState(false);
  const [newCardName, setNewCardName] = useState("");
  const [scanMode, setScanMode] = useState(false);
  const [scannedUID, setScannedUID] = useState("");

  useEffect(() => {
    const unsubscribe1 = subscribe("iot/gate/status", (message) => {
      setGateStatus(message);
      if (message === "CAR_DETECTED") {
        setCarDetected(true);
      } else if (message === "IDLE" || message === "CLOSED") {
        setCarDetected(false);
      }
    });

    const unsubscribe2 = subscribe("iot/gate/access/log", (message) => {
      try {
        const log = JSON.parse(message);
        console.log("Gate access log:", log);
      } catch (e) {
        console.error("Failed to parse gate log:", e);
      }
    });

    const unsubscribe3 = subscribe("iot/gate/rfid/scan", (message) => {
      try {
        const data = JSON.parse(message);
        if (data.status === "SCANNED" && scanMode) {
          setScannedUID(data.uid);
        }
      } catch (e) {
        console.error("Failed to parse RFID scan:", e);
      }
    });

    const unsubscribe4 = subscribe("iot/gate/rfid/registered", (message) => {
      try {
        const card = JSON.parse(message);
        setRegisteredCards(prev => [...prev, card]);
        setScanMode(false);
        setShowRegister(false);
        setNewCardName("");
        setScannedUID("");
      } catch (e) {
        console.error("Failed to parse registered card:", e);
      }
    });

    return () => {
      if (unsubscribe1) unsubscribe1();
      if (unsubscribe2) unsubscribe2();
      if (unsubscribe3) unsubscribe3();
      if (unsubscribe4) unsubscribe4();
    };
  }, [subscribe, scanMode]);

  const openGate = () => {
    publish("iot/gate/control", "OPEN");
  };

  const closeGate = () => {
    publish("iot/gate/control", "CLOSE");
  };

  const startScanMode = () => {
    setScanMode(true);
    setScannedUID("");
    publish("iot/gate/rfid/scanmode", "START");
  };

  const stopScanMode = () => {
    setScanMode(false);
    setScannedUID("");
    publish("iot/gate/rfid/scanmode", "STOP");
  };

  const registerCard = () => {
    if (!scannedUID || !newCardName) {
      alert("Scan kartu dulu dan isi nama!");
      return;
    }

    const data = JSON.stringify({
      uid: scannedUID,
      name: newCardName,
    });

    publish("iot/gate/rfid/register", data);
  };

  const deleteCard = (uid: string) => {
    if (confirm("Hapus kartu ini?")) {
      publish("iot/gate/rfid/delete", uid);
      setRegisteredCards(prev => prev.filter(card => card.uid !== uid));
    }
  };

  const getStatusColor = () => {
    if (gateStatus === "OPEN") return "bg-green-500";
    if (carDetected) return "bg-yellow-500 animate-pulse";
    return "bg-red-500";
  };

  const getStatusText = () => {
    if (gateStatus === "OPEN") return "TERBUKA";
    if (carDetected) return "MOBIL TERDETEKSI";
    return "TERTUTUP";
  };

  return (
    <div className="space-y-4">
      {/* Status Card */}
      <div className="bg-white rounded-xl shadow-lg p-6">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-3">
            <Car className="w-6 h-6 text-indigo-600" />
            <h3 className="text-lg font-bold text-gray-800">Smart Gate</h3>
          </div>
          <div className={`px-4 py-2 rounded-full text-white font-semibold ${getStatusColor()}`}>
            {getStatusText()}
          </div>
        </div>

        {/* Gate Visual */}
        <div className="bg-gray-100 rounded-lg p-6 mb-4">
          <div className="flex items-center justify-center gap-4">
            {/* Left Gate */}
            <div 
              className={`w-20 h-32 bg-gradient-to-r from-gray-600 to-gray-700 rounded transition-all duration-1000 ${
                gateStatus === "OPEN" ? "-translate-x-12 rotate-[-45deg]" : ""
              }`}
              style={{ transformOrigin: "right center" }}
            >
              <div className="h-full flex items-center justify-center text-white text-xs">
                GATE
              </div>
            </div>

            {/* Center - Car Icon */}
            <div className="relative">
              {carDetected && (
                <Car className="w-16 h-16 text-blue-600 animate-bounce" />
              )}
              {!carDetected && (
                <div className="w-16 h-16 flex items-center justify-center text-gray-400">
                  <Car className="w-12 h-12" />
                </div>
              )}
            </div>

            {/* Right Gate */}
            <div 
              className={`w-20 h-32 bg-gradient-to-l from-gray-600 to-gray-700 rounded transition-all duration-1000 ${
                gateStatus === "OPEN" ? "translate-x-12 rotate-[45deg]" : ""
              }`}
              style={{ transformOrigin: "left center" }}
            >
              <div className="h-full flex items-center justify-center text-white text-xs">
                GATE
              </div>
            </div>
          </div>
        </div>

        {/* Car Detection Alert */}
        {carDetected && (
          <div className="bg-yellow-100 border-l-4 border-yellow-500 p-4 mb-4">
            <div className="flex items-center gap-2">
              <Shield className="w-5 h-5 text-yellow-600" />
              <div>
                <div className="font-semibold text-yellow-800">Mobil Terdeteksi!</div>
                <div className="text-sm text-yellow-700">Silakan scan kartu RFID untuk membuka gerbang</div>
              </div>
            </div>
          </div>
        )}

        {/* Manual Controls */}
        <div className="grid grid-cols-2 gap-3">
          <button
            onClick={openGate}
            className="py-3 bg-green-500 text-white rounded-lg hover:bg-green-600 font-semibold"
          >
            🚗 Buka Gerbang
          </button>
          <button
            onClick={closeGate}
            className="py-3 bg-red-500 text-white rounded-lg hover:bg-red-600 font-semibold"
          >
            🚫 Tutup Gerbang
          </button>
        </div>
      </div>

      {/* RFID Card Management */}
      <div className="bg-white rounded-xl shadow-lg p-6">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-3">
            <Radio className="w-6 h-6 text-indigo-600" />
            <h3 className="text-lg font-bold text-gray-800">Kartu RFID Terdaftar</h3>
          </div>
          <button
            onClick={() => setShowRegister(!showRegister)}
            className="px-4 py-2 bg-indigo-500 text-white rounded-lg hover:bg-indigo-600 flex items-center gap-2"
          >
            <UserPlus className="w-4 h-4" />
            Tambah Kartu
          </button>
        </div>

        {/* Register Form */}
        {showRegister && (
          <div className="bg-indigo-50 rounded-lg p-4 mb-4">
            <h4 className="font-semibold text-gray-800 mb-3">Daftarkan Kartu Baru</h4>
            
            {!scanMode && (
              <button
                onClick={startScanMode}
                className="w-full py-3 bg-indigo-500 text-white rounded-lg hover:bg-indigo-600 font-semibold mb-3"
              >
                📡 Mulai Scan Kartu
              </button>
            )}

            {scanMode && (
              <div className="space-y-3">
                <div className="bg-white p-4 rounded-lg border-2 border-indigo-300 animate-pulse">
                  <div className="text-center">
                    <Radio className="w-12 h-12 text-indigo-600 mx-auto mb-2" />
                    <div className="font-semibold text-indigo-800">Scan kartu sekarang...</div>
                    {scannedUID && (
                      <div className="mt-2 text-sm text-green-600">
                        ✓ Kartu terdeteksi: {scannedUID}
                      </div>
                    )}
                  </div>
                </div>

                {scannedUID && (
                  <input
                    type="text"
                    value={newCardName}
                    onChange={(e) => setNewCardName(e.target.value)}
                    placeholder="Nama pemilik kartu"
                    className="w-full px-4 py-2 border rounded-lg"
                  />
                )}

                <div className="grid grid-cols-2 gap-2">
                  <button
                    onClick={registerCard}
                    disabled={!scannedUID || !newCardName}
                    className="py-2 bg-green-500 text-white rounded-lg hover:bg-green-600 disabled:opacity-50 disabled:cursor-not-allowed"
                  >
                    💾 Simpan
                  </button>
                  <button
                    onClick={stopScanMode}
                    className="py-2 bg-gray-500 text-white rounded-lg hover:bg-gray-600"
                  >
                    ✕ Batal
                  </button>
                </div>
              </div>
            )}
          </div>
        )}

        {/* Registered Cards List */}
        <div className="space-y-2">
          {registeredCards.length === 0 && (
            <div className="text-center py-8 text-gray-500">
              <Radio className="w-12 h-12 mx-auto mb-2 opacity-50" />
              <div>Belum ada kartu terdaftar</div>
            </div>
          )}

          {registeredCards.map((card, index) => (
            <div key={`card-${card.uid}-${index}`} className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
              <div>
                <div className="font-semibold text-gray-800">{card.name}</div>
                <div className="text-xs text-gray-600">UID: {card.uid}</div>
              </div>
              <button
                onClick={() => deleteCard(card.uid)}
                className="p-2 text-red-500 hover:bg-red-100 rounded-lg"
              >
                <Trash2 className="w-4 h-4" />
              </button>
            </div>
          ))}
        </div>
      </div>

      {/* Info */}
      <div className="bg-blue-50 rounded-lg p-4 text-sm text-blue-800">
        <div className="font-semibold mb-2">ℹ️ Cara Kerja:</div>
        <ol className="list-decimal list-inside space-y-1">
          <li>Mobil berhenti di depan gerbang (sensor jarak &lt; 50cm)</li>
          <li>Sistem mendeteksi mobil dan menunggu scan kartu RFID</li>
          <li>Scan kartu yang sudah terdaftar</li>
          <li>Gerbang otomatis terbuka selama 5 detik</li>
          <li>Gerbang otomatis tertutup kembali</li>
        </ol>
      </div>
    </div>
  );
}
