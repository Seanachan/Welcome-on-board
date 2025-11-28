# Welcome-on-board

We're implementing a automatic car combining arduino toolkit and PIC XC8.

Integrated with EPS32-WROVER, which serves as a bi-directional bridge between a smartphone (via Web Bluetooth/BLE) and the target **PIC18F4520** microcontroller (via UART).

## Web App

1. **Start listening** - When you tap the mic button, it calls `recognition.start()` using `window.SpeechRecognition` or `window.webkitSpeechRecognition`
2. **Browser handles everything** - The browser listens to your microphone and converts speech to text in real-time
3. **Get the result** - When you stop speaking, the `onresult` event fires with the transcribed text
4. **Send to LLM** - The text is sent to the AI to convert natural language (like "turn left") into a command (`TURN_LEFT`)
5. **Send via Bluetooth** - The command is sent to your ESP32 over Bluetooth

## Voice Commands

After connect your phone to ESP32 via [Our Web App](https://voice-command-hub-copy-c1d22b6b.base44.app/), speak to your phone to manipulate the car. Here are the commands that you can use:

> Chrome and Edge are recommended to use.

**Movement:**

* "Go forward" → `FORWARD`
* "Go back" → `REVERSE`
* "Stop" → `STOP`

**Steering:**

* "Turn left" → `TURN_LEFT`
* "Turn right" → `TURN_RIGHT`
* "Go straight" → `STRAIGHT`

**Gears:**

* "First gear" → `GEAR_ONE`
* "Second gear" → `GEAR_TWO`
* "Third gear" → `GEAR_THREE`
* "Neutral" → `GEAR_NEUTRAL`

**Speed:**

* "Speed up" → `ACCELERATE`
* "Slow down" → `DECELERATE`
* "Set speed to 50" → `SPEED:50`

**Lights:**

* "Turn on lights" → `LIGHTS_ON`
* "Turn off lights" → `LIGHTS_OFF`
* "Headlights on" → `HEADLIGHTS_ON`

**Other:**

* "Honk" → `HORN`
* "Emergency stop" → `EMERGENCY_STOP`
