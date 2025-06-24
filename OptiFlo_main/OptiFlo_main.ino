//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//

#include <Wire.h>
#include <climits>

/* camera sensor address as per the manufacturer */
#define OV7670_ADDR 0x42

/* Pin definitions for the OV7670 camera */
#define VSYNC_PIN 4
#define HREF_PIN 5
#define PCLK_PIN 6
#define XCLK_PIN 3

#define D7_PIN 20
#define D6_PIN 23
#define D5_PIN 22
#define D4_PIN 16
#define D3_PIN 17
#define D2_PIN 15
#define D1_PIN 14
#define D0_PIN 0

#define RESET 21

/* Variable definitions */
volatile bool frameAvailable = false;
volatile bool frameCapture = false;
bool firstFrame = true;

/* Image resolution parameters */
const int CameraWidth = 160; // Camera capture width
const int CameraHeight = 120; // Camera capture height
const int windoWidth = 50; // SSD-fed image width
const int windoHeight = 50; // SSD-fed image height
float realFactorX = 1;
float realFactorY = 1;

/* defining the cropping variables constant to increase head room */
const int startRow = (CameraHeight - windoHeight) / 2;
const int startCol = (CameraWidth - windoWidth) / 2;

byte imgF[windoWidth*windoHeight];
byte imgFprev[windoWidth*windoHeight];

int dx = 0, dy = 0;
int cnt = 0;

/* variable to initiate global time keeping and relative FPS calculation */
unsigned long time0;

/* Mode Definition */
// const String MODE = "WIRED_DATA";
// const String MODE = "WIRED_VISUAL";
const String MODE = "WIRELESS";
// const String MODE = "WIRELESS_DATA";
// const String MODE = "WIRED_IMG";

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

struct RadioPacket
{
    float dx;
    float dy;
};

RadioPacket r1;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//

#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int distance = 33;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//

void setup() {
  if(MODE == "WIRED_VISUAL") Serial.begin(115200);
  else Serial.begin(4000000);
  Wire.begin();
  delay(100);

  RadioSetup();
  CameraSetupBasic();
  CameraSetupSCCB();
  CameraSetupInterrupt();
  vlSetup();

  r1.dx = 0;
  r1.dy = 0;
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//

void loop() {

  /* first time-stamp, before image capture */
  time0 = micros();

  /* Main Loop only calls functions from device specific files */
  vlMeasure();
  CaptureCalculate();
  
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//

