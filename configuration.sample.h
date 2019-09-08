// setup your LED strip (refer to FASTLED for details)
#define DATA_PIN    D3 
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB //RGB
#define NUM_LEDS    27 
#define BRIGHTNESS          255 // 0-255
#define FRAMES_PER_SECOND   120 //remove?

// setup WIFI network
#define WIFI_NETW "secret_wifi"
#define WIFI_PASS "secret_pass"

//define MQTT details
#define MQTT_SERVER "192.168.1.1"
#define MQTT_CLIENT "LED_STRIP_1"
#define MQTT_IN "Room/Device/state"
#define MQTT_OUT "Room/Device/door"

#define PIR_PIN   D1

#define DOOR_PIN D8
