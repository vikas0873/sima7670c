#include "Arduino.h"
#include "SPIFFS.h" //This library is use to save, read and edit different file format
#include <WiFi.h>   //wifi library is use for AP, WIFI connection, and for wifi managment

#include <ArduinoJson.h>
#include <ESP32Ping.h>
#include "ledSetup.cpp"
#include "lcu.cpp"
#include "bluetooth.cpp"
#include "bootup_parameters.cpp"
#include "globleVariables.cpp"
#include "flageManager.cpp"

#define DEFAULT_SSID_PASSWORD "{\"TW2\": \"T3chW@lnut2\"}"
#define LOOP_TIME 60000           // bluetooth will stay on for this much time period
#define SSID_CHANGE_INTERVAL 7000 // time given to each ssid to try wifi connectivity
#define SSID_START_MILLIS -7000
#define WIFI_FILE "wifi_credentials_file.json" // file in which all wifi credentials will be stored
#define WNCF_DOC_CAPACITY 512                  // heap memory size in bytes for json to store temp data
#define REPEATWIFICONST 4                      // number of trials for each saved ssid

class Wncf : public Bluetooth
{
private:
    String getMacLastSixBytes()
    {
        uint8_t mac[6];
        char mac_str[18];
        WiFi.macAddress(mac);
        snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        String mac_address_no_colons = "";
        for (int i = 0; i < strlen(mac_str); i++)
        {
            if (mac_str[i] != ':')
            {
                mac_address_no_colons += mac_str[i];
            }
        }
        String last_four_bytes = mac_address_no_colons.substring(4);
        last_four_bytes.toUpperCase();

        return last_four_bytes;
    };

public:
    IPAddress local_IP;
    IPAddress gateway;
    IPAddress subnet;
    //    WebServer server;

    FlagManager flagManager;
    LedBlink ledblink;
    Lcu lcu;

    enum WNCFFLOW
    {
        BOOTUPCHECK,
        WNCFSETUP,
        WNCFLOOP,
        WNCFEXIT
    };

    enum WNCFSETUPFLOW
    {
        CHECKTIMER,
        RUNSERVER,
        WNCFLOOPEXIT
    };

    int wncf_setup_flow = CHECKTIMER;
    int wifi_trial_count = REPEATWIFICONST;

    bool isWifiOnBoard()
    {
        WiFi.disconnect(true);
        delay(1000); // Allow time for WiFi module to process the disconnection
        WiFi.begin("TEST_SSID", "TEST_PASSWORD");
        delay(1000); // Allow time for WiFi module to start the connection process
        bool wifi_flag;
        if (WiFi.status() == WL_NO_SHIELD)
        {
            wifi_flag = false;
        }
        else
        {
            wifi_flag = true;
        }
        WiFi.disconnect(true);
        delay(1000); // Allow time for WiFi module to process the disconnection and reset
        return wifi_flag;
    };

    void wncfBluetoothMain()
    {
        unsigned long wncf_start_time = millis();
        //  //--------------------------------------------LOGGING--------------------------------------------------------------------------------
        lcu.logs("WNCF|wncfBluetoothMain|######WNCF BLUETOOTH START######", DEBUG, true);
        //  //--------------------------------------------LOGGING--------------------------------------------------------------------------------
        bool loop_complete = false; // Flag to control the main loop
        int wncf_flow = BOOTUPCHECK;

        while (!loop_complete)
        {

            switch (wncf_flow)
            {
            case BOOTUPCHECK:
            {

                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("WNCF|wncfBluetoothMain|WNCF MEC AND BLC WORKING CHECK CASE", INFO, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------

                int bluetooth_status = flagManager.getFlagStatus(boot_up_bit_series, BLC, 0);
                int memory_status = flagManager.getFlagStatus(boot_up_bit_series, MEC, 0);

                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("WNCF|wncfBluetoothMain|WNCF MEC WORKING CHECK: " + String(memory_status), INFO, true);
                lcu.logs("WNCF|wncfBluetoothMain|WNCF BLC WORKING CHECK: " + String(bluetooth_status), INFO, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------

                if (!bluetooth_status || !memory_status) // If either Bluetooth or memory status is not OK
                {
                    wncf_flow = WNCFEXIT;
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    lcu.logs("WNCF|wncfBluetoothMain|WNCF MAIN EXIT CASE", INFO, true);
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                }
                else
                {
                    wncf_flow = WNCFSETUP;
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    lcu.logs("WNCF|wncfBluetoothMain|WNCF SETUP CASE", DEBUG, true);
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                }
                break;
            }
            case WNCFSETUP:
            {
                ledblink.ledSetup();
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("WNCF|wncfBluetoothMain|WNCF BLUETOOTH SETUP CASE", DEBUG, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                String four_byte_mac_addr = getMacLastSixBytes();
                four_byte_mac_addr = String(ORCANODE_NAME) + "-" + four_byte_mac_addr;
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("WNCF|wncfBluetoothMain|WNCF BLUETOOTH NAME: " + four_byte_mac_addr, INFO, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                bluetoothSetup(four_byte_mac_addr);

                wncf_flow = WNCFLOOP;
                break;
            }
            case WNCFLOOP:
            {
                static unsigned long bluetooth_previous_millis = 0;
                unsigned long bluetooth_current_millis = millis();
                static bool timeout; // Static variable to indicate timeout
                static bool connect_disconnect_flage = false;
                String ble_status = ""; // Variable to hold Bluetooth status
                switch (wncf_setup_flow)
                {
                case CHECKTIMER:
                {
                    if (bluetooth_current_millis - bluetooth_previous_millis >= LOOP_TIME) // Check if the loop time has passed
                    {
                        timeout = true;
                    }
                    else
                    {
                        timeout = false;
                    }
                    if (timeout)
                    {
                        SerialBT.end();
                        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                        lcu.logs("WNCF|wncfBluetoothMain|BLUETOOTH TURNED OFF", DEBUG, true);
                        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                        wncf_setup_flow = WNCFLOOPEXIT;
                    }
                    else
                    {
                        wncf_setup_flow = RUNSERVER;
                    }
                    break;
                }
                case RUNSERVER:
                {

                    bool device_connected_to_bluetooth = checkDeviceConnectedBluetooth(); // Check if a device is connected to Bluetooth
                    if (device_connected_to_bluetooth)
                    {
                        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                        lcu.logs("WNCF|wncfBluetoothMain|DEVICE CONNECTED TO BLUETOOTH", INFO, true);
                        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                        connect_disconnect_flage = true;
                        communicateWithBluetoothDevice(&ble_status);
                        bluetooth_previous_millis = bluetooth_current_millis;
                    }
                    else
                    {
                        wncf_setup_flow = CHECKTIMER;
                    }

                    bool wifi_status = isWiFiConnected();
                    if (!wifi_status)
                    {
                        ledblink.WifiLedPin();
                    }
                    else
                    {
                        ledblink.WifiLedPin(true);
                        SerialBT.end(); // End Bluetooth communication
                    }

                    // check Bluetooth Timeout/disconnect
                    if ((!checkDeviceConnectedBluetooth() && connect_disconnect_flage) || (connect_disconnect_flage && ble_status == "TIMEOUT"))
                    {
                        wncf_flow = WNCFLOOPEXIT;
                        SerialBT.end();
                        delay(500); // Ensure the connection is fully terminated before proceeding
                    }

                    break;
                }
                case WNCFLOOPEXIT:
                {
                    wncf_flow = WNCFEXIT;
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    lcu.logs("WNCF|wncfBluetoothMain|WNCF LOOP EXIT CASE", DEBUG, true);
                    //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                    break;
                }
                }
                break;
            }

            case WNCFEXIT:
            {
                loop_complete = true;
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("WNCF|wncfBluetoothMain|###### WNCF EXIT #######", DEBUG, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                unsigned long wncf_end_time = millis();
                float wncf_execution_time = (wncf_end_time - wncf_start_time) / 1000.0; // Calculate the execution time in seconds

                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("WNCF|wncfBluetoothMain|WNCF EXECUTION TIME: " + String(wncf_execution_time) + " SECONDS", INFO, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                break;
            }
            }
        }
    }

    bool isWiFiConnected()
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            return true;
        }
        else
        {
            return false;
        }
    };
    
    String connectedToSsid()
    {
        return WiFi.SSID();
    };
   
    bool connectToWiFi()
    {
        static int index = 0;
        bool flag = false;

        static unsigned long wifi_previous_millis = SSID_START_MILLIS;
        unsigned long wifi_current_millis = millis();

        if (wifi_current_millis - wifi_previous_millis >= SSID_CHANGE_INTERVAL)
        {
            wifi_previous_millis = wifi_current_millis;
            WiFi.mode(WIFI_STA);
            File file_read = SPIFFS.open("/" + String(WIFI_FILE), "r+");
            DynamicJsonDocument doc(WNCF_DOC_CAPACITY);
            deserializeJson(doc, file_read);
            JsonObject root = doc["wifi"].as<JsonObject>();
            JsonObject::iterator it = root.begin();

            it += index;
            String ssid = it->key().c_str();
            String password = it->value().as<String>();

            if (ssid != NULL)
            {
                WiFi.begin(ssid.c_str(), password.c_str());
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("WNCF|connectToWiFi|WNCF TRYING TO CONNECT TO SAVED SSID: " + ssid, INFO, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                index += 1;
                flag = false;
            }
            else
            {
                index = 0;
                deserializeJson(doc, DEFAULT_SSID_PASSWORD);
                JsonObject root = doc.as<JsonObject>();
                JsonObject::iterator default_it = root.begin();
                it += index;
                String ssid = default_it->key().c_str();
                String password = default_it->value().as<String>();
                WiFi.begin(ssid.c_str(), password.c_str());
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                lcu.logs("WNCF|connectToWiFi|WNCF TRYING TO CONNECT TO DEFAULT SSID: " + ssid, INFO, true);
                //--------------------------------------------LOGGING--------------------------------------------------------------------------------
                flag = true;
            }
            doc.clear();
            file_read.close();
        }
        return flag;
    }; // setup
};
