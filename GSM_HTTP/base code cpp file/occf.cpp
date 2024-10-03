#include "Arduino.h"
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <flageManager.cpp>
#include "globleVariables.cpp"
#include "utils.cpp"
#include "mqtt.cpp"
#include "lcu.cpp"

#define OCCF_HTTP_CODE 200
#define OCCF_FILE_NAME "el_config.json"
#define DATA_KEY "data"
#define JSON_REPLY_KEY "ack_string"
#define OCCF_DOC_CAPACITY 20000

//---------------------defined for htbt switch case-----------------------------------------------------
#define DOWNLOAD_CONFIG_FILE 0
#define RESET_NODE 1
#define OFFLINE_JSON 2
#define DEFAULT_JSON 3
#define EXIT 4
//------------------------------------------------------------------------------------------------------

class Occf
{
public:
    String server_host = SERVER_HOST;
    String OCCF_SERVER_URL = "https://" + server_host + "/occf/config";

    FlagManager flagManager;
    Lcu lcu;
    Utils utils;

    void occfMain()
    {

        unsigned long occf_start_time = millis();
        bool loop_complete = false;
        int occf_flow = DOWNLOAD_CONFIG_FILE;

        while (!loop_complete)
        {
            switch (occf_flow)
            {
            case DOWNLOAD_CONFIG_FILE:
            {
                JsonVariant temp_json_variant;
                String payload;
                int occf_http_code;
                String mac_addr = utils.getMacAddress();
                String occf_mqtt_ack_publish_topic = "orcanode/" + mac_addr + "/occf/ack";

                downloadJsonFromServer(OCCF_SERVER_URL, &payload, &occf_http_code);
                //--------------------------------------------LOGGING--------------------------------------------------
                lcu.logs("OCCF|occfMain|OCCF HTTP CODE RESPONSE: " + String(occf_http_code), INFO, true);
                //--------------------------------------------LOGGING--------------------------------------------------
                Serial.println(payload);
                if (occf_http_code == 200)
                {
                    int key_available = utils.parseValueFromJsonKey(payload, DATA_KEY, &temp_json_variant);
                    if (key_available == 1)
                    {
                        String config_data = utils.convertJsonVariant<String>(temp_json_variant);
                        utils.saveJsonStringToFile(OCCF_FILE_NAME, config_data);
                        //--------------------------------------------LOGGING--------------------------------------------------
                        lcu.logs("OCCF|occfMain|SAVED CONFIG FILE TO SPIFFS", INFO, true);
                        //--------------------------------------------LOGGING--------------------------------------------------

                        int ack_key_available = utils.parseValueFromJsonKey(payload, JSON_REPLY_KEY, &temp_json_variant);
                        if (ack_key_available)
                        {
                            //--------------------------------------------LOGGING------------------------------------------------
                            lcu.logs("OCCF|occfMain|OCCF SENDING ACK TO SERVER", INFO, true);
                            //--------------------------------------------LOGGING------------------------------------------------
                            String ack_data = utils.convertJsonVariant<String>(temp_json_variant);
                            /* sendAckToServer(OCCF_SERVER_REPLY_URL, ack_data, &ack_payload, &ack_http_code); */
                            DynamicJsonDocument occf_ack_doc(200);
                            String occf_ack_string;
                            occf_ack_doc[JSON_REPLY_KEY] = ack_data;
                            serializeJson(occf_ack_doc, occf_ack_string);
                            occf_ack_doc.clear();
                            Serial.println(occf_ack_string);
                            PUBSUB_INSTANCE.mqttPublish(occf_mqtt_ack_publish_topic, occf_ack_string, false);
                            delay(1000);
                            occf_flow = RESET_NODE;
                        }
                        else
                        {
                            //--------------------------------------------LOGGING------------------------------------------------
                            lcu.logs("OCCF|occfMain|OCCF ACK KEY NOT FOUND", WARN, true);
                            //--------------------------------------------LOGGING------------------------------------------------
                        }
                    }
                    else
                    {
                        //--------------------------------------------LOGGING--------------------------------------------------
                        lcu.logs("OCCF|occfMain|DATA KEY IS NOT AVAIALBLE IN DOWNLOADED RESPONSE", WARN, true);
                        //--------------------------------------------LOGGING--------------------------------------------------
                        occf_flow = OFFLINE_JSON;
                    }
                }
                else
                {
                    //--------------------------------------------LOGGING--------------------------------------------------
                    lcu.logs("OCCF|occfMain|OCCF HTTP REQUEST FAILED", WARN, true);
                    //--------------------------------------------LOGGING--------------------------------------------------
                    occf_flow = OFFLINE_JSON;
                }
                break;
            }

            case RESET_NODE:
            {
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("OCCF|occfMain|OCCF CONFIG FILE DOWNLOADED, RESETING THE DEVICE", WARN, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------

                ESP.restart();
            }

            case OFFLINE_JSON:
            {
                bool file_exist = checkJsonOffline(OCCF_FILE_NAME);
                if (file_exist)
                {
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    lcu.logs("OCCF|occfMain|OCCF OFFLINE JSON FILE IS OK", INFO, true);
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    occf_flow = EXIT;
                }
                else
                {
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    lcu.logs("OCCF|occfMain|OCCF JSON FILE IS NOT OK", WARN, true);
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    occf_flow = DEFAULT_JSON;
                }
                break;
            }

            case DEFAULT_JSON:
            {
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("OCCF|occfMain|OCCF DEFAULT JSON READING", WARN, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                occf_flow = EXIT;
                break;
            }

            case EXIT:
            {
                loop_complete = true;
                unsigned long occf_end_time = millis();
                float occf_execution_time = (occf_end_time - occf_start_time) / 1000.0;
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("OCCF|occfMain|OCCF EXECUTION TIME: " + String(occf_execution_time) + " SECONDS", INFO, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                break;
            }
            }
        }
    };

private:
    void downloadJsonFromServer(String server_url, String *payload, int *http_code)
    {
        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient https;
        https.setTimeout(15000);

        String mac_addr = utils.getMacAddress();

        if (https.begin(client, server_url.c_str()))
        {
            https.addHeader("macid", mac_addr.c_str());
        }

        int https_response_code = https.sendRequest("GET");
        *payload = https.getString();
        *http_code = https_response_code;
        https.end();
    };
    void sendAckToServer(String server_url, String send_payload, String *payload, int *http_code);
    bool checkJsonOffline(String file_name)
    {
        if (SPIFFS.exists("/" + file_name))
        {
            File file_read = SPIFFS.open("/" + file_name, "r");
            DynamicJsonDocument doc(OCCF_DOC_CAPACITY);
            DeserializationError error = deserializeJson(doc, file_read);
            if (error)
            {
                return false;
            }
            return true;
        }
        else
        {
            return false;
        }
    };
    bool saveJsonToSpiffs(String file_name, String data_recieved);
};
