# Mobile Client and Notification Setup

This guide explains how to build the mobile version of the client and configure push notifications. The project relies on React Native and the Expo tooling.

## Building the Mobile Client

1. Ensure you have **Node ≥20** and [Expo CLI](https://docs.expo.dev/get-started/installation/) installed globally:
   ```bash
   npm install -g expo-cli
   ```
2. Navigate to the client directory and install dependencies:
   ```bash
   cd client
   npm install
   ```
3. The repository includes a minimal Expo project under `client/mobile`. To start it run:
   ```bash
   cd client/mobile
   npm install
   npm start
   ```
4. Use the Expo Go application on your device or an emulator to run the project.

## Enabling Push Notifications

The mobile client can notify you of battles, training completions and marketplace events. The demo uses Expo's push service with Firebase Cloud Messaging (FCM).

1. Set up a Firebase project and download the `google-services.json` file for Android. Place the file under `mobile/android/app/`.
2. Enable push notifications in `app.json` by adding your Firebase `projectId`:
   ```json
   {
       "expo": {
           "android": {
               "googleServicesFile": "./google-services.json"
           }
       }
   }
   ```
3. Register for notifications in your React Native code and send the device token to your backend. The backend forwards events using Expo's push API.
4. For iOS, configure APNs certificates in the Apple Developer portal and run `expo build:ios` with the appropriate credentials.

When everything is configured you will receive push notifications whenever battles finish or your listings change in the marketplace.

---

© 2025 Cognithesis Labs – Draft
