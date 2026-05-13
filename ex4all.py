import paho.mqtt.client as mqtt
import mysql.connector
from datetime import datetime

# --- Database Configuration ---
DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',
    'password': '',
    'database': 'wmstds'
}

# --- MQTT Configuration ---
MQTT_BROKER = "broker.emqx.io"
MQTT_PORT = 1883
MQTT_TOPICS = [
    ("watermonitor/ph", 0),
    ("watermonitor/flow", 0),
    ("watermonitor/level", 0),
    ("watermonitor/tds", 0),
    ("watermonitor/turbidity", 0)  # ✅ New Turbidity topic
]

# --- Connect to MySQL and create tables if not exist ---
def create_tables():
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()

        cursor.execute("""
            CREATE TABLE IF NOT EXISTS ph_data (
                id INT AUTO_INCREMENT PRIMARY KEY,
                value FLOAT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS flow_data (
                id INT AUTO_INCREMENT PRIMARY KEY,
                value FLOAT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS level_data (
                id INT AUTO_INCREMENT PRIMARY KEY,
                value FLOAT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS tds_data (
                id INT AUTO_INCREMENT PRIMARY KEY,
                value FLOAT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS turbidity_data (
                id INT AUTO_INCREMENT PRIMARY KEY,
                value FLOAT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)

        conn.commit()
        conn.close()
        print("MySQL tables created or already exist.")
    except Exception as e:
        print("Database error:", e)

# --- Save data into respective table ---
def save_to_db(sensor, value):
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()

        table_name = {
            "ph": "ph_data",
            "flow": "flow_data",
            "level": "level_data",
            "tds": "tds_data",
            "turbidity": "turbidity_data"  # ✅ Added turbidity table mapping
        }.get(sensor)

        if table_name:
            cursor.execute(f"INSERT INTO {table_name} (value) VALUES (%s)", (value,))
            conn.commit()

        conn.close()
    except Exception as e:
        print("Error saving data:", e)

# --- MQTT Callback: on_connect ---
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker")
        client.subscribe(MQTT_TOPICS)
    else:
        print("Connection failed with code", rc)

# --- MQTT Callback: on_message ---
def on_message(client, userdata, msg):
    try:
        value = float(msg.payload.decode())
        if "ph" in msg.topic:
            print(f" pH Value: {value}")
            save_to_db("ph", value)
        elif "flow" in msg.topic:
            print(f" Flow Rate: {value}")
            save_to_db("flow", value)
        elif "level" in msg.topic:
            print(f" Water Level: {value}")
            save_to_db("level", value)
        elif "tds" in msg.topic:
            print(f" TDS Value: {value}")
            save_to_db("tds", value)
        elif "turbidity" in msg.topic:
            print(f" Turbidity Value: {value}")
            save_to_db("turbidity", value)
    except:
        print("Invalid data received")

# --- Main Execution ---
if __name__ == "__main__":
    create_tables()

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_forever()
