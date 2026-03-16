"use client";

import "./globals.css";
import { useState, useEffect } from "react";
import Sidebar from "@/components/Sidebar";
import IntroAnimation from "@/components/IntroAnimation";
import { useMQTT } from "@/hooks/useMQTT";

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  const { isConnected } = useMQTT();
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

  return (
    <html lang="id">
      <head>
        <title>IoT Monitoring Dashboard</title>
        <meta name="description" content="Smart Home IoT Control & Monitoring System" />
      </head>
      <body className="bg-gradient-to-br from-slate-50 to-slate-100">
        <div className="flex h-screen">
          <Sidebar isConnected={isConnected} />
          <main className="flex-1 overflow-y-auto">{children}</main>
        </div>
      </body>
    </html>
  );
}
