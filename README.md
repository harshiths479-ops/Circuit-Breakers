# Circuit-Breakers
Wearable Health Monitoring Device
<br>
Team Number-49
<br>
Team Members-Harshith S
             Girish T S
             Gomini L
             Impana V
 
Wearable Health Monitoring & Emergency Alert System

 Overview
This project is a wearable IoT-based health monitoring device designed to detect abnormal heart rate, SpO₂ levels, and sudden falls.
In case of emergency, the system automatically sends an alert message along with GPS location using GSM.
The goal is to reduce emergency response time and improve patient safety.

Components Used
ESP32 / Arduino
MAX30102 (Heart Rate & SpO₂ Sensor)
MPU6050 (Fall Detection)
GSM Module (SIM800L)
GPS Module (Neo-6M)
0.96" OLED Display
Buzzer
Rechargeable Battery

Prototype Images & Explanation

 1️⃣ Complete Prototype Setup
(Insert image of full assembled device)
Description: This image shows the fully assembled wearable health monitoring system.
All sensors and communication modules are connected to the microcontroller.
The system continuously monitors vital parameters and displays real-time data on the OLED screen.

2️⃣ Heart Rate & SpO₂ Sensor (MAX30102)
(Insert close-up image of MAX30102)
Description: The MAX30102 sensor is used to measure heart rate and blood oxygen levels (SpO₂).
It works using photoplethysmography (PPG) technology.
The sensor sends real-time physiological data to the microcontroller for processing.

3️⃣ Fall Detection Sensor (MPU6050)
(Insert MPU6050 image)
Description: The MPU6050 accelerometer and gyroscope module detects sudden changes in motion.
If a fall is detected based on acceleration thresholds, the system triggers an emergency alert.

4️⃣ GSM & GPS Modules
(Insert image of GSM + GPS modules)
Description: The GSM module is used to send SMS alerts to emergency contacts.
The GPS module retrieves real-time location coordinates, which are included in the alert message.

5️⃣ OLED Display Output
(Insert OLED showing heart rate)
Description: The 0.96-inch OLED display shows live heart rate and SpO₂ readings.
It also displays emergency status notifications when abnormal conditions are detected.

Working Principle
Sensors collect physiological and motion data.
The microcontroller analyzes the readings.
If abnormal heart rate or fall is detected:
Buzzer alert is activated
SMS is sent via GSM
GPS location is shared
Real-time data is displayed on OLED

Features
Real-time health monitoring
Automatic fall detection
GSM-based emergency alerts
GPS location sharing
Portable and low-cost design
