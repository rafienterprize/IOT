"use client";

import { useState, useEffect } from "react";
import { Fish, Clock } from "lucide-react";

interface Props {
  publish: (topic: string, message: string) => void;
  subscribe: (topic: string, callback: (message: string) => void) => void;
}

export default function FishFeeder({ publish, subscribe }: Props) {
  const [isOpen, setIsOpen] = useState(false);
  const [feedTimes, setFeedTimes] = useState<string[]>([]);
  const [newTime, setNewTime] = useState("");
  const [lastFeed, setLastFeed] = useState<string>("");

  // Load feed times from localStorage on component mount
  useEffect(() => {
    const savedTimes = localStorage.getItem('fishFeederTimes');
    if (savedTimes) {
      try {
        const times = JSON.parse(savedTimes);
        setFeedTimes(times);
      } catch (e) {
        console.error('Failed to parse saved feed times:', e);
        setFeedTimes([]);
      }
    }
  }, []);

  // Save feed times to localStorage whenever they change
  useEffect(() => {
    localStorage.setItem('fishFeederTimes', JSON.stringify(feedTimes));
  }, [feedTimes]);

  useEffect(() => {
    const unsubscribe = subscribe("iot/feeder/status", (message) => {
      if (message === "FED") {
        setLastFeed(new Date().toLocaleTimeString("id-ID"));
        setTimeout(() => setIsOpen(false), 2000);
      }
    });
    return unsubscribe;
  }, [subscribe]);

  const feedNow = () => {
    setIsOpen(true);
    publish("iot/feeder/control", "FEED");
  };

  const addFeedTime = () => {
    if (newTime && !feedTimes.includes(newTime)) {
      const updated = [...feedTimes, newTime].sort();
      setFeedTimes(updated);
      publish("iot/feeder/schedule", JSON.stringify(updated));
      setNewTime("");
    }
  };

  const removeFeedTime = (time: string) => {
    const updated = feedTimes.filter(t => t !== time);
    setFeedTimes(updated);
    publish("iot/feeder/schedule", JSON.stringify(updated));
  };

  const clearAllTimes = () => {
    setFeedTimes([]);
    publish("iot/feeder/schedule", JSON.stringify([]));
    localStorage.removeItem('fishFeederTimes');
  };

  return (
    <div className="bg-white rounded-xl shadow-lg p-6">
      <div className="flex items-center gap-3 mb-4">
        <Fish className="w-8 h-8 text-orange-500" />
        <h2 className="text-xl font-bold text-gray-800">Fish Feeder</h2>
      </div>

      <div className="space-y-4">
        <button
          onClick={feedNow}
          className={`w-full py-3 rounded-lg font-semibold transition-colors ${
            isOpen
              ? 'bg-green-400 text-white'
              : 'bg-orange-500 text-white hover:bg-orange-600'
          }`}
        >
          {isOpen ? '🐟 Memberi Makan...' : 'Beri Makan Sekarang'}
        </button>

        {lastFeed && (
          <div className="text-sm text-gray-600 text-center">
            Terakhir diberi makan: {lastFeed}
          </div>
        )}

        <div className="border-t pt-4">
          <div className="flex items-center justify-between mb-3">
            <div className="flex items-center gap-2">
              <Clock className="w-5 h-5 text-gray-600" />
              <span className="font-semibold text-gray-700">Jadwal Otomatis</span>
            </div>
            {feedTimes.length > 0 && (
              <button
                onClick={clearAllTimes}
                className="text-red-500 hover:text-red-700 text-xs"
              >
                Hapus Semua
              </button>
            )}
          </div>

          <div className="space-y-2 mb-3">
            {feedTimes.length === 0 ? (
              <div className="text-gray-500 text-sm text-center py-2">
                Belum ada jadwal otomatis
              </div>
            ) : (
              feedTimes.map((time) => (
                <div key={time} className="flex items-center justify-between bg-gray-50 p-2 rounded">
                  <span className="text-gray-700">{time}</span>
                  <button
                    onClick={() => removeFeedTime(time)}
                    className="text-red-500 hover:text-red-700 text-sm"
                  >
                    Hapus
                  </button>
                </div>
              ))
            )}
          </div>

          <div className="flex gap-2">
            <input
              type="time"
              value={newTime}
              onChange={(e) => setNewTime(e.target.value)}
              className="flex-1 px-3 py-2 border rounded-lg"
            />
            <button
              onClick={addFeedTime}
              className="px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600"
            >
              Tambah
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}
