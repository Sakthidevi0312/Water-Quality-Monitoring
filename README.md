# Water-Quality-Monitoring

This project is an IoT-based water monitoring system using ESP32, MQTT, Python, and MySQL.

## Features

- Real-time water quality monitoring
- MQTT communication using ESP32
- MySQL database logging
- Multi-sensor support

## Sensors Used

- pH Sensor
- TDS Sensor
- Turbidity Sensor
- Water Level Sensor
- Flow Sensor

## Technologies

- ESP32
- Arduino IDE
- MQTT Protocol
- Python
- MySQL

## MQTT Topics

- watermonitor/ph
- watermonitor/flow
- watermonitor/level
- watermonitor/tds
- watermonitor/turbidity

## Files

### WMSall.ino
ESP32 Arduino source code for sensor reading and MQTT publishing.

### ex4all.py
Python MQTT subscriber that stores sensor data into MySQL database.

## Database Tables

- ph_data
- flow_data
- level_data
- tds_data
- turbidity_data

## Future Improvements

- Web dashboard
- Mobile app
- Email/SMS alerts
- Cloud integration
- AI-based water quality prediction

## Author

Prabhu
