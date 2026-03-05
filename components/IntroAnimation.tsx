"use client";

import { useState, useEffect } from "react";
import { Wifi, CheckCircle, Zap } from "lucide-react";

interface Props {
  onComplete: () => void;
}

export default function IntroAnimation({ onComplete }: Props) {
  const [step, setStep] = useState(0);
  const [progress, setProgress] = useState(0);

  useEffect(() => {
    // Progress animation
    const progressInterval = setInterval(() => {
      setProgress(prev => {
        if (prev >= 100) {
          clearInterval(progressInterval);
          return 100;
        }
        return prev + 2;
      });
    }, 50);

    // Step transitions
    const stepTimers = [
      setTimeout(() => setStep(1), 1000),
      setTimeout(() => setStep(2), 2000),
      setTimeout(() => setStep(3), 3000),
      setTimeout(() => setStep(4), 4500),
      setTimeout(() => {
        onComplete();
      }, 6500),
    ];

    return () => {
      clearInterval(progressInterval);
      stepTimers.forEach(timer => clearTimeout(timer));
    };
  }, [onComplete]);

  return (
    <div className="fixed inset-0 bg-gradient-to-br from-indigo-900 via-purple-900 to-blue-900 z-50 flex items-center justify-center">
      {/* Animated background particles */}
      <div className="absolute inset-0 overflow-hidden">
        {[...Array(50)].map((_, i) => {
          const size = Math.random() * 3 + 1;
          const duration = Math.random() * 3 + 2;
          const delay = Math.random() * 2;
          
          return (
            <div
              key={i}
              className="absolute bg-white rounded-full animate-float"
              style={{
                width: `${size}px`,
                height: `${size}px`,
                left: `${(i * 7) % 100}%`,
                top: `${(i * 13) % 100}%`,
                opacity: Math.random() * 0.3 + 0.1,
                animationDuration: `${duration}s`,
                animationDelay: `${delay}s`,
              }}
            />
          );
        })}
      </div>

      <div className="relative z-10 text-center px-8 max-w-2xl">
        {/* Logo/Icon */}
        <div className="mb-8 flex justify-center">
          <div className="relative">
            <div className="w-32 h-32 bg-gradient-to-br from-blue-400 to-purple-500 rounded-3xl flex items-center justify-center shadow-2xl animate-bounce">
              <span className="text-6xl">🏠</span>
            </div>
            <div className="absolute -top-2 -right-2 w-8 h-8 bg-green-500 rounded-full flex items-center justify-center animate-ping">
              <Zap className="w-4 h-4 text-white" />
            </div>
          </div>
        </div>

        {/* Title */}
        <h1 className="text-5xl font-bold text-white mb-4 animate-fade-in">
          Smart Home IoT
        </h1>

        {/* Subtitle with typing effect */}
        <div className="text-xl text-blue-200 mb-8 h-8">
          {step >= 4 && (
            <div className="animate-fade-in">
              BY XI SIJA 1 SMK NEGERI 7 SEMARANG
            </div>
          )}
        </div>

        {/* Loading Steps */}
        <div className="space-y-4 mb-8">
          <div className={`flex items-center justify-center gap-3 transition-all duration-500 ${step >= 1 ? 'opacity-100 translate-y-0' : 'opacity-0 translate-y-4'}`}>
            <Wifi className="w-5 h-5 text-blue-400" />
            <span className="text-white">Connecting to MQTT Broker...</span>
            {step >= 2 && <CheckCircle className="w-5 h-5 text-green-400" />}
          </div>

          <div className={`flex items-center justify-center gap-3 transition-all duration-500 ${step >= 2 ? 'opacity-100 translate-y-0' : 'opacity-0 translate-y-4'}`}>
            <Zap className="w-5 h-5 text-yellow-400" />
            <span className="text-white">Initializing ESP32 Devices...</span>
            {step >= 3 && <CheckCircle className="w-5 h-5 text-green-400" />}
          </div>

          <div className={`flex items-center justify-center gap-3 transition-all duration-500 ${step >= 3 ? 'opacity-100 translate-y-0' : 'opacity-0 translate-y-4'}`}>
            <CheckCircle className="w-5 h-5 text-purple-400" />
            <span className="text-white">Loading Dashboard...</span>
            {step >= 4 && <CheckCircle className="w-5 h-5 text-green-400" />}
          </div>
        </div>

        {/* Progress Bar */}
        <div className="w-full bg-gray-700 rounded-full h-3 overflow-hidden shadow-inner">
          <div
            className="h-full bg-gradient-to-r from-blue-500 via-purple-500 to-pink-500 transition-all duration-300 ease-out"
            style={{ width: `${progress}%` }}
          >
            <div className="h-full w-full bg-white opacity-30 animate-pulse"></div>
          </div>
        </div>

        <div className="mt-4 text-blue-300 text-sm">
          {progress}% Complete
        </div>
      </div>
    </div>
  );
}
