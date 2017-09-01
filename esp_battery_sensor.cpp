// Do not remove the include below
#include "esp_battery_sensor.h"

#include <string>
//#include "OneWire.h"
#include "DallasTemperature.h"


#define ONE_WIRE_BUS 13  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


WiFiClient espClient;
PubSubClient client(espClient);

unsigned long bootTime;
DeviceAddress devices[10];
int sensorsFound = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


//The setup function is called once at startup of the sketch
void setup()
{
	bootTime = millis();
	Serial.begin(115200);
	//pinMode(13, OUTPUT);
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
	// Initialize the LED_BUILTIN pin as an output// Add your initialization code here
}



bool setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  // static ip
  WiFi.config(
		  	 IPAddress(192,168,0,233),
			 IPAddress(192,168,0,1),
			 IPAddress(255,255,255,0),
			 IPAddress(8,8,8,8)
			 );

  WiFi.begin(wifi_ssid, wifi_password);
  unsigned long s = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
	if ((millis() > (s + 20000)) || (millis() < s)) {
		Serial.println("Giving up on WIFI");
		return false;
	}
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}


bool connect_mqtt() {
  // Loop until we're reconnected
  unsigned long s = millis();
  while (!client.connected()) {
	if ((millis() > (s + 20000)) || (millis() < s)) {
		Serial.println("Giving up on MQTT connection");
		return false;
	}
	Serial.print("Attempting MQTT connection...");
	// Attempt to connect
	if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
	  Serial.println("connected");
	} else {
	  Serial.print("failed, rc=");
	  Serial.print(client.state());
	  Serial.println(" try again in 5 seconds");
	  // Wait 5 seconds before retrying
	  delay(5000);
	}
  }
  return true;
}

float getTemperature(DeviceAddress address) {
	float temp;
	unsigned long s = millis();

	int xtry = 0;

	do {
		xtry += 1;
		if (xtry % 3 == 0) {
			DS18B20.requestTemperatures();
		}
		temp = DS18B20.getTempC(address);
		if ((millis() > (s + 1000)) || (millis() < s)) {
			Serial.print("timeout temp");
			return -127.0;
		}

	} while (((temp > 84.0) && (temp < 86.0)) || temp < (-126.0));
	return temp;
}


void goToSleep() {
	delay(100);
	Serial.print("Entering deep sleep mode for ");
	Serial.print(SLEEP_DELAY_IN_SECONDS);
	Serial.print(" seconds after ");
	Serial.print((millis() - bootTime)/1000);
	Serial.println("seconds");
	ESP.deepSleep(SLEEP_DELAY_IN_SECONDS * 1000000, WAKE_RF_DEFAULT);
	delay(500);
}

void setup_ds18b20() {
	DS18B20.begin();
	sensorsFound =  DS18B20.getDeviceCount();

	for (int i = 0; i < sensorsFound; i++)
		if (!DS18B20.getAddress(devices[i], i))
			Serial.println("Unable to find address for Device" + i);

	for (int i = 0; i < sensorsFound; i++)
		DS18B20.setResolution(devices[i], 12);

    DS18B20.setWaitForConversion(true);

}


// The loop function is called in an endless loop
void loop()
{
	delay(100);
	float volts = 1.0 * analogRead(A0) / 1024; // 1024 = 1v
	float v_in = volts * (2000 + 330) / 330; // voltage divider: vin --> 2Mohm -->  A0 --> 330Kohm --> gnd
	Serial.print("battery voltage: v_in ");
	Serial.print(v_in);
	Serial.print(" V (adc: ");
	Serial.print(volts);
	Serial.println(")");


	DS18B20.requestTemperatures();

	if (!setup_wifi()) {
		goToSleep();
		return;
	}

	if (!connect_mqtt()) {
		goToSleep();
		return;
	}

	for (int i =0; i < sensorsFound; i++) {
		float temp = getTemperature(devices[i]);
		String s = String(mqtt_topic_temp) + "/";
		for (unsigned int c=0; c<sizeof(devices[i]);c++) {
			s += String(devices[i][c],HEX);
		}
		String value = String(temp);
		client.publish(s.c_str(), value.c_str());
		Serial.print(s + " " + temp + " °C");
	}


	client.publish(mqtt_topic_volt, String(v_in).c_str());

	client.disconnect();

	WiFi.disconnect();

	goToSleep();
}
