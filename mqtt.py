import sqlite3
import paho.mqtt.client as mqtt
import ssl, random, string
import json
import os
import sys
import msvcrt

lock_filename = 'sample-locking.lock'
fh = open(lock_filename, "w")

try:
    print("Not lock yet")
    msvcrt.locking(fh.fileno(), msvcrt.LK_RLCK, len("LOCK!"))
    print("LOCK!")
except IOError as e:
    sys.exit(-1)
except OSError as o:
    sys.exit(-2)


#msvcrt.locking(fh.fileno(), msvcrt.LK_UNLCK, len("LOCK!"))

from configfile import mqttobj
mqtt_config = mqttobj()

## Define configuration
db_filename = 'Progetto_malnati.db'
mqtt_server = mqtt_config["mqtt_server"]
mqtt_port   = mqtt_config["mqtt_port"]
mqtt_user   = mqtt_config["mqtt_user"]
mqtt_pwd    = mqtt_config["mqtt_pwd"]

conn = sqlite3.connect(db_filename)
c = conn.cursor()

# Helper function
def init_db():
    schema = open('schema.sql', 'r').read()
    sql_commands = schema.split(";")
    for command in sql_commands:
        c.execute(command)
    conn.commit()

init_db()

def on_connect(client, userdata, flags, rc):
    client.subscribe("pds/ESP32/myhome/tx/#")

def on_message(client, userdata, msg):
    root_mac = msg.topic.split("/").pop()
    print(root_mac)
    data = msg.payload.decode("utf-8").split("|")
    print(data)
    try:
        c.execute("""   INSERT INTO Originale('MAC','ISPUB','RSSI','TIMESTAMP','ROOT')
                        VALUES ('{}', {}, {}, '{}', '{}')""".format(data[0], data[3], data[1], data[2], root_mac))
    except sqlite3.Error as e:
            print("Database error: %s" % e)
    conn.commit()

def random_char(y):
    return ''.join(random.choice(string.ascii_letters) for x in range(y))

print("Start MQTT")
client_id = "server-" + random_char(5)
client = mqtt.Client(client_id)
client.username_pw_set(mqtt_user, mqtt_pwd)

client.on_connect = on_connect
client.on_message = on_message

client.connect(mqtt_server, mqtt_port, 60)
client.loop_forever()

conn.close()
