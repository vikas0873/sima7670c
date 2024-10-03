
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "globleVariables.cpp"

#define MQTT_RECONNECT_INTERVAL 10000
class MyMqttClient
{
    MyMqttClient() : client(Client) {}

public:
    static MyMqttClient &getInstance()
    {
        static MyMqttClient instance;
        return instance;
    };
    void mqttSetup(const char *mqtt_server, int mqtt_port, String mqtt_username, String mqtt_password)
    {
        client.setServer(mqtt_server, mqtt_port);
        client.setCallback([this](char *topic, byte *message, unsigned int length)
                           { this->callback(topic, message, length); });
    };
    bool isMqttConnected()
    {
        if (!client.connected())
        {
            return false;
        }
        else
        {
            return true;
        }
    };
    bool mqttReconnect(String mqtt_client_name)
    {
        static unsigned long mqtt_previous_millis = 0;
        unsigned long mqtt_current_millis = millis();
        if (!client.connected())
        {
            if (mqtt_current_millis - mqtt_previous_millis >= MQTT_RECONNECT_INTERVAL)
            {
                mqtt_previous_millis = mqtt_current_millis;
                client.connect(mqtt_client_name.c_str());
            }
            return false;
        }
        return true;
    };
    void mqttPublish(String topic, String message, bool topic_switch_flag = true)
    {
        if (topic_switch_flag == true)
        {
            String new_topic = subdomain + "/" + project_id + "/" + node_id + "/" + topic;
            client.publish(new_topic.c_str(), message.c_str());
        }
        else if (!topic_switch_flag)
        {
            client.publish(topic.c_str(), message.c_str());
        }
    };

    void mqttSubscribe(String topic, bool topic_switch_flag = true)
    {
        if (topic_switch_flag == true)
        {
            String new_topic = subdomain + "/" + project_id + "/" + node_id + "/" + topic;
            client.subscribe(new_topic.c_str());
        }
        else if (!topic_switch_flag)
        {
            client.subscribe(topic.c_str());
        }
    };
    bool payload_flag = false;
    void getMessage(String input_topic, String *payload, unsigned int *length)
    {
        if (strcmp(topic_mqtt.c_str(), input_topic.c_str()) == 0)
        {
            *payload = payload_mqtt;
            *length = payload_length_mqtt;
            payload_mqtt = "";       // Set to an empty string or some default value
            payload_length_mqtt = 0; // Set to the default length or 0
        }
    };
    String getMessageApp(String input_topic)
    {
        if (!payload_flag)
        {
            return payload_mqtt;
        }
        return "";
    };
    void mqttLoop()
    {
        client.loop();
    };

private:
    WiFiClient Client;
    PubSubClient client;

    String topic_mqtt;
    String payload_mqtt;
    unsigned int payload_length_mqtt;
    bool mqttConnected;

    MyMqttClient();
    void callback(char *topic, byte *message, unsigned int length)
    {
        payload_flag = false;
        topic_mqtt = String(topic);
        Serial.println("callback :: " + topic_mqtt);
        String messageTemp;
        for (int i = 0; i < length; i++)
        {
            messageTemp += (char)message[i];
        }
        payload_mqtt = messageTemp;
        Serial.print("payload_mqtt :: ");
        Serial.println(payload_mqtt);
        payload_length_mqtt = length;
    };
};

// Define the macro for easier access
#define PUBSUB_INSTANCE MyMqttClient::getInstance()
