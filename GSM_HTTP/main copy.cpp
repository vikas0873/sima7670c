#include <ModbusTW.h>
#include <ArduinoJson.h>


// ----- Modbus Protocol Definations -------
#define MODBUS_SERIAL_BAUD 115200
#define PROTOCOL SERIAL_8E1
#define SLAVE_ID_1 1
#define SLAVE_ID_2 2

// ------------------ Register Declacration -----------------------
#define READ_REG 6000
#define READ_REG_1 6001
#define READ_REG_2 6002
#define READ_REG_3 6003
#define WRITE_REG_4 6004
#define WRITE_REG_5 6005
#define WRITE_REG_6 6006

// ------------------ Write Values ---------------------------------
#define WRITE_VALUE_1 11
#define WRITE_VALUE_2 12
#define WRITE_VALUE_3 13

// JSON
String mac_id = "40:22:D8:5F:30:50";
String node_id = "E3D33748";
String project_key = "B0C357D2";
String subdomain = "vikas";
int baud_rate = 19200;
String baud_config = "SERIAL_8E1";
int delay_interval = 5000;

// Define JSON String
const String jsonConfig = "{"
                          "\"bootup\": {"
                          "\"mac_id\": \"" +
                          mac_id + "\","
                                   "\"node_id\": \"" +
                          node_id + "\","
                                    "\"project_key\": \"" +
                          project_key + "\","
                                        "\"subdomain\": \"" +
                          subdomain + "\""
                                      "},"
                                      "\"config\": {"
                                      "\"baud_rate\": " +
                          String(baud_rate) + ","
                                              "\"baud_config\": \"" +
                          baud_config + "\","
                                        "\"delay_interval\": " +
                          String(delay_interval) + ","
                                                   "\"slave_config\": ["
                                                   "{"
                                                   "\"slave_id\": \"1\","
                                                   "\"start_address\": \"6008\","
                                                   "\"read_upto\": 2,"
                                                   "\"operation\": \"read\","
                                                   "\"Write_value\": null"
                                                   "},"
                                                   "{"
                                                   "\"slave_id\": \"2\","
                                                   "\"start_address\": \"6007\","
                                                   "\"read_upto\": \"NULL\","
                                                   "\"operation\": \"write\","
                                                   "\"Write_value\": 123"
                                                   "}"
                                                   "]"
                                                   "}"
                                                   "}";

// Convert in JSON Document
DynamicJsonDocument doc(2048);
DeserializationError error = deserializeJson(doc, jsonConfig);

//
JsonObject config_object = doc["config"];
JsonArray slave_config_array = config_object["slave_config"];
int modbus_serial = config_object["baud_rate"];
uint32_t baud_config_json = config_object["baud_config"];


ModbusTW modbus;

void setup()
{
  Serial.begin(115200);
  // Serial2.begin(modbus_serial,baud_config_json);
  Serial2.begin(MODBUS_SERIAL_BAUD, PROTOCOL);
  // modbus.begin(SLAVE_ID_1, Serial2);
  // Serial.println(jsonConfig);
}

void loop()
{

  // --------------------------- iterate over JSON -----------------------------
  for (JsonObject slave : slave_config_array)
  {
    const char *slave_id = slave["slave_id"];
    const char *start_address = slave["start_address"];
    const char *operation = slave["operation"];
    int write_value = slave["Write_value"];
    int read_upto = slave["read_upto"];

    // Serial.println(slave_id);
    // Serial.println(start_address);
    // Serial.println(operation);
    // Serial.println(write_value);
    // Serial.println(read_upto);

    // begin communication with slave device
    modbus.begin(atoi(slave_id), Serial2);
    delay(200);

    // Create a JSON document for the response
    DynamicJsonDocument responseDoc(1024);
    responseDoc["mac_id"] = doc["bootup"]["mac_id"];
    JsonObject dataObject = responseDoc.createNestedObject("data");
    dataObject["slave_id"] = slave_id;
    dataObject["start_address"] = start_address;
    dataObject["operation"] = operation;

    // check read or Write operation
    if (strcmp(operation, "read") == 0)
    {
      int count = 1;
      uint16_t arr[read_upto];
      bool read_status;
      while (count <= read_upto)
      {
        // read data
        // uint16_t reg_value = atoi(start_address) + (count - 1);

        uint16_t reg_value = modbus.readRegister(atoi(start_address) + (count - 1), read_status);
        // Serial.print("read value -");
        // Serial.println(reg_value);
        // Serial.print("read status -");
        // Serial.println(read_status);
        arr[count - 1] = reg_value;
        count++;
      }

      JsonArray dataArray = dataObject.createNestedArray("data");
      for (int i = 0; i < read_upto; i++)
      {
        dataArray.add(arr[i]);
      }
      if (read_status)
      {
        dataObject["ack"] = "successful";
      }
      else
      {
        dataObject["ack"] = "unsuccessful";
      }
      // Serial.println(arr[0]);
      // Serial.println(arr[1]);
    }
    else if (strcmp(operation, "write") == 0)
    {
      // Handle write operation
      // Serial.print("Writing value ");
      // Serial.print(write_value);
      // Serial.print(" to address ");
      // Serial.println(start_address);

      bool write_success = modbus.writeRegister(atoi(start_address), write_value);
      // Serial.print("write status -");
      // Serial.println(write_success);
      dataObject["ack"] = write_success ? "successful" : "unsuccessful";
    }

    // printing post JSON 
    String responseString;
    serializeJson(responseDoc, responseString);
    Serial.println(responseString);
    delay(4000);
  }

  /*
  //------------------------------- Slave 1 -------------------------------------
  modbus.begin(SLAVE_ID_1, Serial2);
  // READING
  uint16_t reg_value = modbus.readRegister(READ_REG);
  Serial.println();
  Serial.println("Slave 1 --------------------- ");

  Serial.print("Value read from register 1: ");
  Serial.println(reg_value);

  reg_value = modbus.readRegister(READ_REG_1);
  Serial.print("Value read from register 2: ");
  Serial.println(reg_value);

  reg_value = modbus.readRegister(READ_REG_2);
  Serial.print("Value read from register 3: ");
  Serial.println(reg_value);

  reg_value = modbus.readRegister(READ_REG_3);
  Serial.print("Value read from register 4: ");
  Serial.println(reg_value);

  // WRITING
  bool write_success = modbus.writeRegister(WRITE_REG_4, WRITE_VALUE_1);
  if (write_success)
  {
    Serial.println("Write to register successful.");
  }
  else
  {
    Serial.println("Failed to write to register.");
  }

  write_success = modbus.writeRegister(WRITE_REG_5, WRITE_VALUE_2);
  if (write_success)
  {
    Serial.println("Write to register successful.");
  }
  else
  {
    Serial.println("Failed to write to register.");
  }

  write_success = modbus.writeRegister(WRITE_REG_6, WRITE_VALUE_3);
  if (write_success)
  {
    Serial.println("Write to register successful.");
  }
  else
  {
    Serial.println("Failed to write to register.");
  }

  delay(2000);

  // ----------------------------- Slave 2 --------------------------------

  modbus.begin(SLAVE_ID_2, Serial2);
  // READING
  reg_value = modbus.readRegister(READ_REG);
  Serial.println();
  Serial.println("Slave 2 --------------------- ");

  Serial.print("Value read from register 1: ");
  Serial.println(reg_value);

  reg_value = modbus.readRegister(READ_REG_1);
  Serial.print("Value read from register 2: ");
  Serial.println(reg_value);

  reg_value = modbus.readRegister(READ_REG_2);
  Serial.print("Value read from register 3: ");
  Serial.println(reg_value);

  reg_value = modbus.readRegister(READ_REG_3);
  Serial.print("Value read from register 4: ");
  Serial.println(reg_value);

  // WRITING
  write_success = modbus.writeRegister(WRITE_REG_4, WRITE_VALUE_1);
  if (write_success)
  {
    Serial.println("Write to register successful.");
  }
  else
  {
    Serial.println("Failed to write to register.");
  }

  write_success = modbus.writeRegister(WRITE_REG_5, WRITE_VALUE_2);
  if (write_success)
  {
    Serial.println("Write to register successful.");
  }
  else
  {
    Serial.println("Failed to write to register.");
  }

  write_success = modbus.writeRegister(WRITE_REG_6, WRITE_VALUE_3);
  if (write_success)
  {
    Serial.println("Write to register successful.");
  }
  else
  {
    Serial.println("Failed to write to register.");
  }

  delay(2000);


  */
}
