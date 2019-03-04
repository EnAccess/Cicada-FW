# https://github.com/eclipse/paho.mqtt.embedded-c

project(
    'paho.mqtt.embedded-c',
    'c',
    'cpp'
)

eclipse_paho_mqtt_inc = include_directories([
    './src/MQTTPacket/src',
    './src/MQTTClient/src',
    './src/MQTTClient/src/linux'
])

eclipse_paho_mqtt_src = files([
    './src/MQTTPacket/src/MQTTConnectClient.c',
    './src/MQTTPacket/src/MQTTConnectServer.c',
    './src/MQTTPacket/src/MQTTDeserializePublish.c',
    './src/MQTTPacket/src/MQTTFormat.c',
    './src/MQTTPacket/src/MQTTPacket.c',
    './src/MQTTPacket/src/MQTTSerializePublish.c',
    './src/MQTTPacket/src/MQTTSubscribeClient.c',
    './src/MQTTPacket/src/MQTTSubscribeServer.c',
    './src/MQTTPacket/src/MQTTUnsubscribeClient.c',
    './src/MQTTPacket/src/MQTTUnsubscribeServer.c'
])

eclipse_paho_mqtt_dep = declare_dependency(
    include_directories : [ eclipse_paho_mqtt_inc ],
	sources             : [ eclipse_paho_mqtt_src ]
)