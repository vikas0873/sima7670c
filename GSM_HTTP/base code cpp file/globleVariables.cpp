#include "Arduino.h"
#include <map>
#include <deque>

//--------------------------------------------version----------------------------------------------------------------------[1]
#define VERSION "V1.0.0.2.4"

//---------------------------------------------name or orcanode------------------------------------------------------------[2]
#define ORCANODE_NAME "Orcanode_base"

//------------------------------------------application selection----------------------------------------------------------[3]
// #define UHF18
#define MODBUS

//--------------------------------------------list of hardware to check-----------------------------------------------------[4]
/* below is the array defined, to edit the array go to global_variable.cpp reference no [4] */

//--------------------------------------------base code mqtt variables----------------------------------------------------- [5]
#define BASE_MQTT_SERVER "mqtt.eldockyard.in" /* mqtt server name or ip address */
#define BASE_MQTT_PORT 1883                   /* mqtt server port if websocket is using then put websocket port number */
#define BASE_MQTT_USER_NAME ""
#define BASE_MQTT_PASSWORD ""

//-------------------------------------------common http request veriables variables-----------------------------------------[6]
#define SERVER_HOST "www.eldockyard.in"
#define SERVER_PORT 443

#define LCU_INTERVAL 2000
#define CLIENTTIMEOUT 20000

//-------------------------------------------lcu variables--------------------------------------------------------------------[7]
/* uncomment the log level which you want to enable */
#define DEBUG 1
#define INFO 2
#define WARN 3
#define ERROR 4
#define FATAL 5

#define LOG_LEVEL INFO

#define SET 1
#define RESET 0
#define ACCESSED 1

//----------------------bit position in block status bit series variables defined-----------------------------------------------[8]
#define WNCF 0                    /* wncf index in block status bit series which holds 1 for wncf working and 0 for not working */
#define LCU_LOGS 1                /* occf index in block status bit series which holds 1 for occf has made http request and 0 for yet accessed */
#define DOWNLOAD_OCCF 2           /* this is bit in block_status which holds 1 for offline file is available 0 for not */
#define INTERNAL_TIMER 3          /* holds a bit which represents 0 = epoch was not able to fetch from server and vice versa */
#define TIMER_FUNCTION_ACCESSED 4 /* this is to understand if timer function has been executed 0 = function yet execution and vice versa */
#define CONFIG_PRESENT 5          /* this bit represent if config data is present or not */

//--------------------------------------bit position in lock unlock bit series variable defined----------------------------------[9]
#define LOCK 0 /* bit position in bit series */
#define UNLOCK 1
#define LOGS 4

//-------------------------------------lock mapped list and htbt veriables-------------------------------------------------------[10]
#define NONE 0
#define GREEN 1
#define YELLOW 2
#define RED 3
#define LOGS 4

/* to edit the lock parameters got to global_variable.cpp refernace [10] */
struct lock_unlock_mapped_struct
{
    int status;
    String status_name;
    int lock_unlock_list;
};

//---------------------------------vector string var to store temp log before internal timer function------------------------------[12]
extern std::deque<String> log_strings; // this veriable will be use to store log string before getting time from internet for timestamp.

//-------------------------------------------vector queue(buffer) for tags---------------------------------------------------------[13]
extern std::deque<String> data_queue;

//--------------------------------------------list of hardware to check-----------------------------------------------------[4]
/* Uncomment those hardware names which need to be checked */
String hardware_to_check[] = {
    "SERIALCOM",
    "MEMORY",
    "WIFI",
    "BLUETOOTH",
    //  "I2CCOM",
    //  "SPICOM",
    //  "ETHERNET",
    // "EXTERNALMEMORY",
    //  "REALTIMECLOCK",
    //  "SIM800LS",
    "" /* do not remove this */
};
//-------------------------------------------bit position in block status bit series variables defined-----------------------[8]
 unsigned int block_status_bit_series = 0;

//--------------------------------------bit position in lock unlock bit series variable defined------------------------------[9]
 unsigned int lock_bit_series = 0;

//-------------------------------------------lock mapped list and htbt veriables---------------------------------------------[10]
std::map<int, lock_unlock_mapped_struct> lock_unlock_mapped_list = {
    {0, {0, "NONE", LOCK}},
    {1, {1, "GREEN", UNLOCK}},
    {2, {2, "YELLOW", UNLOCK}},
    {3, {3, "RED", LOCK}},
    {4, {4, "LOGS", LOGS}},
};

unsigned int htbt_bit_series = 0;
unsigned int previous_htbt_bit_series = 0;

//-------------------------------------------node application variables----------------------------------------------------------[11]
extern String node_id;
extern String project_id;
extern String subdomain;
extern String time_zone;

//-------------------------------------------queue string var to store temp log before htbt---------------------------------------[12]


//------------------------------------------- queue(buffer) for tags--------------------------------------------------------------[13]