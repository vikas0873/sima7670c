#include <WiFi.h>
#include <SPIFFS.h>
#include "Arduino.h"
#include <ArduinoJson.h>
#include "flagemanager.cpp"
#include "globleVariables.cpp"

//----------------------------------DOC CAPACITY--------------------------------------------------------
#define UTILS_DOC_CAPACITY 20000

//----------------------------------el_config_keys--------------------------------------------------------
#define SUBDOMAIN_KEY "subdomain"
#define NODE_ID_KEY "node_id"
#define PROJECT_ID_KEY "project_key"
#define MODE_STATUS_KEY "mode_key"

//--------------------------------------------------------------------------------------------------------

class Utils
{
public:
    FlagManager flagManager;

    String getMacAddress() /* this function returns node mac id*/
    {
        return WiFi.macAddress();
    };
    void listAllFilesInSpiffs() /* this function prints all file names stored in spiffs */
    {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file)
        {
            file = root.openNextFile();
        }
    };

    template <typename variant_type>
    static variant_type convertJsonVariant(JsonVariant json_value)
    {
        return json_value.as<variant_type>();
    }

    template <typename custom_struct>
    int readLockUnlockListFromArray(std::map<int, custom_struct> &dictionary, int index)
    {
        int lock_status;
        for (const auto &pair : dictionary)
        {
            if (pair.first == index)
            {
                lock_status = pair.second.lock_unlock_list;
                return lock_status;
            }
        }
        return 0;
    }

    int parseValueFromJsonKey(String http_response, String key, JsonVariant *json_variant)
    {
        DynamicJsonDocument doc(UTILS_DOC_CAPACITY);
        DeserializationError error = deserializeJson(doc, http_response);
        if (error)
        {
            return -1;
        }
        //  serializeJson(doc, Serial);//remove afterward
        //  Serial.println();//remove afterward
        JsonVariant json_parse = findNestedKey(doc.as<JsonObject>(), key.c_str());

        if (!json_parse.isNull())
        {
            *json_variant = json_parse;
            doc.clear();
            return 1;
        }
        else
        {
            return 0;
        }
    };
    bool saveJsonStringToFile(String file_name, String data)
    {
        DynamicJsonDocument doc(UTILS_DOC_CAPACITY);
        DeserializationError error = deserializeJson(doc, data);
        if (error)
        {
            return false;
        }
        File file_read = SPIFFS.open("/" + file_name, "w");
        serializeJson(doc, file_read);
        //  serializeJson(doc, Serial);
        //  Serial.println("");
        file_read.close();
        doc.clear();
        return true;
    };
    bool convertJsonFileToString(String file_name, String *json_string)
    {
        DynamicJsonDocument doc(UTILS_DOC_CAPACITY);

        File file_read = SPIFFS.open("/" + file_name, "r+");
        if (!file_read)
        {
            file_read.close();
            return false;
        }

        DeserializationError error = deserializeJson(doc, file_read);
        if (error)
        {
            file_read.close();
            return false;
        }
        String temp_json_storage;
        serializeJson(doc, temp_json_storage);

        *json_string = temp_json_storage;
        file_read.close();
        doc.clear();
        return true;
    };
    bool readBootupFromConfigFile()
    {
        JsonVariant temp_json_variant;
        String bootup_data_string;

        bool file_read_successfully = convertJsonFileToString("el_config.json", &bootup_data_string); // get data from bootup.json and stores in string

        if (file_read_successfully)
        {
            parseValueFromJsonKey(bootup_data_string, SUBDOMAIN_KEY, &temp_json_variant); // will return value of subdomain from json to temp_jason_variant
            subdomain = convertJsonVariant<String>(temp_json_variant);                    // sub_domain is global variable, defined in global_variable.cpp

            parseValueFromJsonKey(bootup_data_string, NODE_ID_KEY, &temp_json_variant); // will return value of node_id from json to temp_jason_variant
            node_id = convertJsonVariant<String>(temp_json_variant);                    // node_id is global variable, defined in global_variable.cpp

            parseValueFromJsonKey(bootup_data_string, PROJECT_ID_KEY, &temp_json_variant); // will return value of project_id from json to temp_jason_variant
            project_id = convertJsonVariant<String>(temp_json_variant);                    // project_id is global variable, defined in global_variable.cpp

            int mode_key_present = parseValueFromJsonKey(bootup_data_string, MODE_STATUS_KEY, &temp_json_variant); // this will return if to log file or not
            if (mode_key_present)
            {
                int working_mode = convertJsonVariant<int>(temp_json_variant); // log_key is local variable
                int read_mode_index = readLockUnlockListFromArray(lock_unlock_mapped_list, working_mode);

                flagManager.setFlagStatus(lock_bit_series, read_mode_index, 0, 1);
            }
            else
            {
                int read_mode_index = 0;
                flagManager.setFlagStatus(lock_bit_series, read_mode_index, 0, 1);
            }
            return true;
        }
        else
        {
            return false;
        }
    };

private:
    JsonVariant findNestedKey(JsonObject obj, const char *key)
    {
        JsonVariant found_object = obj[key];
        if (!found_object.isNull())
            return found_object;

        for (JsonPair pair : obj)
        {
            JsonVariant nested_object = findNestedKey(pair.value(), key);
            if (!nested_object.isNull())
                return nested_object;
        }
        return JsonVariant();
    };
};