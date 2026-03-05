import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "IoT Monitoring Dashboard",
  description: "Smart Home IoT Control & Monitoring System",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="id">
      <body>{children}</body>
    </html>
  );
}
