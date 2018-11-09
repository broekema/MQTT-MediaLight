/*
    MQTT MediaLight: A simple ESP8266 based RGB light controller. Subscribes
    to three MQTT subjects, one for each color, and outputs an analog signal
    on three pins to drive an RGB light strip.

    Dependencies:
    - PubSubClient (https://github.com/knolleary/pubsubclient)
    - ArduinoOTA   (https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA)

    Copyright 2018 Chris Broekema

    MQTT MediaLight  is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    kwh-meter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

#define VERSION "0.1"

#define GREEN_PIN D0
#define RED_PIN   D1
#define BLUE_PIN  D2

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define OTA_HOSTNAME "MEDIALIGHT"
#define OTA_PASSWORD "PASSWORD"

// MQTT server
#define MQTT_SERVER ""
#define MQTT_PORT   1883


#define MQTT_ROOT   "Home/"
#define MQTT_DEVICE "MediaLight/"
#define MQTT_WILL_TOPIC MQTT_ROOT MQTT_DEVICE "LWT"
#define MQTT_WILL_RETAIN true
#define MQTT_WILL_QOS 0
#define MQTT_WILL_MESSAGE "Offline"
#define MQTT_GATEWAY_ANNOUNCEMENT "Online"
#define MQTT_VERSION_TOPIC MQTT_ROOT MQTT_DEVICE "version"
                 
#define RED_TOPIC MQTT_ROOT MQTT_DEVICE "RED"
#define GREEN_TOPIC MQTT_ROOT MQTT_DEVICE "GREEN"
#define BLUE_TOPIC MQTT_ROOT MQTT_DEVICE "BLUE"

WiFiClient eClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, NULL, eClient);

void setupWifi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup_OTA() {
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() { Serial.println("Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("End");});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
#ifdef DEBUG
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
#endif
    });
#ifdef DEBUG
  ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
      else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
      else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
      else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
      else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
    });
#endif
  ArduinoOTA.begin();
}

boolean reconnect_mqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if (client.connect(MQTT_DEVICE, MQTT_WILL_TOPIC, MQTT_WILL_QOS, MQTT_WILL_RETAIN, MQTT_WILL_MESSAGE)) {
#ifdef DEBUG
	Serial.println("Connected to broker ");
#endif
	client.publish(MQTT_WILL_TOPIC, MQTT_GATEWAY_ANNOUNCEMENT, MQTT_WILL_RETAIN);
	client.publish(MQTT_VERSION_TOPIC, VERSION, MQTT_WILL_RETAIN);
      } else {
#ifdef DEBUG
	Serial.println("failed, rc=" + String(client.state()));
	Serial.println("try again in 5s");
#endif

    //  Wait 5 seconds before retrying
	  delay(5000);
    }
  }
	
  // subscribe to the topics we're interested in
  client.subscribe(RED_TOPIC);
  client.subscribe(GREEN_TOPIC);
  client.subscribe(BLUE_TOPIC);
  
#ifdef DEBUG
  Serial.print("Subscribed to "); Serial.println( RED_TOPIC);
  Serial.print("Subscribed to "); Serial.println( GREEN_TOPIC);
  Serial.print("Subscribed to "); Serial.println( BLUE_TOPIC);
#endif
  return client.connected();
}

void callback(char* topic, byte* payload, unsigned int length) {
#ifdef DEBUG
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
#endif  
  long value=0;
  for (unsigned int i=0;i<length;i++) {
#ifdef DEBUG
    Serial.print((char)payload[i]);
#endif
    value = value * 10;
    value = value + ((char)payload[i] - (char)'0');
  }

  // we expect values ranging 0..100; schale to ESP8266 default range (10 bit, 0..1023)
  value = value * 1023 / 100;

  if (strcmp(topic, RED_TOPIC) == 0){
    analogWrite(RED_PIN, value);
  } else if (strcmp(topic, GREEN_TOPIC) == 0) {
    analogWrite(GREEN_PIN, value);
  } else if (strcmp(topic, BLUE_TOPIC) == 0) {
    analogWrite(BLUE_PIN, value);
  }
}

void setup() {
  // put your setup code here, to run once:
#ifdef DEBUG
  Serial.begin(9600);
#endif
  setupWifi();
  setup_OTA();
  
  reconnect_mqtt();
  client.setCallback(callback);

  Serial.println("Setup complete");
}

void loop() {
   if (!client.connected()) {
    reconnect_mqtt() ;
  }
  client.loop();         // MQTT
  ArduinoOTA.handle();   // OTA
}