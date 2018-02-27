#include <TimerOne.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 5
#define FRAMEINTERVAL 19
#define DRAWINTERVAL 20
#define ANIMATIONDURATION 6000

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(4, 9, PIN,
                            NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
                            NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                            NEO_GRB            + NEO_KHZ800);


float bbX = 1;
float bbY = 1;
float bbSX = 0.5;
float bbSY = 0.74;


enum animation {
  bouncyBall,
  rainbow,
  //  rgbSlowFade,
  //  fallingBall,
  none
};

animation currentAnimation = rainbow;

void setup() {
  Serial.begin(115200);
  matrix.begin();
  Timer1.initialize(FRAMEINTERVAL * 1000);
  Timer1.attachInterrupt(calculateDisplay);
}

unsigned long lastDraw = 0;
uint8_t readBuffer[32];
uint8_t readBufferLength = 0;

void loop() {
  if (millis() > lastDraw + DRAWINTERVAL) {
    matrix.show();
    lastDraw = millis();
  }
  while (Serial.available() > 0) {
    if (Serial.peek() == 137 || readBufferLength > 32) {
      readBufferLength = 0;
    }
    readBuffer[readBufferLength] = Serial.read();
    readBufferLength++;
  }
  if (readBufferLength >= 2 + readBuffer[1]) {
    switch (readBuffer[2]) {
      case ('A') :
        currentAnimation = none;
        matrix.fillScreen(matrix.Color(readBuffer[3], readBuffer[4], readBuffer[5]));
        break;
      case ('N') :
        switch (readBuffer[3]) {
          case ('B'):
            currentAnimation = bouncyBall;
            break;
          case ('R'):
            currentAnimation = rainbow;
            break;
        }
        break;
      case ('O') :
        currentAnimation = none;
        matrix.fillScreen(0x000000);
        break;
      case ('P'):
        currentAnimation = none;
        matrix.drawPixel(readBuffer[3], readBuffer[4], matrix.Color(readBuffer[5], readBuffer[6], readBuffer[7]));
        break;
    }
    readBufferLength = 0;
  }
}

void calculateDisplay() {
  switch (currentAnimation) {
    case bouncyBall:
      for (int x = 0; x < matrix.width(); x++) {
        for (int y = 0; y < matrix.height(); y++) {
          matrix.drawPixel(x, y,  bball(x, y));
        }
      }
      break;
    case rainbow:
      for (int x = 0; x < matrix.width(); x++) {
        for (int y = 0; y < matrix.height(); y++) {
          matrix.drawPixel(x, y,  Wheel( (int) (127 * x / 6.0 + 127 * y / 6.0 + millis() / 15.0) % 255 ));
        }
      }
      break;
    case none:
      break;
  }
}

int16_t ball(int x, int y) {
  float omega = M_2_PI / 0.08;
  //if (x == 2) {
  if (8 - y == (int) ( abs(8.0 * cos(omega * (millis() % 10000 / 1000.0)) * exp(-0.2 * (millis() % 10000 / 1000.0))))) {
    return 0xFFFFFF;
  }
  //}
  return 0x000000;
}

int16_t bball(int x, int y) {

  bbX += bbSX * (FRAMEINTERVAL / 1000.0);
  bbY += bbSY * (FRAMEINTERVAL / 1000.0);
  if (bbX > matrix.width() || bbX < 0) {
    bbSX *= -1;
  }
  if (bbY > matrix.height() || bbY < 0) {
    bbSY *= -1;
  }

  if (x == (int) bbX) {
    if (y == (int) bbY) {
      return 0xFFFFFF;
    }
  }
  return 0x000000;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
