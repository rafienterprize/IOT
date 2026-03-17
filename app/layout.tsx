import "./globals.css";
import type { Metadata } from 'next';
import ClientLayout from './client-layout';

export const metadata: Metadata = {
  title: 'IoT Monitoring Dashboard',
  description: 'Smart Home IoT Control & Monitoring System',
  viewport: 'width=device-width, initial-scale=1',
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="id">
      <body className="bg-gradient-to-br from-slate-50 to-slate-100">
        <ClientLayout>{children}</ClientLayout>
      </body>
    </html>
  );
}
