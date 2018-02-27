#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ESP8266WiFi.h>

// WiFi settings
#define WLAN_SSID       "JIoT"
#define WLAN_PASS       "uHgARQ8AY3Dm39ay"
WiFiClient client;

// MQTT settings
#define AIO_SERVER      "odroid.lan"
#define AIO_SERVERPORT  1883

uint8_t currentR;
uint8_t currentG;
uint8_t currentB;

// MQTT
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT);
Adafruit_MQTT_Subscribe red = Adafruit_MQTT_Subscribe(&mqtt, "/lights/mateLight/all/red");
Adafruit_MQTT_Subscribe green = Adafruit_MQTT_Subscribe(&mqtt, "/lights/mateLight/all/green");
Adafruit_MQTT_Subscribe blue = Adafruit_MQTT_Subscribe(&mqtt, "/lights/mateLight/all/blue");

Adafruit_MQTT_Subscribe redA = Adafruit_MQTT_Subscribe(&mqtt, "/lights/red");
Adafruit_MQTT_Subscribe greenA = Adafruit_MQTT_Subscribe(&mqtt, "/lights/green");
Adafruit_MQTT_Subscribe blueA = Adafruit_MQTT_Subscribe(&mqtt, "/lights/blue");

Adafruit_MQTT_Subscribe pixel = Adafruit_MQTT_Subscribe(&mqtt, "/lights/mateLight/pixel");
Adafruit_MQTT_Subscribe animation = Adafruit_MQTT_Subscribe(&mqtt, "/lights/mateLight/animation");


void redCallback(uint32_t r) {
  if (r < 256) {
    currentR = r;
    sendAll();
  }
}

void greenCallback(uint32_t g) {
  if (g < 256) {
    currentG = g;
    sendAll();
  }
}

void blueCallback(uint32_t b) {
  if (b < 256) {
    currentB = b;
    sendAll();
  }
}

void pixelCallback(char *data, uint16_t len) {
  sendPixel(data[0], data[1], data[2], data[3], data[4]);
}

void animationCallback(char *data, uint16_t len) {
  if (strcmp(data, "rainbow") == 0) {
    sendAnimation('R');
  }
  if (strcmp(data, "ball") == 0) {
    sendAnimation('B');
  }
}

void sendAnimation(byte a) {
  sendHeader(2);
  Serial.write('N');
  Serial.write(a);
}

void sendAll() {
  sendHeader(4);
  Serial.write('A');        //"SET ALL"
  Serial.write(currentR);   //RED
  Serial.write(currentG);   //GREEN
  Serial.write(currentB);   //BLUE
}

void sendPixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
  sendHeader(6);
  Serial.write('P');
  Serial.write(x);
  Serial.write(y);
  Serial.write(r);
  Serial.write(g);
  Serial.write(b);
}

void sendHeader(byte dataBytesCount) {
  Serial.write(137);            //STARTBYTE
  Serial.write(dataBytesCount);
}

void setup() {
  Serial.begin(115200);

  WiFi.hostname("ESP-MateLight");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  red.setCallback(redCallback);
  green.setCallback(greenCallback);
  blue.setCallback(blueCallback);

  redA.setCallback(redCallback);
  greenA.setCallback(greenCallback);
  blueA.setCallback(blueCallback);

  pixel.setCallback(pixelCallback);

  animation.setCallback(animationCallback);

  mqtt.subscribe(&red);
  mqtt.subscribe(&green);
  mqtt.subscribe(&blue);

  mqtt.subscribe(&redA);
  mqtt.subscribe(&greenA);
  mqtt.subscribe(&blueA);

  mqtt.subscribe(&pixel);
  mqtt.subscribe(&animation);
}

void loop() {
  MQTT_connect();
  mqtt.processPackets(1000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    mqtt.disconnect();
    delay(10000);  // wait 10 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
}
