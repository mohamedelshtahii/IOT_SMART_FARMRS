🌾 Smart Farming System (AIoT)

<img width="1429" height="803" alt="Screenshot 2026-03-22 054927" src="https://github.com/user-attachments/assets/90852f1c-4834-4de6-aaaf-e03b6350dea1" />


ESP32 • Sensors • Firmware • Cloud • Dashboard
A full end‑to‑end AIoT Smart Farming System designed to automate irrigation, monitor environmental conditions, and provide a real‑time interactive web dashboard for farm management.
The system integrates Hardware, Firmware, Software, and Cloud layers to deliver a scalable, efficient and intelligent smart‑agriculture solution.

🚀 Project Overview
This Smart‑Farming solution transforms a traditional farm into a fully automated environment through:

Real‑time environmental sensing
Intelligent irrigation logic
On‑device safety rules
Cloud connectivity (MQTT / HTTP)
Interactive live dashboard
Alerts + buzzer + LED indicators
Data logging, charts, and remote monitoring

Designed for production‑grade AIoT showcases, judge presentations, and portfolio demonstration.

🧱 System Architecture (Full AIoT Stack)
 ┌─────────────────────────────┐
 │         Cloud Layer         │
 │  MQTT / HTTP API / Blynk    │
 │  Real-time Telemetry        │
 │  Remote Monitoring          │
 └──────────────┬──────────────┘
                │
 ┌──────────────┴──────────────┐
 │       Web Dashboard (UI)     │
 │  Live KPIs, Charts, Alerts   │
 │  Map (Monufia), CSV Export   │
 │  Presenter Tools (hidden)    │
 └──────────────┬──────────────┘
                │
 ┌──────────────┴──────────────┐
 │        Firmware Layer        │
 │  ESP32 Logic:                │
 │  • Irrigation Automation     │
 │  • pH Monitoring             │
 │  • Moisture Logic (30/50%)   │
 │  • Tank Level Logic (<25%)   │
 │  • DHT22 Environment Sensor  │
 │  • Alerts & Safety Rules     │
 └──────────────┬──────────────┘
                │
 ┌──────────────┴──────────────┐
 │       Hardware Layer         │
 │ Sensors + Actuators:         │
 │ • Soil Moisture              │
 │ • Water Level                │
 │ • pH Sensor Module           │
 │ • DHT22 Temp & Humidity      │
 │ • LCD 20x4 I2C               │
 │ • LEDs (Green/Red/Blue)      │
 │ • Buzzer                     │
 │ • Relay / Valve Control      │
 │ • ESP32 Dev Board            │
 └─────────────────────────────┘

<img width="406" height="271" alt="env-ph" src="https://github.com/user-attachments/assets/d795dc43-1a2b-42c8-ab77-f06d2a2d0e8e" />

<img width="584" height="389" alt="soil-tank (1)" src="https://github.com/user-attachments/assets/df96cdae-5a64-4ed9-8378-b30f7c515ebc" />


🔌 Hardware Layer
Components Used

ESP32 (DevKit v1)
Soil Moisture Sensor
Water Level Sensor
DHT22 Temperature & Humidity
Soil pH Sensor Module
Relay Module + Solenoid Valve
20×4 LCD (I2C)
Buzzer
3 LEDs (Green, Red, Blue)

Functions

Reads all sensors every 1s
Controls irrigation based on moisture
Handles buzzer alerts
Displays live readings on LCD
Sends readings to dashboard/cloud


⚙️ Firmware Layer (Logic Summary)

<img width="1455" height="816" alt="Screenshot 2026-03-22 054822" src="https://github.com/user-attachments/assets/1bb4001b-08a1-4d6f-b8db-c43a966dd273" />

No firmware code placed here — but logic fully described.

Irrigation Logic

If moisture < 30% → Irrigation ON
If moisture ≥ 50% → Irrigation OFF
Supports Manual Control: Open, Close, Pulse 10s
Measures Flow L/min and Water Used per Day

Alerts
Triggered when:

Soil moisture < 25%
Water tank < 25%
Soil pH < 5.8 or > 7.8

Indicators

Green LED → System OK
Red LED → Soil needs watering
Blue LED → Tank Low
Buzzer → Plays on any alert

LCD Display

20×4 shows:

Moisture
pH
Temperature
Humidity
Tank Level
Irrigation State




☁️ Cloud Layer (Optional Integration)
The system is designed to easily support:
MQTT

Publish telemetry
Subscribe to irrigation commands
Supports QoS0/QoS1 workflows

HTTP API

Push sensor data
Receive control commands
Ready for Node‑RED / Flask / FastAPI gateway

Blynk IoT

Mobile app visualization
Virtual pins
Real‑time notifications


Switching from local simulation to real cloud only requires changing DATA_MODE to http or ws.


🖥️ Software Layer — Live Dashboard (Frontend)
A modern, production‑grade UI built using only HTML + CSS + JS:
Features

Real‑time KPIs
5 interactive Gauges
Sparkline micro‑charts
2 large zoomable Charts
Threshold lines
Alerts panel
Event Log
Fullscreen Mode
CSV Export
Map (Monufia, Egypt)
Random farm positioning
Hidden Presenter Mode (Shift + P)
Scenario Simulator
Speed & Noise Controls
Mobile responsive

Farm Map

Default location: Shibin el‑Kom, Monufia
Highlight marker on irrigation
Buttons:

Recenter
Randomize Farm (inside Monufia area)




📂 File Structure
.
├── index.html    # Interactive dashboard (all-in-one)
└── README.md     # Project documentation


🚀 How to Run

Download index.html
Open it in Chrome / Edge / Firefox
That’s it — no server needed

For deployment:

GitHub Pages
Netlify
Vercel
Any static hosting

