"use client";

import { useState, useEffect } from "react";
import { DoorOpen, DoorClosed, CreditCard, Lock, UserPlus, Trash2 } from "lucide-react";

interface Props {
  publish: (topic: string, message: string) => void;
  subscribe: (topic: string, callback: (message: string) => void) => void;
}

interface RegisteredCard {
  uid: string;
  name: string;
  addedAt: string;
}

interface AccessLog {
  time: string;
  method: string;
  status: string;
  identifier: string;
}

export default function SmartDoor({ publish, subscribe }: Props) {
  const [isLocked, setIsLocked] = useState(true);
  const [registeredCards, setRegisteredCards] = useState<RegisteredCard[]>([]);
  const [registeredPins, setRegisteredPins] = useState<string[]>([]);
  const [accessLogs, setAccessLogs] = useState<AccessLog[]>([]);
  
  const [showAddCard, setShowAddCard] = useState(false);
  const [showAddPin, setShowAddPin] = useState(false);
  const [newCardName, setNewCardName] = useState("");
  const [newPin, setNewPin] = useState("");
  const [isRegistering, setIsRegistering] = useState(false);
  const [scannedUID, setScannedUID] = useState("");
  const [scanMode, setScanMode] = useState(false);
  
  const [lcdMessage, setLcdMessage] = useState("Ready to Scan");

  useEffect(() => {
    // Subscribe to door status
    const unsubscribeDoor = subscribe("iot/door/status", (message) => {
      setIsLocked(message === "LOCKED");
    });

    // Subscribe to RFID scan
    const unsubscribeRfid = subscribe("iot/door/rfid/scan", (message) => {
      const data = JSON.parse(message);
      
      // If in scan mode, capture the UID
      if (scanMode && data.uid) {
        setScannedUID(data.uid);
        setLcdMessage("Card Scanned!");
      }
      
      handleRfidScan(data);
    });

    // Subscribe to PIN entry
    const unsubscribePin = subscribe("iot/door/pin/entry", (message) => {
      const data = JSON.parse(message);
      handlePinEntry(data);
    });

    // Subscribe to LCD updates
    const unsubscribeLcd = subscribe("iot/door/lcd", (message) => {
      setLcdMessage(message);
    });

    // Subscribe to card registered confirmation
    const unsubscribeRegistered = subscribe("iot/door/rfid/registered", (message) => {
      const data = JSON.parse(message);
      const newCard: RegisteredCard = {
        uid: data.uid,
        name: data.name,
        addedAt: new Date().toLocaleString('id-ID')
      };
      setRegisteredCards(prev => [...prev, newCard]);
      setIsRegistering(false);
      setScanMode(false);
      setScannedUID("");
      setNewCardName("");
    });

    // Subscribe to access logs
    const unsubscribeLog = subscribe("iot/door/access/log", (message) => {
      const log = JSON.parse(message);
      setAccessLogs(prev => [log, ...prev].slice(0, 10));
    });

    return () => {
      if (unsubscribeDoor) unsubscribeDoor();
      if (unsubscribeRfid) unsubscribeRfid();
      if (unsubscribePin) unsubscribePin();
      if (unsubscribeLcd) unsubscribeLcd();
      if (unsubscribeLog) unsubscribeLog();
      if (unsubscribeRegistered) unsubscribeRegistered();
    };
  }, [subscribe, scanMode]);

  const handleRfidScan = (data: { uid: string; status: string }) => {
    if (data.status === "ACCESS_GRANTED") {
      setIsLocked(false);
      setTimeout(() => setIsLocked(true), 5000);
    }
  };

  const handlePinEntry = (data: { pin: string; status: string }) => {
    if (data.status === "ACCESS_GRANTED") {
      setIsLocked(false);
      setTimeout(() => setIsLocked(true), 5000);
    }
  };

  const startScanMode = () => {
    setScanMode(true);
    setScannedUID("");
    setShowAddCard(true);
    publish("iot/door/rfid/scanmode", "START");
    setLcdMessage("Scan Mode Active");
  };

  const cancelScanMode = () => {
    setScanMode(false);
    setScannedUID("");
    setShowAddCard(false);
    setNewCardName("");
    publish("iot/door/rfid/scanmode", "STOP");
    setLcdMessage("Ready to Scan");
  };

  const registerScannedCard = () => {
    if (!newCardName) {
      alert("Masukkan nama untuk kartu!");
      return;
    }
    
    if (!scannedUID) {
      alert("Belum ada kartu yang di-scan!");
      return;
    }

    setIsRegistering(true);
    publish("iot/door/rfid/register", JSON.stringify({ 
      uid: scannedUID, 
      name: newCardName 
    }));
    
    setLcdMessage("Registering...");
  };

  const startCardRegistration = () => {
    if (!newCardName) {
      alert("Masukkan nama untuk kartu!");
      return;
    }
    setIsRegistering(true);
    publish("iot/door/rfid/register", JSON.stringify({ mode: "START", name: newCardName }));
    setLcdMessage("Tap Card Now...");
    
    setTimeout(() => {
      setIsRegistering(false);
      setShowAddCard(false);
      setNewCardName("");
    }, 10000);
  };

  const addPin = () => {
    if (newPin.length !== 4 || !/^\d+$/.test(newPin)) {
      alert("PIN harus 4 digit angka!");
      return;
    }
    
    if (registeredPins.includes(newPin)) {
      alert("PIN sudah terdaftar!");
      return;
    }

    publish("iot/door/pin/register", JSON.stringify({ pin: newPin }));
    setRegisteredPins(prev => [...prev, newPin]);
    setNewPin("");
    setShowAddPin(false);
    alert("PIN berhasil ditambahkan!");
  };

  const deleteCard = (uid: string) => {
    if (confirm("Hapus kartu ini?")) {
      publish("iot/door/rfid/delete", uid);
      setRegisteredCards(prev => prev.filter(card => card.uid !== uid));
    }
  };

  const deletePin = (pin: string) => {
    if (confirm("Hapus PIN ini?")) {
      publish("iot/door/pin/delete", pin);
      setRegisteredPins(prev => prev.filter(p => p !== pin));
    }
  };

  const manualToggle = () => {
    const newState = !isLocked;
    setIsLocked(newState);
    publish("iot/door/control", newState ? "LOCK" : "UNLOCK");
  };

  const emergencyUnlock = () => {
    setIsLocked(false);
    publish("iot/door/emergency", "UNLOCK");
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-3">
          {isLocked ? (
            <DoorClosed className="w-8 h-8 text-red-600" />
          ) : (
            <DoorOpen className="w-8 h-8 text-green-600" />
          )}
          <h2 className="text-xl font-bold text-gray-800">Smart Door Lock</h2>
        </div>
        <div className={`px-3 py-1 rounded-full text-sm font-semibold ${
          isLocked ? 'bg-red-100 text-red-700' : 'bg-green-100 text-green-700'
        }`}>
          {isLocked ? '🔒 LOCKED' : '🔓 UNLOCKED'}
        </div>
      </div>

      {/* Manual Control */}
      <div className="grid grid-cols-2 gap-3 mb-4">
        <button
          onClick={manualToggle}
          className={`py-3 rounded-lg font-semibold transition-colors ${
            isLocked
              ? 'bg-green-500 text-white hover:bg-green-600'
              : 'bg-red-500 text-white hover:bg-red-600'
          }`}
        >
          {isLocked ? 'Unlock Door' : 'Lock Door'}
        </button>
        <button
          onClick={emergencyUnlock}
          className="py-3 bg-orange-500 text-white rounded-lg hover:bg-orange-600 font-semibold"
        >
          🚨 Emergency
        </button>
      </div>

      {/* RFID Cards Management */}
      <div className="mb-4 border-t pt-4">
        <div className="flex items-center justify-between mb-3">
          <div className="flex items-center gap-2">
            <CreditCard className="w-5 h-5 text-blue-600" />
            <span className="font-semibold text-gray-800">Registered Cards</span>
            <span className="text-sm text-gray-500">({registeredCards.length})</span>
          </div>
          <button
            onClick={() => setShowAddCard(!showAddCard)}
            className="px-3 py-1 bg-blue-500 text-white rounded-lg hover:bg-blue-600 text-sm flex items-center gap-1"
          >
            <UserPlus className="w-4 h-4" />
            Add Card
          </button>
        </div>

        {showAddCard && (
          <div className="bg-blue-50 p-4 rounded-lg mb-3 space-y-3">
            {!scanMode ? (
              <button
                onClick={startScanMode}
                className="w-full py-3 bg-blue-500 text-white rounded-lg hover:bg-blue-600 font-semibold flex items-center justify-center gap-2"
              >
                <CreditCard className="w-5 h-5" />
                Start Scan Mode
              </button>
            ) : (
              <>
                <div className="bg-white p-3 rounded border-2 border-blue-300">
                  <div className="text-sm text-gray-600 mb-1">Scanned UID:</div>
                  <div className="font-mono text-lg font-bold text-blue-700">
                    {scannedUID || "Waiting for card..."}
                  </div>
                  {scannedUID && (
                    <div className="text-xs text-green-600 mt-1 flex items-center gap-1">
                      ✓ Card detected! Enter name below
                    </div>
                  )}
                </div>

                <input
                  type="text"
                  value={newCardName}
                  onChange={(e) => setNewCardName(e.target.value)}
                  placeholder="Nama pemilik kartu"
                  className="w-full px-3 py-2 border rounded-lg"
                  disabled={!scannedUID}
                />

                <div className="grid grid-cols-2 gap-2">
                  <button
                    onClick={registerScannedCard}
                    disabled={!scannedUID || !newCardName || isRegistering}
                    className={`py-2 rounded-lg font-semibold ${
                      !scannedUID || !newCardName || isRegistering
                        ? 'bg-gray-300 text-gray-500'
                        : 'bg-green-500 text-white hover:bg-green-600'
                    }`}
                  >
                    {isRegistering ? 'Saving...' : 'Register Card'}
                  </button>
                  <button
                    onClick={cancelScanMode}
                    className="py-2 bg-red-500 text-white rounded-lg hover:bg-red-600 font-semibold"
                  >
                    Cancel
                  </button>
                </div>

                <div className="text-xs text-gray-600 bg-yellow-50 p-2 rounded">
                  💡 Tap kartu RFID ke reader untuk scan UID
                </div>
              </>
            )}
          </div>
        )}

        <div className="space-y-2 max-h-40 overflow-y-auto">
          {registeredCards.length === 0 ? (
            <div className="text-sm text-gray-500 text-center py-2">
              Belum ada kartu terdaftar
            </div>
          ) : (
            registeredCards.map((card) => (
              <div key={card.uid} className="flex items-center justify-between bg-gray-50 p-2 rounded">
                <div>
                  <div className="font-semibold text-sm">{card.name}</div>
                  <div className="text-xs text-gray-500">UID: {card.uid}</div>
                </div>
                <button
                  onClick={() => deleteCard(card.uid)}
                  className="text-red-500 hover:text-red-700"
                >
                  <Trash2 className="w-4 h-4" />
                </button>
              </div>
            ))
          )}
        </div>
      </div>

      {/* PIN Management */}
      <div className="mb-4 border-t pt-4">
        <div className="flex items-center justify-between mb-3">
          <div className="flex items-center gap-2">
            <Lock className="w-5 h-5 text-purple-600" />
            <span className="font-semibold text-gray-800">Registered PINs</span>
            <span className="text-sm text-gray-500">({registeredPins.length})</span>
          </div>
          <button
            onClick={() => setShowAddPin(!showAddPin)}
            className="px-3 py-1 bg-purple-500 text-white rounded-lg hover:bg-purple-600 text-sm flex items-center gap-1"
          >
            <UserPlus className="w-4 h-4" />
            Add PIN
          </button>
        </div>

        {showAddPin && (
          <div className="bg-purple-50 p-3 rounded-lg mb-3">
            <input
              type="password"
              value={newPin}
              onChange={(e) => setNewPin(e.target.value.slice(0, 4))}
              placeholder="4 digit PIN"
              maxLength={4}
              className="w-full px-3 py-2 border rounded-lg mb-2 text-center text-2xl tracking-widest"
            />
            <button
              onClick={addPin}
              className="w-full py-2 bg-purple-500 text-white rounded-lg hover:bg-purple-600 font-semibold"
            >
              Add PIN
            </button>
          </div>
        )}

        <div className="space-y-2 max-h-32 overflow-y-auto">
          {registeredPins.length === 0 ? (
            <div className="text-sm text-gray-500 text-center py-2">
              Belum ada PIN terdaftar
            </div>
          ) : (
            registeredPins.map((pin, index) => (
              <div key={index} className="flex items-center justify-between bg-gray-50 p-2 rounded">
                <div className="font-mono text-lg">****</div>
                <button
                  onClick={() => deletePin(pin)}
                  className="text-red-500 hover:text-red-700"
                >
                  <Trash2 className="w-4 h-4" />
                </button>
              </div>
            ))
          )}
        </div>
      </div>

      {/* Access Logs */}
      <div className="border-t pt-4">
        <div className="font-semibold text-gray-800 mb-2">Recent Access</div>
        <div className="space-y-1 max-h-40 overflow-y-auto">
          {accessLogs.length === 0 ? (
            <div className="text-sm text-gray-500 text-center py-2">
              Belum ada aktivitas
            </div>
          ) : (
            accessLogs.map((log, index) => (
              <div key={index} className={`text-xs p-2 rounded ${
                log.status === 'GRANTED' ? 'bg-green-50' : 'bg-red-50'
              }`}>
                <span className="font-semibold">{log.time}</span> - 
                <span className="ml-1">{log.method}</span> - 
                <span className={`ml-1 font-semibold ${
                  log.status === 'GRANTED' ? 'text-green-700' : 'text-red-700'
                }`}>
                  {log.status}
                </span>
              </div>
            ))
          )}
        </div>
      </div>
    </div>
  );
}
