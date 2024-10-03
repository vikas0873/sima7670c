#include <flageManager.cpp>
#include <lcu.cpp>
#include <bluetooth.cpp>
#include <globleVariables.cpp>
#include <utils.cpp>
#include <wncf.cpp>

#define BOOTUP_FILE_NAME "el_diagnosis.json"
class Bootup
{
public:
    FlagManager flagManager;
    Lcu lcu;
    Utils utils;
    Wncf wncf;
    Bluetooth bluetooth;

    bool isSerialRunning()
    {
        Serial.begin(115200);
        delay(1000);
        return Serial ? true : false;
    }

    bool isMemoryRunning()
    {
        return SPIFFS.begin(true);
    }

    bool isWiFiRunning()
    {
        return wncf.isWifiOnBoard();
    }

    bool isHotspotRunning()
    {
        bool temp_flag = false;
        WiFi.mode(WIFI_AP);
        if (WiFi.getMode() == WIFI_AP)
        {
            temp_flag = true;
        }
        WiFi.softAPdisconnect(true);
        return temp_flag;
    }

    bool isBluetoothRunning()
    {
        return bluetooth.isBluetoothOnBoard();
    }

    bool isI2cRunning()
    {
        return false;
    }

    bool isSpiRunning()
    {
        return false;
    }

    bool isEthernetRunning()
    {
        return false;
    }

    bool isExternalMemoryRunning()
    {
        return false;
    }

    bool isRtcRunning()
    {
        return false;
    }

    bool isSim800lRunning()
    {
        return false;
    }

    void bootupMain()
    {
        DynamicJsonDocument doc(512);
        unsigned long bootup_start_time = millis();
        int array_length = 0;

        while (hardware_to_check[array_length] != "")
        {
            delay(50);
            const char *hardware_to_find = hardware_to_check[array_length].c_str();
            auto iterate = bootup_check.find(hardware_to_find);
            int bootup_cases = iterate->second;
            String bootup_hardware_name = iterate->first;
            array_length += 1;

            switch (bootup_cases)
            {
            case SERIALCOM:
            {
                bool serial_status = isSerialRunning();
                if (serial_status)
                {
                    lcu.logs("BOOTUP|bootupMain|SERIAL INITIATED SUCCESSFULLY", INFO, true);
                    flagManager.setFlagStatus(boot_up_bit_series, SRC, SET, 0);
                }
                else
                {
                    lcu.logs("BOOTUP|bootupMain|SERIAL FAILED", ERROR, true);
                    flagManager.setFlagStatus(boot_up_bit_series, SRC, RESET, 0);
                }
                break;
            }

            case MEMORY:
            {
                bool memory_status = isMemoryRunning();
                if (memory_status)
                {
                    lcu.logs("BOOTUP|bootupMain|SPIFFS WORKING", INFO, true);
                    flagManager.setFlagStatus(boot_up_bit_series, MEC, SET, 0);
                }
                else
                {
                    lcu.logs("BOOTUP|bootupMain|SPIFFS NOT WORKING", ERROR, true);
                    flagManager.setFlagStatus(boot_up_bit_series, MEC, RESET, 0);
                }
                break;
            }

            case WIFI:
            {
                bool wifi_status = isWiFiRunning();
                if (wifi_status)
                {
                    lcu.logs("BOOTUP|bootupMain|Wi-Fi WORKING", INFO, true);
                    flagManager.setFlagStatus(boot_up_bit_series, WFC, SET, 0);

                    bool hotspot_check = isHotspotRunning();
                    if (hotspot_check)
                    {
                        flagManager.setFlagStatus(boot_up_bit_series, APC, SET, 0);
                        lcu.logs("BOOTUP|bootupMain|HOTSPOT IS WORKING", INFO, true);
                    }
                    else
                    {
                        flagManager.setFlagStatus(boot_up_bit_series, APC, RESET, 0);
                        lcu.logs("BOOTUP|bootupMain|HOTSPOT IS NOT WORKING", ERROR, true);
                    }
                }
                else
                {
                    lcu.logs("BOOTUP|bootupMain|Wi-Fi NOT WORKING", ERROR, true);
                    flagManager.setFlagStatus(boot_up_bit_series, WFC, RESET, 0);
                    flagManager.setFlagStatus(boot_up_bit_series, APC, RESET, 0);
                    lcu.logs("BOOTUP|bootupMain|CANNOT CHECK HOTSPOT", ERROR, true);
                }
                break;
            }

            case BLUETOOTH:
            {
                bool bluetooth_status = isBluetoothRunning();
                if (bluetooth_status)
                {
                    flagManager.setFlagStatus(boot_up_bit_series, BLC, SET, 0);
                    lcu.logs("BOOTUP|bootupMain|BLUETOOTH WORKING", INFO, true);
                }
                else
                {
                    flagManager.setFlagStatus(boot_up_bit_series, BLC, RESET, 0);
                    lcu.logs("BOOTUP|bootupMain|BLUETOOTH FAILED", ERROR, true);
                }
                break;
            }

            case I2CCOM:
            {
                bool i2c_status = isI2cRunning();
                if (i2c_status)
                {
                    flagManager.setFlagStatus(boot_up_bit_series, ICC, SET, 0);
                    lcu.logs("BOOTUP|bootupMain|I2C COM WORKING", INFO, true);
                }
                else
                {
                    flagManager.setFlagStatus(boot_up_bit_series, ICC, RESET, 0);
                    lcu.logs("BOOTUP|bootupMain|I2C COM NOT WORKING", ERROR, true);
                }
                break;
            }

            case SPICOM:
            {
                bool spi_status = isSpiRunning();
                if (spi_status)
                {
                    flagManager.setFlagStatus(boot_up_bit_series, SPC, SET, 0);
                    lcu.logs("BOOTUP|bootupMain|SPI COM WORKING", INFO, true);
                }
                else
                {
                    flagManager.setFlagStatus(boot_up_bit_series, SPC, RESET, 0);
                    lcu.logs("BOOTUP|bootupMain|SPI COM NOT WORKING", ERROR, true);
                }
                break;
            }

            case ETHERNET:
            {
                bool ethernet_status = isEthernetRunning();
                if (ethernet_status)
                {
                    flagManager.setFlagStatus(boot_up_bit_series, ETC, SET, 0);
                    lcu.logs("BOOTUP|bootupMain|ETHERNET WORKING", INFO, true);
                }
                else
                {
                    flagManager.setFlagStatus(boot_up_bit_series, ETC, RESET, 0);
                    lcu.logs("BOOTUP|bootupMain|ETHERENT NOT WORKING", ERROR, true);
                }
                break;
            }

            case EXTERNALMEMORY:
            {
                bool external_memory_status = isExternalMemoryRunning();
                if (external_memory_status)
                {
                    flagManager.setFlagStatus(boot_up_bit_series, EXC, SET, 0);
                    lcu.logs("BOOTUP|bootupMain|EXTERNAL MEMORY WORKING", INFO, true);
                }
                else
                {
                    flagManager.setFlagStatus(boot_up_bit_series, EXC, RESET, 0);
                    lcu.logs("BOOTUP|bootupMain|EXTERNAL MEMORY NOT WORKING", ERROR, true);
                }
                break;
            }

            case REALTIMECLOCK:
            {
                bool rtc_status = isRtcRunning();
                if (rtc_status)
                {
                    flagManager.setFlagStatus(boot_up_bit_series, CKC, SET, 0);
                    lcu.logs("BOOTUP|bootupMain|REAL TIME CLOCK WORKING", INFO, true);
                }
                else
                {
                    flagManager.setFlagStatus(boot_up_bit_series, CKC, RESET, 0);
                    lcu.logs("BOOTUP|bootupMain|REAL TIME CLOCK NOT WORKING", ERROR, true);
                }
                break;
            }

            case SIM800LS:
            {
                bool sim800l_status = isSim800lRunning();
                if (sim800l_status)
                {
                    flagManager.setFlagStatus(boot_up_bit_series, GSC, SET, 0);
                    lcu.logs("BOOTUP|bootupMain|SIM800L WORKING", INFO, true);
                }
                else
                {
                    flagManager.setFlagStatus(boot_up_bit_series, GSC, RESET, 0);
                    lcu.logs("BOOTUP|bootupMain|SIM800L NOT WORKING", ERROR, true);
                }
                break;
            }

            default:
            {
                lcu.logs("BOOTUP|bootupMain|FOLLOWING HARDWARE DOES NOT EXIST: " + (bootup_hardware_name), WARN, true);
                break;
            }
            }
            doc[hardware_to_find] = flagManager.getFlagStatus(boot_up_bit_series, bootup_cases, 0);
        }

        doc["VERSION"] = VERSION;
        serializeJson(doc, bootup_json_var);
        doc.clear();

        lcu.logs("BOOTUP|bootupDiagnosis|ASSIGNED DATA TO VARIABLE", DEBUG, true);

        unsigned long bootup_end_time = millis();
        float bootup_execution_time = (bootup_end_time - bootup_start_time) / 1000.0;
        lcu.logs("BOOTUP|bootupMain|BOOTUP EXECUTION TIME: " + String(bootup_execution_time) + " SECONDS", INFO, true);
    }

    void printLogLevel()
    {
        if (LOG_LEVEL == 1)
        {
            lcu.logs("BOOTUP|printLogLevel|LOG LEVEL IS SET TO DEBUG", DEBUG, true);
        }
        else if (LOG_LEVEL == 2)
        {
            lcu.logs("BOOTUP|printLogLevel|LOG LEVEL IS SET TO INFO", INFO, true);
        }
        else if (LOG_LEVEL == 3)
        {
            lcu.logs("BOOTUP|printLogLevel|LOG LEVEL IS SET TO WARN", WARN, true);
        }
        else if (LOG_LEVEL == 4)
        {
            lcu.logs("BOOTUP|printLogLevel|LOG LEVEL IS SET TO ERROR", ERROR, true);
        }
        else if (LOG_LEVEL == 5)
        {
            lcu.logs("BOOTUP|printLogLevel|LOG LEVEL IS SET TO FATAL", FATAL, true);
        }
        else
        {
            lcu.logs("BOOTUP|printLogLevel|WRONG LOG LEVEL SET", DEBUG, true);
        }
    }
};