/*
  ESP8266 MQTT FastLED Client
  Sets LED Color with MQTT
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>
#include "MySecret.h"

#define DATA_PIN    2
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    20
CRGB leds[NUM_LEDS];
#define BRIGHTNESS          64
#define FRAMES_PER_SECOND  120

// create a new MySecret.h file with  following content:
// -cut------------------------------------------>
// #define MY_WLAN_SSID "MyWLAN_SSID"             // SSID
// #define MY_WLAN_PASS "my_very_secret_password" // Password
// #define MY_MQTT_SERVER "my_mqtt_server_ip      // MQTT Server IP
// <-cut------------------------------------------
// (and you do not check them into the repository!)

const char *ssid =  MY_WLAN_SSID; // cannot be longer than 32 characters!
const char *password =  MY_WLAN_PASS; //
const char *mqtt_server =  MY_MQTT_SERVER; //


// Update these with values suitable for your network.

const char* inTopic = "cmnd/FastLedDisplay/rbw";
const char* outTopic = "out_fastled_display_abfall";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char message_buff[100];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  int i = 0;
  // create character buffer with ending null terminator (string)
  for (i = 0; i < length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);
  Serial.println("Message:" + msgString);
  changeColor(msgString);
}

void changeColor(String rgbString)
{
  //Parse RGB;
  int lIndex = rgbString.indexOf(',');
  int rIndex = rgbString.indexOf(',', lIndex + 1);
  int gIndex = rgbString.indexOf(',', rIndex + 1);

  int l = rgbString.substring(0, lIndex).toInt();
  int r = rgbString.substring(lIndex + 1, rIndex).toInt();
  int g = rgbString.substring(rIndex + 1, gIndex).toInt();
  int b = rgbString.substring(gIndex + 1).toInt();

  leds[l] = CRGB( r, g, b);
  FastLED.show();
  /*
  Serial.print("Set LED ");
  Serial.print(l);
  Serial.print("to r=");
  Serial.print(r);
  Serial.print(" g=");
  Serial.print(g);
  Serial.print(" b=");
  Serial.println(b);
  */
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "FastLED");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  resetLEDs();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void resetLEDs()
{
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  /*
    for(int i=0 ; i<NUM_LEDS; i++)
    {
    leds[i] = CRGB( 0, 0, 0);
    }
  */
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}
