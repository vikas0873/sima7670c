#include "Arduino.h"
#include "flageManager.cpp"
#include "globleVariables.cpp"
#include "bootup_parameters.cpp"
#include "lcu.cpp"
#include "mqtt.cpp"
#include "utils.cpp"
#include <map>

//---------------------defined for htbt switch case-----------------------------------------------------
#define HTBT_TIMER 0
#define HTBT_MQTT_CHECK 1
#define HTBT_MQTT_PUBLISH 2
#define HTBT_EXIT 3

//----------------------------------el_config_keys--------------------------------------------------------
#define OCCF_STATUS_KEY "update_occf"

//--------------------------------------------------------------------------------------------------------
#define HTBT_INTERVAL 10000

class HeartBeat
{
    FlagManager flagManager;
    Lcu lcu;
    Utils utils;

public:
    int htbt_flow = HTBT_TIMER;

    void heartBeatMain()
    {
        bool loop_complete = false;
        unsigned long current_millis = millis();
        static long htbt_previous_millis = 0;
        while (!loop_complete)
        {
            switch (htbt_flow)
            {
            case HTBT_TIMER:
            {
                if (current_millis - htbt_previous_millis >= HTBT_INTERVAL)
                {
                    htbt_previous_millis = current_millis;
                    htbt_flow = HTBT_MQTT_CHECK;
                }
                else
                {
                    loop_complete = true;
                }
                break;
            }

            case HTBT_MQTT_CHECK:
            {
                if (PUBSUB_INSTANCE.isMqttConnected())
                {
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    lcu.logs("HEARTBEAT|heartBeatMain|CONNECTED TO MQTT SERVER", INFO, true);
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    htbt_flow = HTBT_MQTT_PUBLISH;
                }
                else
                {
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    lcu.logs("HEARTBEAT|heartBeatMain|NOT CONNECTED TO MQTT SERVER", WARN, true);
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    htbt_flow = HTBT_EXIT;
                }
                break;
            }

            case HTBT_MQTT_PUBLISH:
            {
                String mac_addr = utils.getMacAddress();

                String htbt_mqtt_publish_topic = "orcanode/" + mac_addr + "/config/htbt";

                PUBSUB_INSTANCE.mqttPublish(htbt_mqtt_publish_topic, bootup_json_var, false); /* input datatype - string */
                htbt_flow = HTBT_EXIT;
                break;
            }

            case HTBT_EXIT:
            {
                htbt_flow = HTBT_TIMER;
                unsigned long htbt_end_time = millis();
                float htbt_execution_time = (htbt_end_time - current_millis) / 1000.0;
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("HEARTBEAT|heartBeatMain|HEARTBEAT EXECUTION TIME: " + String(htbt_execution_time) + " SECONDS", INFO, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                loop_complete = true;
                break;
            }
            }
        }
    };
    void heartBeatGetSubscribeData()
    {
        JsonVariant temp_json_variant;
        String htbt_ack_payload = "";
        unsigned int htbt_ack_length = 0;
        static bool one_time_subscribe_flage = false;

        String mac_addr = utils.getMacAddress();
        String htbt_mqtt_subscribe_topic = "orcanode/" + mac_addr + "/htbt/ack";

        if (PUBSUB_INSTANCE.isMqttConnected())
        {
            if (!one_time_subscribe_flage)
            {
                PUBSUB_INSTANCE.mqttSubscribe(htbt_mqtt_subscribe_topic, false); /* input datatype - string */
                one_time_subscribe_flage = true;
            }
        }
        else
        {
            one_time_subscribe_flage = false;
        }

        PUBSUB_INSTANCE.getMessage(htbt_mqtt_subscribe_topic, &htbt_ack_payload, &htbt_ack_length);

        int occf_key_available = utils.parseValueFromJsonKey(htbt_ack_payload, OCCF_STATUS_KEY, &temp_json_variant); /* will return value of project_id from json to temp_jason_variant */

        if (occf_key_available == 1)
        {
            int occf_response_status = utils.convertJsonVariant<int>(temp_json_variant);
            //--------------------------------------------LOGGING--------------------------------------------------------------------------------
            lcu.logs("HEARTBEAT|heartBeatMain|OCCF STATUS: " + String(occf_response_status), WARN, true);
            //--------------------------------------------LOGGING--------------------------------------------------------------------------------
            if (occf_response_status == 1)
            {
                flagManager.setFlagStatus(block_status_bit_series, DOWNLOAD_OCCF, SET, 0);
            }
        }
    };
};