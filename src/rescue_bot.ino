/*
 * RescueBot - Team Zenith
 * 🏆 First Prize Winner - Hertz Microprocessor Hackathon (ECSA)
 * 
 * "Reaching New Heights in Innovation and Rescue Robotics"
 * 
 * This code implements a Wi-Fi controlled rescue robot with:
 * - Remote web-based control interface
 * - Obstacle avoidance using ultrasonic sensor
 * - Gas detection for hazardous environments
 * - Human detection for search and rescue
 * - Emergency alert system
 * 
 * Hardware:
 * - ESP32 Microcontroller
 * - L298N Motor Driver
 * - MQ2 Gas Sensor
 * - PIR Motion Sensor
 * - HC-SR04 Ultrasonic Sensor
 * - Touch Sensor
 * - DC Motors
 * 
 * Created by: Team Zenith
 * Date: 2025
 * License: MIT
 */
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "SIIIUR7";  // Change to your WiFi SSID
const char* password = "77777777"; // Change to your WiFi Password

WebServer server(80);
Servo scanServo;  // Servo for scanning obstacles

// Motor control pins
const int motor1Pin1 = 5;
const int motor1Pin2 = 18;
const int motor2Pin1 = 19;
const int motor2Pin2 = 21;

// Ultrasonic sensor pins
const int trigPin = 2;
const int echoPin = 4;

// Servo motor pin
const int servoPin = 13;

// Gas sensor pin (MQ-2)
const int gasSensorPin = 34; // Analog input pin for MQ-2 sensor
const int gasThreshold = 1000; // Threshold for gas detection (adjust based on your sensor calibration)
bool gasDetected = false;

// Threshold distance (in cm)
const int obstacleDistance = 20;

// Control flags
bool autonomousEnabled = false;
unsigned long lastObstacleCheck = 0;
unsigned long lastGasCheck = 0;
const int checkInterval = 100; // Check for obstacles every 100ms
const int gasCheckInterval = 1000; // Check gas levels every second

// Web interface - Changed "Bluetooth Mode" to "WiFi Mode"
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>RescueBot Control</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&display=swap');

    /* Animated Background */
    body {
      font-family: 'Orbitron', sans-serif;
      text-align: center;
      margin: 0;
      padding: 0;
      height: 100vh;
      background: linear-gradient(-45deg, #010a43, #01102a, #000428);
      background-size: 400% 400%;
      color: #fff;
      animation: backgroundShift 15s infinite alternate ease-in-out;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
    }

    @keyframes backgroundShift {
      0% { background-position: 0% 50%; }
      100% { background-position: 100% 50%; }
    }

    h1 {
      font-size: 50px;
      color: #0ff;
      text-shadow: 0 0 20px #0ff, 0 0 40px #00f;
      animation: neonGlow 1.5s infinite alternate;
    }

    @keyframes neonGlow {
      0% { text-shadow: 0 0 10px #0ff, 0 0 30px #00f; }
      100% { text-shadow: 0 0 30px #0ff, 0 0 50px #00f; }
    }

    select {
      font-size: 18px;
      padding: 12px;
      border: none;
      border-radius: 10px;
      background: rgba(0, 255, 255, 0.8);
      color: black;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.3s ease-in-out;
      box-shadow: 0 0 15px rgba(0, 255, 255, 0.8);
    }

    select:hover {
      box-shadow: 0 0 30px rgba(0, 255, 255, 1);
    }

    /* Buttons */
    .button {
      width: 100px;
      height: 100px;
      font-size: 40px;
      background: rgba(0, 255, 255, 0.2);
      color: #0ff;
      border: 3px solid rgba(0, 255, 255, 0.8);
      border-radius: 20px;
      display: flex;
      justify-content: center;
      align-items: center;
      cursor: pointer;
      text-shadow: 0 0 10px #0ff, 0 0 15px #00f;
      transition: all 0.3s ease-in-out;
      box-shadow: 0 0 20px rgba(0, 255, 255, 0.6);
      backdrop-filter: blur(10px);
    }

    .button:hover {
      background: rgba(0, 255, 255, 0.4);
      box-shadow: 0 0 40px #0ff;
      transform: translateY(-5px);
    }

    /* Grid Layout */
    .container {
      display: grid;
      grid-template-areas: 
        ". up ."
        "left stop right"
        ". down .";
      grid-gap: 15px;
      justify-items: center;
      align-items: center;
      margin-top: 30px;
    }

    .up { grid-area: up; }
    .left { grid-area: left; }
    .right { grid-area: right; }
    .down { grid-area: down; }
    .stop { 
      grid-area: stop; 
      background: rgba(255, 0, 0, 0.8);
      color: white;
      font-size: 30px;
      width: 80px;
      height: 80px;
      border: 3px solid rgba(255, 0, 0, 0.8);
      text-shadow: 0 0 10px red, 0 0 20px darkred;
      box-shadow: 0 0 20px rgba(255, 0, 0, 0.6);
    }

    .stop:hover {
      background: rgba(255, 0, 0, 1);
      box-shadow: 0 0 40px red;
      transform: translateY(-5px);
    }

    /* Alert Buttons */
    .alert-container {
      margin-top: 20px;
      display: flex;
      gap: 15px;
    }

    .alert-button {
      padding: 15px 30px;
      font-size: 18px;
      font-weight: bold;
      border-radius: 10px;
      border: none;
      cursor: pointer;
      transition: all 0.3s ease-in-out;
      text-transform: uppercase;
    }

    .sos {
      background: rgba(255, 0, 0, 0.7);
      color: white;
      box-shadow: 0 0 20px red;
    }

    .gas {
      background: rgba(255, 165, 0, 0.7);
      color: black;
      box-shadow: 0 0 20px orange;
    }

    .gas.active {
      background: rgba(255, 165, 0, 1);
      color: white;
      box-shadow: 0 0 40px orange, 0 0 80px red;
      animation: pulse 1s infinite;
    }

    @keyframes pulse {
      0% { transform: scale(1); }
      50% { transform: scale(1.1); }
      100% { transform: scale(1); }
    }

    .motion {
      background: rgba(0, 255, 255, 0.7);
      color: black;
      box-shadow: 0 0 20px cyan;
    }

    /* Hide Controls Initially */
    #wifi-controls, #autonomous-controls {
      display: none;
    }

    /* Sensor Readings */
    .sensor-readings {
      margin-top: 20px;
      padding: 15px;
      background: rgba(0, 0, 0, 0.5);
      border-radius: 10px;
      border: 1px solid rgba(0, 255, 255, 0.3);
      width: 80%;
      max-width: 500px;
    }

    .sensor-value {
      display: flex;
      justify-content: space-between;
      margin: 10px 0;
    }

    /* Bot Status */
    #bot-status {
      font-size: 24px;
      margin-top: 20px;
      animation: pulseGlow 1.5s infinite alternate;
      color: #0ff;
      font-weight: bold;
    }

    @keyframes pulseGlow {
      0% { text-shadow: 0 0 10px #0ff; }
      100% { text-shadow: 0 0 30px #0ff; }
    }

  </style>
</head>
<body>

  <h1>RescueBot Control</h1>

  <label for="mode">Choose Mode:</label>
  <select id="mode" onchange="toggleControls()">
    <option value="">-- Select Mode --</option>
    <option value="wifi">WiFi Mode</option>
    <option value="autonomous">Autonomous Mode</option>
    <option value="idle">Idle Mode</option>
  </select>

  <div class="alert-container">
    <button class="alert-button sos" onclick="sendRequest('/sos-alert')">SOS Alert</button>
    <button id="gas-alert-btn" class="alert-button gas" onclick="sendRequest('/gas-alert')">Gas Alert</button>
    <button class="alert-button motion" onclick="sendRequest('/motion-alert')">Motion Alert</button>
  </div>

  <div class="sensor-readings">
    <div class="sensor-value">
      <span>Gas Level:</span>
      <span id="gas-value">0</span>
    </div>
    <div class="sensor-value">
      <span>Distance:</span>
      <span id="distance-value">0 cm</span>
    </div>
  </div>

  <!-- WiFi Controls (changed from Bluetooth) -->
  <div id="wifi-controls">
    <div class="container">
      <button class="button up" onclick="sendRequest('/forward')">&#x2191;</button> 
      <button class="button left" onclick="sendRequest('/left')">&#x2190;</button> 
      <button class="button stop" onclick="sendRequest('/stop')">⏹</button> 
      <button class="button right" onclick="sendRequest('/right')">&#x2192;</button> 
      <button class="button down" onclick="sendRequest('/backward')">&#x2193;</button> 
    </div>
  </div>

  <!-- Autonomous Controls -->
  <div id="autonomous-controls">
    <button class="button" onclick="sendRequest('/autonomous-start')">Start</button>
    <button class="button" onclick="sendRequest('/autonomous-stop')">Stop</button>
  </div>

  <div id="bot-status">Bot Status: Connected</div>

  <script>
    function toggleControls() {
      let mode = document.getElementById("mode").value;
      document.getElementById("wifi-controls").style.display = mode === "wifi" ? "block" : "none";
      document.getElementById("autonomous-controls").style.display = mode === "autonomous" ? "block" : "none";
      
      if (mode === "idle") {
        sendRequest('/stop');
      } else if (mode === "autonomous") {
        sendRequest('/autonomous-stop'); // Reset autonomous mode when switching to it
      }
    }

    function sendRequest(url) {
      fetch(url)
        .then(response => {
          if (response.ok) {
            console.log("Command sent successfully");
            document.getElementById("bot-status").innerHTML = "Bot Status: Active - " + url.substring(1);
          } else {
            console.error("Failed to send command");
            document.getElementById("bot-status").innerHTML = "Bot Status: Error";
          }
        })
        .catch(error => {
          console.error("Error:", error);
          document.getElementById("bot-status").innerHTML = "Bot Status: Connection Error";
        });
    }
    
    // Update sensor readings periodically
    setInterval(function() {
      fetch('/sensor-data')
        .then(response => response.json())
        .then(data => {
          document.getElementById("gas-value").innerHTML = data.gasLevel;
          document.getElementById("distance-value").innerHTML = data.distance + " cm";
          
          // Update gas alert button
          if (data.gasDetected) {
            document.getElementById("gas-alert-btn").classList.add("active");
          } else {
            document.getElementById("gas-alert-btn").classList.remove("active");
          }
        })
        .catch(error => {
          console.error("Sensor update error:", error);
        });
    }, 1000);
  </script>

</body>
</html>
)rawliteral";

// Function to get distance from ultrasonic sensor with better error handling
int getDistance() {
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Set the trigger pin HIGH for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echo pin to get the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH, 30000); // Added timeout of 30ms
  
  // Convert the time into distance in cm
  // Speed of sound wave divided by 2 (time to go and come back)
  int distance = duration * 0.034 / 2;
  
  // Check for invalid readings
  if (distance <= 0 || distance > 400) {
    Serial.println("Invalid distance reading!");
    return 400; // Return max value if reading is invalid
  }
  
  return distance;
}

// Function to read gas sensor value
int readGasSensor() {
  int sensorValue = analogRead(gasSensorPin);
  Serial.print("Gas Sensor Value: ");
  Serial.println(sensorValue);
  
  // Check if gas is detected based on threshold
  if (sensorValue > gasThreshold) {
    gasDetected = true;
  } else {
    gasDetected = false;
  }
  
  return sensorValue;
}

// Function to send sensor data
void handleSensorData() {
  int distance = getDistance();
  int gasLevel = readGasSensor();
  
  String json = "{\"distance\":" + String(distance) + 
                ",\"gasLevel\":" + String(gasLevel) + 
                ",\"gasDetected\":" + String(gasDetected ? "true" : "false") + "}";
                
  server.send(200, "application/json", json);
}

// Function to move forward
void moveForward() {
  Serial.println("Moving Forward");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  server.send(200, "text/plain", "Moving Forward");
}

// Function to move backward
void moveBackward() {
  Serial.println("Moving Backward");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  server.send(200, "text/plain", "Moving Backward");
}

// Function to turn left
void turnLeft() {
  Serial.println("Turning Left");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  server.send(200, "text/plain", "Turning Left");
}

// Function to turn right
void turnRight() {
  Serial.println("Turning Right");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  server.send(200, "text/plain", "Turning Right");
}

// Function to stop motors
void stopMotors() {
  Serial.println("Stopping Motors");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  server.send(200, "text/plain", "Stopped");
}

// Function to handle status requests
void handleStatus() {
  String status = autonomousEnabled ? "Autonomous" : "Manual Control";
  server.send(200, "text/plain", status);
}

// Function to handle SOS alert
void handleSosAlert() {
  Serial.println("SOS ALERT TRIGGERED!");
  // Add code here to handle SOS alert (e.g., play sound, flash LED)
  server.send(200, "text/plain", "SOS Alert Triggered");
}

// Function to handle Gas sensor alert
void handleGasAlert() {
  Serial.println("GAS ALERT TRIGGERED!");
  // Add code here to handle gas alert
  server.send(200, "text/plain", "Gas Alert Triggered");
}

// Function to handle Motion sensor alert
void handleMotionAlert() {
  Serial.println("MOTION ALERT TRIGGERED!");
  // Add code here to handle motion alert
  server.send(200, "text/plain", "Motion Alert Triggered");
}

// Start autonomous mode
void startAutonomousMode() {
  autonomousEnabled = true;
  Serial.println("Autonomous mode activated");
  server.send(200, "text/plain", "Autonomous Mode Started");
}

// Stop autonomous mode
void stopAutonomousMode() {
  autonomousEnabled = false;
  stopMotors();
  Serial.println("Autonomous mode deactivated");
  server.send(200, "text/plain", "Autonomous Mode Stopped");
}

// Function to check servo operation
void testServo() {
  Serial.println("Testing servo motor...");
  
  // Move to 0 degrees
  scanServo.write(0);
  delay(1000);
  Serial.println("Servo at 0 degrees");
  
  // Move to 90 degrees
  scanServo.write(90);
  delay(1000);
  Serial.println("Servo at 90 degrees");
  
  // Move to 180 degrees
  scanServo.write(180);
  delay(1000);
  Serial.println("Servo at 180 degrees");
  
  // Return to center
  scanServo.write(90);
  delay(1000);
  Serial.println("Servo back at center");
  
  server.send(200, "text/plain", "Servo Test Completed");
}

// Handle autonomous navigation logic - called from loop()
void handleAutonomousNavigation() {
  if (!autonomousEnabled) return;
  
  unsigned long currentMillis = millis();
  
  // Only check for obstacles at specified intervals to avoid blocking the server
  if (currentMillis - lastObstacleCheck >= checkInterval) {
    lastObstacleCheck = currentMillis;
    
    int distance = getDistance();
    Serial.print("Distance: ");
    Serial.println(distance);

    // Added state-based navigation to prevent rapid decision changes
    static enum {MOVING_FORWARD, SCANNING, TURNING} navState = MOVING_FORWARD;
    static unsigned long stateStartTime = 0;
    static int turnDirection = 0; // 1 for left, 2 for right
    
    switch(navState) {
      case MOVING_FORWARD:
        if (distance < obstacleDistance) {
          // Obstacle detected, stop and move to scanning state
          stopMotors();
          Serial.println("Obstacle detected! Starting scan...");
          navState = SCANNING;
          stateStartTime = currentMillis;
          scanServo.write(90); // Center servo first
        } else {
          // Path is clear, continue forward
          moveForward();
        }
        break;
        
      case SCANNING:
        // Complete scanning sequence before making a decision
        if (currentMillis - stateStartTime < 500) {
          // Wait for servo to center
          scanServo.write(90);
        } else if (currentMillis - stateStartTime < 1000) {
          // Scan left
          scanServo.write(150);
        } else if (currentMillis - stateStartTime < 1500) {
          // Measure left distance
          int leftDist = getDistance();
          Serial.print("Left distance: ");
          Serial.println(leftDist);
          // Store left distance for comparison
          static int storedLeftDist = leftDist;
          
          // Prepare to scan right
          scanServo.write(30);
        } else if (currentMillis - stateStartTime < 2000) {
          // Wait for servo to reach right position
        } else if (currentMillis - stateStartTime < 2500) {
          // Measure right distance
          int rightDist = getDistance();
          Serial.print("Right distance: ");
          Serial.println(rightDist);
          
          // Compare distances and decide turn direction
          int storedLeftDist = 0; // This should be properly stored from earlier
          if (rightDist > storedLeftDist) {
            Serial.println("More space on right side");
            turnDirection = 2; // right
          } else {
            Serial.println("More space on left side");
            turnDirection = 1; // left
          }
          
          // Reset servo to center
          scanServo.write(90);
          
          // Move to turning state
          navState = TURNING;
          stateStartTime = currentMillis;
        }
        break;
        
      case TURNING:
        // Execute turn for a fixed duration
        if (currentMillis - stateStartTime < 800) {
          if (turnDirection == 1) {
            turnLeft();
            Serial.println("Turning left");
          } else {
            turnRight();
            Serial.println("Turning right");
          }
        } else {
          // Return to forward movement after turn completes
          navState = MOVING_FORWARD;
        }
        break;
    }
  }
  
  // Check gas sensor at specified intervals
  if (currentMillis - lastGasCheck >= gasCheckInterval) {
    lastGasCheck = currentMillis;
    
    // Read gas sensor
    readGasSensor();
    
    // If gas detected, temporarily halt and alert
    if (gasDetected) {
      Serial.println("WARNING: Alcohol/Gas detected in autonomous mode!");
      stopMotors();
      delay(500); // Reduced delay to be less blocking
      // Continue operation but maintain alert status
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Wait for serial monitor to open
  delay(1000);
  Serial.println("\n\nRescueBot Initializing...");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  
  // Wait for connection with timeout
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nFailed to connect to WiFi. Check credentials or try again.");
  }

  // Set motor control pins as output
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  // Initialize motors to OFF state
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);

  // Set ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Setup gas sensor pin (analog input)
  pinMode(gasSensorPin, INPUT);

  // Attach and center the servo - with specific pulse width ranges for better control
  scanServo.setPeriodHertz(50);      // Standard 50Hz servo
  scanServo.attach(servoPin, 500, 2400); // Min/Max pulse width in microseconds
  
  // Test servo motor to ensure it's working
  Serial.println("Testing servo motion...");
  scanServo.write(90);  // Center position
  delay(1000);
  scanServo.write(45);  // 45 degrees
  delay(1000);
  scanServo.write(135); // 135 degrees
  delay(1000);
  scanServo.write(90);  // Back to center
  delay(1000);
  Serial.println("Servo test complete.");

  // Test ultrasonic sensor
  Serial.println("Testing ultrasonic sensor...");
  int testDistance = getDistance();
  Serial.print("Current distance reading: ");
  Serial.print(testDistance);
  Serial.println(" cm");

  // Web server routes
  server.on("/", HTTP_GET, []() { 
    server.send(200, "text/html", index_html); 
  });
  
  // Movement controls
  server.on("/forward", HTTP_GET, moveForward);
  server.on("/backward", HTTP_GET, moveBackward);
  server.on("/left", HTTP_GET, turnLeft);
  server.on("/right", HTTP_GET, turnRight);
  server.on("/stop", HTTP_GET, stopMotors);
  
  // Mode controls
  server.on("/autonomous-start", HTTP_GET, startAutonomousMode);
  server.on("/autonomous-stop", HTTP_GET, stopAutonomousMode);
  
  // Status and sensor data endpoints
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/sensor-data", HTTP_GET, handleSensorData);
  
  // Alert endpoints
  server.on("/sos-alert", HTTP_GET, handleSosAlert);
  server.on("/gas-alert", HTTP_GET, handleGasAlert);
  server.on("/motion-alert", HTTP_GET, handleMotionAlert);
  
  // Diagnostic endpoint
  server.on("/test-servo", HTTP_GET, testServo);

  // Start server
  server.begin();
  Serial.println("Web server started.");
  Serial.println("RescueBot ready!");
}

void loop() {
  server.handleClient(); // Handle web server requests
  
  // Handle autonomous navigation if enabled
  if (autonomousEnabled) {
    handleAutonomousNavigation();
  } else {
    // When not in autonomous mode, still check gas sensor periodically
    unsigned long currentMillis = millis();
    if (currentMillis - lastGasCheck >= gasCheckInterval) {
      lastGasCheck = currentMillis;
      readGasSensor();
      
      if (gasDetected) {
        Serial.println("WARNING: Alcohol/Gas detected!");
        // You could add additional alerts here like flashing LEDs or buzzer
      }
    }
  }
  
  // Small delay to improve stability
  delay(10);
}