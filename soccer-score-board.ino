#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

// #define USE_SERIAL

#define PIN D5

volatile int goalFlag = 0;
int goalCounter = 0, prevCounter = -1;
uint8_t helloRobotois = 1;

char* ssid = "robotoisAP";
char* password = "robotois8899";
char* brokerAdd = "192.168.50.27";

char* clientId = "white-score";
String boardColor = "white";
String driveTopic = "score-boards/" + String(boardColor);
String goalAction = String("goal");
String startAction = String("start");
String endAction = String("end");

WiFiClient espClient;
PubSubClient client(espClient);

// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_GRBW    Pixels are wired for GRBW bitstream (RGB+W NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)


// Example for NeoPixel Shield.  In this application we'd like to use it
// as a 5x8 tall matrix, with the USB port positioned at the top of the
// Arduino.  When held that way, the first pixel is at the top right, and
// lines are arranged in columns, progressive order.  The shield uses
// 800 KHz (v2) pixels that expect GRB color data.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(128, PIN, NEO_GRB + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };

const uint16_t greenColor = matrix.Color(76, 175, 80);
const uint16_t redColor = matrix.Color(255,87,34);
const uint16_t yellowColor = matrix.Color(255, 193, 7);
const uint16_t whiteColor = matrix.Color(240,244,195);
const uint16_t purpleColor = matrix.Color(156,39,176);
uint16_t primaryColor;

void setupWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    #ifdef USE_SERIAL
    Serial.println("Connecting to WiFi..");
    #endif
    delay(1000);
  }
  #ifdef USE_SERIAL
  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());
  #endif
  randomSeed(micros());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    #ifdef USE_SERIAL
    //Serial.print("Attempting MQTT connection...");
    #endif
    // Attempt to connect
    if (client.connect(clientId)) {
      #ifdef USE_SERIAL
      Serial.println("connected");
      #endif
      // Once connected, publish an announcement...
      // client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(driveTopic.c_str());
    } else {
      #ifdef USE_SERIAL
      Serial.print("failed, rc=");
      Serial.println(client.state());
      #endif
      // //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}

void mqttLoop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void messageProcessor(char* topic, byte* payload, unsigned int length) {
  #ifdef USE_SERIAL
  String msg = String((char*)payload);
  Serial.println(msg);
  #endif

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);

  if(!root.success()) {
    return;
  }
  const char* action = root["action"];
  String actionStr = String(action);
  if(actionStr == goalAction){
    goalFlag = root["increment"];
  }
  if(actionStr == startAction || actionStr == endAction) {
    goalFlag = 0;
    goalCounter = 0;
    prevCounter = -1;
    helloRobotois = 1;
  }
}

void setup() {
  #ifdef USE_SERIAL
  Serial.begin(115200);
  Serial.println("SoccerBoard");
  Serial.setDebugOutput(true);
  #endif

  setupWifi();
  client.setServer(brokerAdd, 1883);
  client.setCallback(messageProcessor);

  if(boardColor == "yellow") {
    primaryColor = yellowColor;
  }
  if(boardColor == "white") {
    primaryColor = whiteColor;
  }

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(16);
  matrix.setTextColor(primaryColor);
  strip.setBrightness(16);
  delay(2000);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void theaterChaseRainbow(uint8_t wait = 30) {
  for (int j=0; j < 256; j+=15) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void robotois(const char* text, uint16_t textColor = purpleColor) {
  int pass = 0;
  int x = matrix.width();
  while(pass < 1) {
    matrix.setTextColor(textColor);
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.print(text);
    if(--x < -100) {
      x = matrix.width();
      pass ++;
    }
    matrix.show();
    delay(75);
  }
}

void celebrate(const char* text, uint16_t textColor = redColor) {
  int pass = 0;
  int x = matrix.width();
  theaterChaseRainbow();
  while(pass < 1) {
    matrix.setTextColor(textColor);
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.print(text);
    if(--x < -36) {
      x = matrix.width();
      pass ++;
    }
    matrix.show();
    delay(75);
  }
}

void goalChange() {
  if (goalFlag == 1) {
    celebrate("GOOL!");
    goalCounter++;
  }
  if (goalFlag == -1 && goalCounter > 0) {
    goalCounter--;
  }
  goalFlag = 0;
}

void goalPrint(uint16_t textColor = primaryColor) {
  String countStr;
  if (goalCounter < 10) {
    countStr = "0" + String(goalCounter, DEC);
  } else {
    countStr = String(goalCounter, DEC);
  }
  matrix.setTextColor(textColor);
  matrix.fillScreen(0);
  matrix.setCursor(3, 0);
  matrix.print(countStr.c_str());
  matrix.show();
}

void loop() {
  mqttLoop();
  if (helloRobotois == 1) {
    robotois("www.robotois.com");
    helloRobotois = 0;
  }
  if (goalFlag != 0) {
    goalChange();
  }
  if (prevCounter != goalCounter) {
    goalPrint();
    prevCounter = goalCounter;
  }
}
