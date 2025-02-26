🔍 Overview

The Rescue Robot is designed to assist in search and rescue operations by navigating hazardous environments and detecting obstacles, victims, or dangerous conditions. It uses Arduino for control, motor drivers for movement, and various sensors for navigation and detection.


🛠 Features

Obstacle Avoidance – Uses ultrasonic sensors to detect and avoid obstacles.

Autonomous Navigation – Moves independently with path-planning algorithms.

Remote Control Mode – Can be manually controlled via a wireless module.

Real-time Data Transmission – Sends live sensor data to a remote system.


🏗 Components Used

Microcontroller: Arduino Uno / Mega

Motor Driver: L298N or similar

Motors: DC motors with encoders

Sensors:

PIR sensor 

Ultrasonic (HC-SR04) for obstacle detection

Gas / Smoke sensor (MQ series) for hazardous environment detection

Communication: Bluetooth / Wi-Fi / RF module

Power: Li-ion battery pack


🔧 How It Works

1. The robot continuously scans its surroundings using ultrasonic sensors.

2. If an obstacle is detected, it calculates an alternate path.

3. If a victim is detected (via PIR/Thermal sensor), it alerts the operator.

4. The data is transmitted to a remote monitoring system in real-time.

5. The robot can be switched between autonomous and manual control modes.
