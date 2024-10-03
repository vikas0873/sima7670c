#include <Arduino.h>
/* pin discription for LEDBlink class */
#define WIFILEDREDPIN 2
#define WIFILEDGREENPIN 4
#define CLOUDLEDREDPIN 14
#define CLOUDLEDGREENPIN 12
#define CLOUD_LED_BLINK_INTERVAL 500 /* Interval at which to blink (milliseconds) */

// class for all led setup
class LedBlink
{
public:
  void ledSetup()
  { // function to setup leds mode
    pinMode(WIFILEDREDPIN, OUTPUT);
    pinMode(WIFILEDGREENPIN, OUTPUT);
    pinMode(CLOUDLEDREDPIN, OUTPUT);
    pinMode(CLOUDLEDGREENPIN, OUTPUT);
  }

  void WifiLedPin(bool flag = false)
  { // Setup Wifi pin if connected then green else red
    if (flag)
    {
      digitalWrite(WIFILEDREDPIN, LOW);
      digitalWrite(WIFILEDGREENPIN, HIGH);
    }
    else
    {
      digitalWrite(WIFILEDREDPIN, HIGH);
      digitalWrite(WIFILEDGREENPIN, LOW);
    }
  }

  void CloudLedPin(bool flag = false)
  { // Setup clode pin if connected then green else red
    if (flag)
    {
      digitalWrite(CLOUDLEDREDPIN, LOW);
      digitalWrite(CLOUDLEDGREENPIN, HIGH);
    }
    else
    {
      digitalWrite(CLOUDLEDREDPIN, HIGH);
      digitalWrite(CLOUDLEDGREENPIN, LOW);
    }
  }

  void lockModeBlink()
  {
    digitalWrite(CLOUDLEDREDPIN, LOW);
    unsigned long current_millis = millis(); // Get the current time

    if (current_millis - cloud_led_previous_millis >= CLOUD_LED_BLINK_INTERVAL)
    {
      cloud_led_previous_millis = current_millis; /* Save the last time LED was updated */
      ledState = !ledState;                       /* Toggle the LED state */

      digitalWrite(CLOUDLEDGREENPIN, ledState); /* Update the LED with the new state */
    }
  }

  void safeModeBlink()
  {
    digitalWrite(CLOUDLEDGREENPIN, LOW);
    unsigned long current_millis = millis(); // Get the current time

    if (current_millis - cloud_led_previous_millis >= CLOUD_LED_BLINK_INTERVAL)
    {
      cloud_led_previous_millis = current_millis; /* Save the last time LED was updated */
      ledState = !ledState;                       /* Toggle the LED state */

      digitalWrite(CLOUDLEDREDPIN, ledState); /* Update the LED with the new state */
    }
  }

private:
  unsigned long cloud_led_previous_millis = 0; /* Variable to store the last time LED was updated */
  bool ledState = false;                       /* Initial state of the LED */
};