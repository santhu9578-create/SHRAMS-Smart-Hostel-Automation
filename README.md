# SHRAMS – Smart Hostel Resource Automation and Monitoring System

## Project Overview
SHRAMS is an ESP32-based IoT project designed to automate hostel room resources and monitor environmental conditions using sensors, MQTT communication, and a dashboard-based monitoring system.

## Features
- **Occupancy-based lighting:** Automatically controls lighting based on room occupancy.
- **Fan automation:** Controls the fan output according to room occupancy.
- **Environmental monitoring:** Monitors temperature, humidity, and gas levels.
- **Door monitoring:** Uses distance sensing to detect door-related conditions.
- **Safety alerts:** Identifies configured temperature and gas threshold conditions and activates alerts.
- **IoT monitoring:** Uses MQTT to communicate sensor readings and system status.

## Hardware Components
- ESP32 development board
- DHT22 temperature and humidity sensor
- MQ-2 gas sensor
- Ultrasonic distance sensors
- Servo motor
- LEDs and buzzer

## Software and Technologies
- Arduino / Embedded C++
- Wokwi simulation
- MQTT
- Node-RED
- InfluxDB
- Grafana

## System Workflow
1. The ESP32 reads sensor values and occupancy-related inputs.
2. The controller applies the configured automation and alert conditions.
3. System data and status messages are published through MQTT.
4. Node-RED processes the messages for monitoring and automation workflows.
5. InfluxDB and Grafana can be used for data storage and visualization when the corresponding services are configured.

## Simulation
The circuit and ESP32 program are developed for simulation using Wokwi.

Wokwi project: https://wokwi.com/projects/467855989331933185

## Repository Contents
- `sketch.ino` – ESP32 source code
- `diagram.json` – Wokwi circuit configuration
- `libraries.txt` – Simulation library dependencies, if applicable

## Future Improvements
- Improve monitoring and alert handling.
- Add additional room automation features.
- Expand dashboard visualization and system reporting.

## Author
R. Santhosh  
B.E. Electronics and Communication Engineering Student
