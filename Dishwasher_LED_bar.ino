#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

FASTLED_USING_NAMESPACE

#include "configuration.h"

// listens to dishwasher and allows a couple of colour patterns based on status
// activitates by motion, standard state: off
// allowed modes are:
// default: off
// running ; moving red light
// full : solid red
// empty : solid green

CRGB leds[NUM_LEDS];
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
String state = "off";

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

int motion, door;
int motion_prev = LOW;
int door_prev = LOW;
long last = millis();

int g_brightness=0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print (topic);
  Serial.println("");
  String command((char*)topic);
  String parameter;
  String s;
  for (int i = 0; i < length; i++) 
    {
    parameter += (char)payload[i];
    }
  state = parameter;
  Serial.print("I change state to: ");
  Serial.println(parameter) ;
}


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_NETW);
  WiFi.begin(WIFI_NETW, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect() 
  {
  // Loop until we're reconnected
  while (!client.connected()) 
    {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT)) 
      {
      Serial.print("connected as ");
      Serial.println(MQTT_CLIENT);
      // Once connected, publish an announcement...
      client.publish(MQTT_OUT, "hello world");
      // ... and resubscribe
      client.subscribe(MQTT_IN);
      } 
    else 
      {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again");
      }
    }
  }



void loop()
{
// do some periodic updates
EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
EVERY_N_SECONDS( 5 ) 
  {
  if (!client.connected())
    reconnect();
  }
EVERY_N_SECONDS( 1 ) 
  {
  door = digitalRead(DOOR_PIN);
  if (door != door_prev) //only for changes
    {
    door_prev =  door;
    Serial.print ("door is now: ");
    if (door == 0)
      {
      Serial.println ("open");
      client.publish(MQTT_OUT, "open");
      }
    else
      {
       Serial.println ("closed");
       client.publish(MQTT_OUT, "closed");
      }
    }
  }
motion = digitalRead(PIR_PIN);
if (motion != motion_prev) // only for changes
  {
  long current = millis();
  long duration = current - last;
  last = current;
  duration = duration / 1000;
  if (motion == LOW)
    { 
    motion_prev = LOW;
    Serial.print("No more motion (");
    Serial.print(duration);
    Serial.println("s)");
    g_brightness = 0;
    fill_solid(leds, NUM_LEDS, CRGB(0,0,0));
    }
  else
    { 
    motion_prev = HIGH;
    Serial.print("Motion detected (");
    Serial.print(duration);
    Serial.println("s)");
    g_brightness = 0;
    }
  }
if (motion == HIGH)
  {
  if (state == "running")
    {
    _mood_running();
    }
  else if (state == "full")
    {
    fill_solid(leds, NUM_LEDS, CRGB(255,0,0));
    }
  else if (state == "empty")
    {
    fill_solid(leds, NUM_LEDS, CRGB(0,255,0));
    }
  else 
    {
    g_brightness = 0;
    fill_solid(leds, NUM_LEDS, CRGB(0,0,0));
    }
  }
if (g_brightness <255)
  {
  EVERY_N_MILLISECONDS(10) 
    { g_brightness = g_brightness + 1;}  
  }
FastLED.setBrightness(g_brightness);

client.loop();
// send the 'leds' array out to the actual LED strip
FastLED.show();  
// insert a delay to keep the framerate modest
//FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void _mood_running()
  {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
  }

void _mood_nightlight()
{
  fill_solid(leds, NUM_LEDS, CRGB(10,10,10));
}
