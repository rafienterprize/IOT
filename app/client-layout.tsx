"use client";

import { useState, useEffect } from "react";
import Sidebar from "@/components/Sidebar";
import IntroAnimation from "@/components/IntroAnimation";
import { useMQTT } from "@/hooks/useMQTT";

export default function ClientLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  const { isConnected } = useMQTT();
  const [showIntro, setShowIntro] = useState(true);
  const [mounted, setMounted] = useState(false);

  useEffect(() => {
    setMounted(true);
    
    // Check if intro has been shown before in this session
    const hasSeenIntro = sessionStorage.getItem('iot-dashboard-intro-seen');
    if (hasSeenIntro === 'true') {
      setShowIntro(false);
    }
  }, []);

  if (!mounted) {
    return null;
  }

  if (showIntro) {
    return <IntroAnimation onComplete={() => {
      setShowIntro(false);
      sessionStorage.setItem('iot-dashboard-intro-seen', 'true');
    }} />;
  }

  return (
    <div className="flex h-screen">
      <Sidebar isConnected={isConnected} />
      <main className="flex-1 overflow-y-auto">{children}</main>
    </div>
  );
}