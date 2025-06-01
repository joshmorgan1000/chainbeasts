import React, { useEffect, useState } from 'react';
import { Text, View } from 'react-native';
import * as Notifications from 'expo-notifications';

export default function App() {
  const [token, setToken] = useState<string | undefined>();

  useEffect(() => {
    Notifications.requestPermissionsAsync().then(({ granted }) => {
      if (granted) {
        Notifications.getExpoPushTokenAsync().then(t => setToken(t.data));
      }
    });

    const wsBattle = new WebSocket('ws://localhost:8767/battles');
    wsBattle.onmessage = msg => {
      Notifications.scheduleNotificationAsync({
        content: { title: 'Battle Update', body: msg.data },
        trigger: null,
      });
    };

    const wsMarket = new WebSocket('ws://localhost:8768/market');
    wsMarket.onmessage = msg => {
      Notifications.scheduleNotificationAsync({
        content: { title: 'Marketplace Update', body: msg.data },
        trigger: null,
      });
    };

    return () => {
      wsBattle.close();
      wsMarket.close();
    };
  }, []);

  return (
    <View style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
      <Text>{token ? `Push token: ${token}` : 'Requesting permission...'}</Text>
    </View>
  );
}
