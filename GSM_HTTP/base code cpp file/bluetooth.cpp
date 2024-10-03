#include "Arduino.h"
#include "ledSetup.cpp"
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "BluetoothSerial.h"
#include "lcu.cpp"

#define WIFISTORAGELIMIT 4
#define TIMEINTERVAL 100
#define BLUETOOTHWIFIFILE "wifi_credentials_file.json"
#define BLUETOOTHCREDENTIALFILESIZE 1024
#define WIFI_CONNECTIVITY_TIME 7000

#define BLUETOOTHTIMEOUTINTERVAL 1000 * 120 /* 2min  */

class Bluetooth
{

public:
    BluetoothSerial SerialBT;
    LedBlink ledblink;
    Lcu lcu;

    bool isBluetoothOnBoard()
    {
        if (SerialBT.begin("ESP32_BT_TEST"))
        {
            delay(2000); // need to improve
            SerialBT.end();
            return true;
        }
        else
        {
            return false;
        }
    };
    bool checkDeviceConnectedBluetooth()
    {
        if (SerialBT.hasClient())
        {
            return true;
        }
        else
        {
            return false;
        }
    };
    void bluetoothSetup(String bluetooth_device_name)
    {
        SerialBT.begin(bluetooth_device_name.c_str()); // Bluetooth device name
    };
    String bluetoothInput(String input_print_string)
    {
        bool while_done = true;
        unsigned long startTime = millis();
        unsigned long prev_millis_for_timeout = startTime;

        String inputString = "";

        SerialBT.println(input_print_string);

        // Wait for Bluetooth Input Check if there is any data available on the Bluetooth serial connection.
        while (!SerialBT.available())
        {
            // Check if the Bluetooth device is still connected
            if (!checkDeviceConnectedBluetooth())
            {
                return "DISCONNECTED";
            }
            // Check for timeout
            if (millis() - prev_millis_for_timeout >= BLUETOOTHTIMEOUTINTERVAL)
            {
                return "TIMEOUT";
            }
        }
        delay(10);
        // Read the input from Bluetooth
        while (while_done)
        {
            if (SerialBT.available() > 0)
            {
                startTime = millis();
            }
            char c = SerialBT.read();
            if (c == '\n')
            {
                break;
            }
            else if (millis() - startTime >= TIMEINTERVAL)
            {
                break;
            }
            else
            {
                inputString += c;
            }
        }

        inputString.trim(); // To remove white spaces
        return inputString; // Return the received input string
    };
    void sendSsidPassToBluetooth(String file_name)
    {
        String string_send_to_bluetooth;
        File file = SPIFFS.open("/" + file_name, "r");
        DynamicJsonDocument doc(BLUETOOTHCREDENTIALFILESIZE);
        deserializeJson(doc, file);

        JsonObject root = doc["wifi"].as<JsonObject>();

        for (JsonPair key_value : root)
        {
            String ssid_key = key_value.key().c_str();
            String ssid_value = key_value.value().as<String>();
            string_send_to_bluetooth += ssid_key + " : " + ssid_value + "\n";
        }
        SerialBT.println(string_send_to_bluetooth);
        doc.clear();
        file.close();
    };
    bool bluetoothConnectToWifiNetwork(String ssid_var, String password_var)
    {
        bool connected = false;
        delay(200);
        SerialBT.println("TRYING TO CONNECT TO PROVIDED SSID AND PASSWORD");
        delay(1000);
        WiFi.begin(ssid_var.c_str(), password_var.c_str());

        unsigned long startTime = millis();

        // Wait for Wi-Fi connection
        while (!connected && millis() - startTime < WIFI_CONNECTIVITY_TIME)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                //      Serial.println("Connected to WiFi");
                connected = true;
            }
            else if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_NO_SSID_AVAIL)
            {
                //      Serial.println("Connection failed. Check SSID and password.");
                break;
            }
            delay(1000);
        }

        return connected;
    };
    bool bluetoothdeleteSelectedSsid(String ssid_key, String file_name)
    {
        File file = SPIFFS.open("/" + file_name, "r");
        DynamicJsonDocument doc(BLUETOOTHCREDENTIALFILESIZE);
        deserializeJson(doc, file);
        JsonObject root = doc["wifi"].as<JsonObject>();
        if (!root.containsKey(ssid_key.c_str()))
        {
            file.close();
            doc.clear();
            return false;
        }
        doc["wifi"].remove(ssid_key.c_str());
        doc.garbageCollect();
        //  Serial.println(doc.memoryUsage());
        file.close();
        File file_write = SPIFFS.open("/" + file_name, "w");
        serializeJsonPretty(doc, file_write);
        file_write.close();
        doc.clear();
        return true;
    };
    void bluetoothAddWifiToMemory(String ssid, String password, String file_name)
    {
        String last_key;

        File file = SPIFFS.open("/" + file_name, "r");
        DynamicJsonDocument doc(BLUETOOTHCREDENTIALFILESIZE);
        deserializeJson(doc, file);
        int wifi_object_size = doc["wifi"].size();
        //  Serial.println(wifi_object_size);
        if (wifi_object_size < WIFISTORAGELIMIT)
        {
            doc["wifi"][ssid] = password;
        }
        else
        {
            JsonObject root = doc["wifi"].as<JsonObject>();

            for (JsonPair key_value : root)
            {
                last_key = key_value.key().c_str();
            }
            doc["wifi"].remove(last_key);
            doc["wifi"][ssid] = password;
            doc.garbageCollect();
        }
        //  Serial.println(doc.memoryUsage());
        serializeJsonPretty(doc, Serial);
        file.close();

        File file_write = SPIFFS.open("/" + file_name, "w");
        serializeJsonPretty(doc, file_write);
        file_write.close();
    };
    void communicateWithBluetoothDevice(String *ble_status)
    {
        static int chosen_case_value = 0;

        switch (chosen_case_value)
        {
        case 0:
        {
            delay(200); // need to improve
            String command_any = bluetoothInput("ENTER ANYTHING TO START WIFI CONFIG");

            /* check if bletooth disconnect error and timeoutcheck */
            if (command_any == "DISCONNECTED")
            {
                *ble_status = "DISCONNECTED";
                break;
            }
            if (command_any == "TIMEOUT")
            {
                *ble_status = "TIMEOUT";
                break;
            }

            if (command_any.length() > 0)
            {
                chosen_case_value = 1;
            }
            break;
        }
        case 1:
        {
            delay(200);
            String chosen_case = bluetoothInput("TO ENTER SSID AND PASSWORD SEND :2\nTO DELETE SAVED SSIDS SEND :3\nTO CHECK LIST OF SSID SEND :4\nTO ESCAPE BLUETOOTH CONFIG ENTER :5");
            chosen_case_value = atoi(chosen_case.c_str());

            /* disconnect error and timeoutcheck */
            if (chosen_case == "DISCONNECTED")
            {
                *ble_status = "DISCONNECTED";
                break;
            }
            if (chosen_case == "TIMEOUT")
            {
                *ble_status = "TIMEOUT";
                break;
            }

            if (!(chosen_case_value >= 1 && chosen_case_value <= 5))
            {
                delay(200);
                SerialBT.println("WRONG CHOICE");
                chosen_case_value = 1;
            }
            break;
        }
        case 2:
        {
            delay(200);
            String ssid = bluetoothInput("ENTER WIFI SSID\nTO SKIP WRITE -> NONE");

            /* disconnect error and timeoutcheck */
            if (ssid == "DISCONNECTED")
            {
                *ble_status = "DISCONNECTED";
                break;
            }
            if (ssid == "TIMEOUT")
            {
                *ble_status = "TIMEOUT";
                break;
            }

            if (ssid == "none" || ssid == "None" || ssid == "NONE")
            {
                chosen_case_value = 1;
            }
            else
            {
                delay(200);
                String password = bluetoothInput("ENTER WIFI PASSWORD");

                /* disconnect error and timeoutcheck */
                if (password == "DISCONNECTED")
                {
                    *ble_status = "DISCONNECTED";
                    break;
                }
                if (password == "TIMEOUT")
                {
                    *ble_status = "TIMEOUT";
                    break;
                }

                delay(200);
                bool wifi_connect = bluetoothConnectToWifiNetwork(ssid, password);
                if (wifi_connect)
                {
                    delay(200);
                    SerialBT.println("CONNECTED TO WIFI: " + ssid);
                    bluetoothAddWifiToMemory(ssid, password, BLUETOOTHWIFIFILE);
                    chosen_case_value = 1;
                }
                else
                {
                    delay(200);
                    SerialBT.println("DID NOT CONNECT TO WIFI: " + ssid);
                    chosen_case_value = 2;
                }
            }
            break;
        }
        case 3:
        {
            delay(200);
            sendSsidPassToBluetooth(BLUETOOTHWIFIFILE);
            delay(200);
            String command = bluetoothInput("ABOVE IS THE LIST OF SAVED SSIDS\nTO DELETE SSID WRITE -> {SSID NAME}\nTO SKIP WRITE -> NONE");

            /* disconnect error and timeoutcheck */
            if (command == "DISCONNECTED")
            {
                *ble_status = "DISCONNECTED";
                break;
            }
            if (command == "TIMEOUT")
            {
                *ble_status = "TIMEOUT";
                break;
            }

            if (command == "none" || command == "None" || command == "NONE")
            {
                chosen_case_value = 1;
            }
            else
            {
                bool delete_flag = bluetoothdeleteSelectedSsid(command, BLUETOOTHWIFIFILE);
                if (delete_flag)
                {
                    delay(200);
                    SerialBT.println("SSID DELETED SUCCESSFULLY");
                    chosen_case_value = 3;
                }
                else
                {
                    delay(200);
                    SerialBT.println("DIDNOT FOUND THE SSID IN LIST OR WRONG SSID INPUT");
                    chosen_case_value = 3;
                }
            }
            break;
        }
        case 4:
        {
            delay(200);
            sendSsidPassToBluetooth(BLUETOOTHWIFIFILE);
            delay(200);
            String command = bluetoothInput("ABOVE IS THE LIST OF SAVED SSIDS\nTO EXIT WRITE -> NONE");

            /* disconnect error and timeoutcheck */
            if (command == "DISCONNECTED")
            {
                *ble_status = "DISCONNECTED";
                break;
            }
            if (command == "TIMEOUT")
            {
                *ble_status = "TIMEOUT";
                break;
            }
            command.toLowerCase();
            if (command == "none")
            {
                chosen_case_value = 1;
            }
            break;
        }
        case 5:
        {
            delay(200);
            SerialBT.println("EXITING BLUETOOTH CONFIG PANAL");
            delay(500);
            SerialBT.end();
            break;
        }
        }
    };
};