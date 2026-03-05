"use client";

import { useEffect, useState, useCallback, useRef } from "react";
import mqtt, { MqttClient } from "mqtt";

export function useMQTT() {
  const [isConnected, setIsConnected] = useState(false);
  const clientRef = useRef<MqttClient | null>(null);
  const subscribersRef = useRef<Map<string, Set<(message: string) => void>>>(new Map());

  useEffect(() => {
    // Connect to MQTT broker
    const broker = process.env.NEXT_PUBLIC_MQTT_BROKER || "broker.hivemq.com";
    const port = process.env.NEXT_PUBLIC_MQTT_PORT || "8000";
    const url = `ws://${broker}:${port}/mqtt`;

    const client = mqtt.connect(url, {
      clientId: `iot_dashboard_${Math.random().toString(16).slice(3)}`,
      clean: true,
      connectTimeout: 4000,
      reconnectPeriod: 1000,
    });

    client.on("connect", () => {
      console.log("Connected to MQTT broker");
      setIsConnected(true);
    });

    client.on("disconnect", () => {
      console.log("Disconnected from MQTT broker");
      setIsConnected(false);
    });

    client.on("message", (topic, message) => {
      const msg = message.toString();
      const callbacks = subscribersRef.current.get(topic);
      if (callbacks) {
        callbacks.forEach(callback => callback(msg));
      }
    });

    clientRef.current = client;

    return () => {
      client.end();
    };
  }, []);

  const publish = useCallback((topic: string, message: string) => {
    if (clientRef.current && isConnected) {
      clientRef.current.publish(topic, message);
      console.log(`Published to ${topic}:`, message);
    }
  }, [isConnected]);

  const subscribe = useCallback((topic: string, callback: (message: string) => void) => {
    if (clientRef.current) {
      clientRef.current.subscribe(topic);
      
      if (!subscribersRef.current.has(topic)) {
        subscribersRef.current.set(topic, new Set());
      }
      subscribersRef.current.get(topic)?.add(callback);

      return () => {
        subscribersRef.current.get(topic)?.delete(callback);
        if (subscribersRef.current.get(topic)?.size === 0) {
          clientRef.current?.unsubscribe(topic);
          subscribersRef.current.delete(topic);
        }
      };
    }
  }, []);

  return { isConnected, publish, subscribe };
}
