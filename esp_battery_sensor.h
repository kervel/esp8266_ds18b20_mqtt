// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _esp_battery_sensor_H_
#define _esp_battery_sensor_H_
#include "Arduino.h"
//add your includes for the project esp_battery_sensor here
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "creds.h"

/* make a file creds.h with the following contents:
#define wifi_ssid "mywifi"
#define wifi_password "mysecret"
#define mqtt_server "192.168.0.1"
#define mqtt_username NULL
#define mqtt_password NULL
*/
#define mqtt_topic_temp "/esp8266_1/temperature"
#define mqtt_topic_volt "/esp8266_1/voltage"
#define SLEEP_DELAY_IN_SECONDS 1800
//end of add your includes here


//add your function definitions for the project esp_battery_sensor here




//Do not add code below this line
#endif /* _esp_battery_sensor_H_ */
