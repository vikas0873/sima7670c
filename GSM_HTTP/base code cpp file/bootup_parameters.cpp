#include "Arduino.h"
#include <map>

//---------------------------------bootup hardware map list--------------------------------------------
extern std::map<String, int> bootup_check;

//-----------------------switch cases variable to loop through it--------------------------------------------------------
#define SERIALCOM 0
#define MEMORY 1
#define WIFI 2
#define BLUETOOTH 3
#define I2CCOM 5
#define SPICOM 6
#define ETHERNET 7
#define EXTERNALMEMORY 8
#define REALTIMECLOCK 9
#define SIM800LS 10

//-----------------------bit position in bootup bit series variables defined---------------------------------------------
#define SRC 0 // serial flag
#define MEC 1 // memory flag
#define WFC 2 // wifi flag
#define BLC 3 // bluetooth flag
#define APC 4 //hotspot flag
#define ICC 5 // i2c com flag
#define SPC 6 //spi com flag
#define ETC 7 //ethernet flag
#define EXC 8 //external memory flag
#define CKC 9 //rtc flag
#define GSC 10 //sim800l flag

extern String bootup_json_var;
extern unsigned int boot_up_bit_series;

//------------------------variable to store number of elements in hardware check list------------------------------------

std::map<String, int> bootup_check = {
  {"SERIALCOM", 0},
  {"MEMORY", 1},
  {"WIFI", 2},
  {"BLUETOOTH", 3},
  {"I2CCOM", 5},
  {"SPICOM", 6},
  {"ETHERNET", 7},
  {"EXTERNALMEMORY", 8},
  {"REALTIMECLOCK", 9},
  {"SIM800LS", 10}
};

//-----------------------bit position in bootup bit series variables defined----------------------------------------------
String bootup_json_var;
unsigned int boot_up_bit_series = 0;