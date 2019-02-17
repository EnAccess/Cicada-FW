# MQTT Include directories
eclipse_paho_mqtt_inc = []
eclipse_paho_mqtt_inc += include_directories('./paho.mqtt.embedded-c/MQTTPacket/src')
eclipse_paho_mqtt_inc += include_directories('./paho.mqtt.embedded-c/MQTTClient/src')
eclipse_paho_mqtt_inc += include_directories('./paho.mqtt.embedded-c/MQTTClient/src/linux')

#MQTT  Source files
eclipse_paho_mqtt_src = []
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectClient.c']
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectServer.c']
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTDeserializePublish.c']
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTFormat.c']
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTPacket.c']
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTSerializePublish.c']
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeClient.c']
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeServer.c']
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeClient.c']
eclipse_paho_mqtt_src += ['./paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeServer.c']

# Embedded printf files
embedded_printf_inc = include_directories(['./printf'])
embedded_printf_src = files(['./printf/printf.c'])

eclipse_paho_mqtt_src = files(eclipse_paho_mqtt_src)
