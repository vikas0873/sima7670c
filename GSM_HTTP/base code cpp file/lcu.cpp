#include <Arduino.h>
#include <SPIFFS.h>
#include <deque>
#include <flageManager.cpp>
#include <tw_timer.cpp>
#include <utils.cpp>
#include <WiFiClientSecure.h>

/* log levels */
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5

/* switch cases */
#define LCU_CHECK_FILE_SIZE 0
#define LCU_CHECK_FLAGS 1
#define LCU_SEND_DATA_TO_SERVER 2
#define LCU_EXIT 3

#define LCU_HTTP_TIMEOUT 20000

#define FILE_HEADER_NAME "file"
#define LOG_FILE_NAME "log.txt"
#define MAX_FILE_SIZE 20000        /* maximum size of file up to which logs will be added to tog.txt after reaching this size file will get reset. */
#define LOG_UPLOAD_THRESHOLD 15000 /* max file size up to which node will not send log to server. after reaching this limit node will upload log file to server */
#define LCU_CHUNK_SIZE 1024

#define PRINT_BUFFER_SIZE 300

#define LOG_SERVER_PATH "/lcu/upload"
#define HTTP_OK 201

class Lcu
{

public:
  FlagManager flagManager;
  TwTimer twTimer;
  Utils utils;

  bool logs(String message, int level, bool print_content = false)
  {

    if (!(level >= LOG_LEVEL))
    { // LOG_LEVEL is defined in global_variable.h
      return false;
    }

    unsigned long current_millis = millis();
    String time_String = twTimer.getFormattedTimeString();
    int timer_working_status = flagManager.getFlagStatus(block_status_bit_series, INTERNAL_TIMER, 0);
    int timer_internet_accessed_status = flagManager.getFlagStatus(block_status_bit_series, TIMER_FUNCTION_ACCESSED, 0);

    int log_in_file = flagManager.getFlagStatus(lock_bit_series, LOGS, 0);

    if (!timer_working_status)
    {
      message = "|" + String(current_millis) + "|" + level + "|" + message;
    }
    else
    {
      message = "|" + String(time_String) + "|" + String(current_millis) + "|" + level + "|" + message;
    }

    if (log_in_file)
    {
      char buffer[PRINT_BUFFER_SIZE];
      File file_read = SPIFFS.open("/" + String(LOG_FILE_NAME));
      size_t file_size = file_read.size();
      Serial.println("LOG FILE SIZE: " + String(file_size)); // temporary print for testing
      file_read.close();

      if (file_size >= MAX_FILE_SIZE) // MAX_FILE_SIZE is defined in lcu.h file
      {
        File file_write = SPIFFS.open("/" + String(LOG_FILE_NAME), "w");
        file_write.close();
      }

      File log_file = SPIFFS.open("/" + String(LOG_FILE_NAME), "a+");
      if (!log_file)
      {
        log_file.close();
        return false;
      }

      if (!timer_internet_accessed_status)
      {
        log_strings.push_back(message);
      }
      else
      {
        while (!log_strings.empty())
        {
          String str = log_strings.front();
          if (timer_working_status)
          {
            str = "[" + String(time_String) + "]" + str;
          }
          log_strings.pop_front();
          str.toCharArray(buffer, PRINT_BUFFER_SIZE);
          log_file.println(buffer);
          str = "";
        }
        message.toCharArray(buffer, PRINT_BUFFER_SIZE);
        log_file.println(buffer);
      }
      log_file.close();
    }

    if (print_content)
    {
      Serial.println(message);
    }

    return true;
  };
  void logMain()
  {

    unsigned long current_millis = millis();
    static unsigned long lcu_previous_millis = 0;
    bool loop_complete = false;

    while (!loop_complete)
    {
      switch (lcu_flow)
      {
      case LCU_CHECK_FILE_SIZE:
      {
        if (current_millis - lcu_previous_millis >= LCU_INTERVAL)
        {
          lcu_previous_millis = current_millis;

          File log_file = SPIFFS.open("/" + String(LOG_FILE_NAME));
          if (log_file)
          {
            size_t file_size = log_file.size();
            log_file.close();
            if (file_size >= LOG_UPLOAD_THRESHOLD) // MAX_FILE_SIZE is defined in lcu.h file
            {
              lcu_flow = LCU_CHECK_FLAGS;
            }
          }
        }
        else
        {
          loop_complete = true;
        }
        break;
      }

      case LCU_CHECK_FLAGS:
      {
        int wncf_status = flagManager.getFlagStatus(block_status_bit_series, WNCF, 0);
        if (wncf_status == 1)
        {
          //--------------------------------------------LOGGING--------------------------------------------------------------------------------
          logs("LCU|logMain|LCU WNCF WORKING", INFO, true);
          //--------------------------------------------LOGGING--------------------------------------------------------------------------------
          lcu_flow = LCU_SEND_DATA_TO_SERVER;
        }
        else
        {
          //--------------------------------------------LOGGING--------------------------------------------------------------------------------
          logs("LCU|logMain|LCU WNCF NOT WORKING", WARN, true);
          //--------------------------------------------LOGGING--------------------------------------------------------------------------------
          lcu_flow = LCU_EXIT;
        }
        break;
      }

      case LCU_SEND_DATA_TO_SERVER:
      {
        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
        logs("LCU|logMain|LCU SENDING FILE TO SERVER", INFO, true);
        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
        String lcu_payload;
        int lcu_http_code;
        int server_status = sendLogFileToServer(SERVER_HOST, LOG_SERVER_PATH, SERVER_PORT, LOG_FILE_NAME, FILE_HEADER_NAME, &lcu_payload, &lcu_http_code);
        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
        logs("LCU|logMain|LCU SERVER HTTP CODE: " + String(lcu_http_code), INFO, true);
        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
        if (server_status == 1)
        {
          if (lcu_http_code == HTTP_OK)
          {
            File log_file = SPIFFS.open("/" + String(LOG_FILE_NAME), "w");
            log_file.close();
          }
        }
        lcu_flow = LCU_EXIT;
        break;
      }

      case LCU_EXIT:
      {
        lcu_flow = LCU_CHECK_FILE_SIZE;
        float lcu_execution_time = (millis() - current_millis) / 1000.0;
        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
        logs("LCU|logMain|LCU EXECUTION TIME: " + String(lcu_execution_time) + " SECONDS", INFO, true);
        //--------------------------------------------LOGGING--------------------------------------------------------------------------------
        loop_complete = true;
        break;
      }
      }
    }
  };

private:
  int sendLogFileToServer(String host, String path, const int port, String file_name, String file_header_name, String *payload, int *httpCode)
  {

    String macAddr = utils.getMacAddress();
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(LCU_HTTP_TIMEOUT);

    if (client.connect(host.c_str(), port))
    {
      // Create the boundary string
      String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

      File file = SPIFFS.open("/" + file_name, "r");
      if (!file)
      {
        file.close();
        return -1;
      }
      // Calculate the content length
      size_t contentLength = file.size() + 196; // Adjust the value as needed

      // Send the HTTP POST request header
      client.println("POST " + path + " HTTP/1.1");
      client.println("Host: " + host);
      client.println("macid: " + macAddr);
      client.print("Content-Length: ");
      client.println(contentLength);
      client.println("Content-Type: multipart/form-data; boundary=" + boundary);
      client.println();

      // Send the form data headers
      client.println("--" + boundary);
      client.println("Content-Disposition: form-data; name=\"" + file_header_name + "\"; filename=\"/" + file_name + "\"");
      client.println("Content-Type: <Content-Type header here>");
      client.println();

      // Send file content in chunks
      uint8_t buffer[LCU_CHUNK_SIZE];
      int bytesRead;

      while ((bytesRead = file.read(buffer, LCU_CHUNK_SIZE)) > 0)
      {
        client.write(buffer, bytesRead);
      }

      // End the form data
      client.println();
      client.println("--" + boundary + "--");

      bool headersReceived = false;
      unsigned long client_loop_break = millis();

      while (client.connected())
      {
        String line = client.readStringUntil('\n');
        if (line.startsWith("HTTP/1.1"))
        {
          int status_code = line.substring(9, 12).toInt();
          *httpCode = status_code;
          if (millis() - client_loop_break > CLIENTTIMEOUT)
          {
            break;
          }
        }

        String line_payload = client.readStringUntil('\r');
        if (line_payload == "\n")
        {
          headersReceived = true;
        }
        else if (headersReceived)
        {
          *payload += line_payload;
        }
      }
      file.close();
      client.stop();
    }
    else
    {
      return 0;
    }
    return 1;
  };

  int lcu_flow = LCU_CHECK_FLAGS;
};