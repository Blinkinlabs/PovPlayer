#include <SPI.h>
#include <SD.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include "MBI6020.h"
#include "APA102.h"
#include "ICND2110.h"
#include "LED_DATA.h"

#include "testpattern.h"

//*************************************************
// POV Player
//
// Designed for use with a Teensy 3.6
//
// Default configuration:

#define SPI_CLOCK_FREQUENCY_APA102    16000000
#define SPI_CLOCK_FREQUENCY_MBI6020   10000000
#define SPI_CLOCK_FREQUENCY_ICND2110  1000000

const unsigned int MAX_LED_COUNT = 500;
unsigned int ledCount = 240;
int fps = 1100;
int spiClockFrequency;
const char fileName[] = "POV.BMP";

//*************************************************
// Pin connections

// LED output pins (hardware SPI 1)
#define LED_MOSI 0
#define LED_SCK 32

// LCD and TS pins (hardware SPI 0)
#define TFT_MOSI 11
#define TFT_MISO 12
#define TFT_SCK 13

#define TFT_RESET 2
#define TFT_DC 3
#define TFT_CS 9

#define TOUCH_CS  10
#define TOUCH_IRQ 24

// Debug pin
#define FRAME_TIMING 1
//*************************************************

#define LOADBMP_IMPLEMENTATION
#include "readbmp.h"

#define PROTOCOL_MBI6020 0
#define PROTOCOL_APA102 1
#define PROTOCOL_ICND2110 2
int protocol = PROTOCOL_ICND2110;

struct Button {
  String text;
  int x;
  int y;
};

Button speedPlus  = {"Speed +", 10, 100};
Button speedMinus = {"Speed -", 10, 135};
Button apa102     = {"APA102",  10, 170};
Button mbi6020    = {"MBI6020", 10, 205};
Button clockPlus  = {"Clock +", 10, 240};
Button clockMinus = {"Clock -", 10, 275};

// Decoded bitmap geometry
unsigned char *imageData;
unsigned int imageWidth = 0;
unsigned int imageHeight = 0;

unsigned int frameDelay;

//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC,TFT_MOSI, TFT_SCK, TFT_RESET, TFT_MISO);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RESET);

//XPT2046_Touchscreen ts(TOUCH_CS);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// LED pixel buffer
LED_Data ledData[MAX_LED_COUNT];

// Playback frame index
unsigned int currentFrame = 0;

// Test pattern to show on the LEDs, when an image cannot be loaded from the microSD card
void colorLoop() {
  static float j = 0;
  static float f = 0;
  static float k = 0;

  const float brightness = .4;
  
  for (unsigned int i = 0; i < ledCount; i++) {
    ledData[i].r = brightness*256.0*(1+sin(i/2.0 + j/4.0       ));
    ledData[i].g = brightness*256.0*(1+sin(i/1.0 + f/9.0  + 2.1));
    ledData[i].b = brightness*256.0*(1+sin(i/3.0 + k/14.0 + 4.2));
  }
  
  j = j + .1;
  f = f + .1;
  k = k + .2;
}

// Read one row of pixel data from the image, and store it in the LED framebuffer.
void imageLoop() {
  static int fraction = 0;
  
  for (unsigned int led = 0; led < ledCount; led++) {
    if(led < imageHeight) {
    
      const unsigned int pos = led*3 + currentFrame*imageHeight*3;
      ledData[led].r = imageData[pos + 0];
      ledData[led].g = imageData[pos + 1];
      ledData[led].b = imageData[pos + 2];
    }
    else {
      ledData[led].r = 0;
      ledData[led].g = 0;
      ledData[led].b = 0;
    }
  }


  fraction++;
  if(fraction > 100) {
    fraction = 0;
    currentFrame++;
  }

  if(currentFrame == imageWidth)
    currentFrame = 0;
}

void drawButton(const Button &button) {
  tft.setTextSize(2);
  tft.setCursor(button.x+5, button.y+10);
  tft.print(button.text);
  
  const int buttonWidth = 100;
  const int buttonHeight = 30;
  tft.drawRect(button.x,button.y,buttonWidth,buttonHeight, ILI9341_WHITE);
}

bool touchesButton(const Button &button, int touchX, int touchY) {
  const int buttonWidth = 100;
  const int buttonHeight = 30;
  return ((touchX >= button.x)
          && (touchX <= button.x + buttonWidth)
          && (touchY >= button.y)
          && (touchY <= button.y + buttonHeight));
}

void drawStats() {
  tft.fillRect(0,0,119,90, ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println("POV Player");
  tft.println("Blinkinlabs, LLC");
  
  tft.println("");
  tft.print("LEDs:");
  tft.println(ledCount);
  tft.print("Protocol:");
  if(protocol == PROTOCOL_MBI6020)
    tft.println("MBI6020");
  else if(protocol == PROTOCOL_APA102)
    tft.println("APA102");
  else if(protocol == PROTOCOL_ICND2110)
    tft.println("ICND2110");
  tft.print("SPI Clock:");
  tft.print((int)spiClockFrequency/1000000);
  tft.println("MHz");
  tft.print("Speed:");
  tft.print((int)fps);
  tft.println("fps");

  tft.println("");
  tft.print("Image:");
  tft.println(fileName);
  tft.print("Width:");
  tft.println(imageWidth);
  tft.print("Height:");
  tft.println(imageHeight);
}

void drawScreen() {
  tft.fillScreen(ILI9341_BLACK);

  // Draw some buttons
  drawButton(speedPlus);
  drawButton(speedMinus);
  drawButton(apa102);
  drawButton(mbi6020);
  drawButton(clockMinus);
  drawButton(clockPlus);

  // Draw the image
  uint16_t xOffset = 121;
  uint16_t yOffset = 1;
  
  for(unsigned int x = 0; x < imageWidth; x++) {
    for(unsigned int y = 0; y < imageHeight; y++) {
      const uint16_t color = tft.color565(
        imageData[x*imageHeight*3 + y*3 + 0],
        imageData[x*imageHeight*3 + y*3 + 1],
        imageData[x*imageHeight*3 + y*3 + 2]
        );
      tft.drawPixel(x + xOffset, y + yOffset, color);
    }
  }

  // And a frame around the image
  const uint16_t borderColor = tft.color565(0,0,255);
  tft.drawRect(xOffset - 1, yOffset - 1, imageWidth + 2, imageHeight + 2, borderColor);

  drawStats();
}

void setup() {
  // Initialize USB serial (for debug messages)
  Serial.begin(9600);
//  delay(3000);
  
  Serial.println("Blinkinlabs POV Player");
  Serial.println("Version 0.1");

  Serial.println("Starting TFT");
  tft.begin();
  
  Serial.println("Starting TS");
  ts.begin();

  Serial.println("Starting SD");
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("failed to initialize SD card!");
  }

  Serial.println("Loading Image");
  unsigned int retval;
  retval = loadbmp_decode_file(fileName, &imageData, &imageWidth, &imageHeight, LOADBMP_RGB);

  if(retval == LOADBMP_NO_ERROR) {
    Serial.println("Image loaded");

    // Set the led count based on the image height
    ledCount = min(MAX_LED_COUNT, imageHeight);
  }
  else {
    Serial.println("Error loading image, switching to test pattern");

    imageWidth = TESTPATTERN_WIDTH;
    imageHeight = TESTPATTERN_HEIGHT;
    imageData = (unsigned char*) testpatternData;

    ledCount = min(MAX_LED_COUNT, imageHeight);
    
    fps = TESTPATTERN_FPS;
  }

  pinMode(FRAME_TIMING, OUTPUT);
  digitalWriteFast(FRAME_TIMING, LOW);


  Serial.println("Starting LED output");
  if(protocol == PROTOCOL_MBI6020) {
    spiClockFrequency = SPI_CLOCK_FREQUENCY_MBI6020;
    mbi6020_begin(ledCount, spiClockFrequency);
  }
  else if(protocol == PROTOCOL_APA102) {
    spiClockFrequency = SPI_CLOCK_FREQUENCY_APA102;
    apa102_begin(ledCount, spiClockFrequency);
  }
  else if(protocol == PROTOCOL_ICND2110) {
    spiClockFrequency = SPI_CLOCK_FREQUENCY_ICND2110;
    icnd2110_begin(ledCount, spiClockFrequency);
  }

  frameDelay = 1000000/fps;

  // Draw some stats
  drawScreen();
}

bool lastTouchState = false;
bool drewUnderflowNotice = false;

int fraction = 0;

void loop() {
  elapsedMicros loopTime;

  bool currentTouchState = ts.touched();

  if((currentTouchState == true) && (lastTouchState == false)) {
    TS_Point p = ts.getPoint();

    // Determined experimentally
    const int mappedX = map(p.y,3800,300,0,240);
    const int mappedY = map(p.x,3900,370,0,320);

    Serial.print(p.x);
    Serial.print(",");
    Serial.println(p.y);

    if(touchesButton(speedPlus,mappedX,mappedY)) {
      // Speed +
      if(fps >= 100)
        fps += 100;
      else if(fps >= 10)
        fps += 10;
      else
        fps += 1;

      frameDelay = 1000000/fps;
      
      drawStats();
      drewUnderflowNotice = false;
    }
    else if(touchesButton(speedMinus,mappedX,mappedY)) {
      // Speed -
      if(fps > 100)
        fps -= 100;
      else if(fps > 10)
        fps -= 10;
      else if(fps > 1)
        fps -= 1;

      frameDelay = 1000000/fps;
      
      drawStats();
      drewUnderflowNotice = false;
    }
    else if(touchesButton(apa102,mappedX,mappedY)) {
      // APA102
      protocol = PROTOCOL_APA102;
      spiClockFrequency = SPI_CLOCK_FREQUENCY_APA102;
      apa102_begin(ledCount, spiClockFrequency);
      
      drawStats();
      drewUnderflowNotice = false;
    }
    else if(touchesButton(mbi6020,mappedX,mappedY)) {
      // MBI6020
      protocol = PROTOCOL_MBI6020;
      spiClockFrequency = SPI_CLOCK_FREQUENCY_MBI6020;
      mbi6020_begin(ledCount, spiClockFrequency);
      
      drawStats();
      drewUnderflowNotice = false;
    }
    else if(touchesButton(clockPlus,mappedX,mappedY)) {
      // Clock speed +
      if (spiClockFrequency < 39000000)
        spiClockFrequency += 1000000;

      drawStats();
      drewUnderflowNotice = false;
    }
    else if(touchesButton(clockMinus,mappedX,mappedY)) {
      // Clock speed -
      if (spiClockFrequency > 1000000)
        spiClockFrequency -= 1000000;

      drawStats();
      drewUnderflowNotice = false;
    }
  }
  lastTouchState = currentTouchState;


  const unsigned char *frameData = imageData + currentFrame*imageHeight*3;
    
  digitalWriteFast(FRAME_TIMING, HIGH);

  if(protocol == PROTOCOL_MBI6020)
    send16bitGrayscaleData(frameData, ledCount, spiClockFrequency);
  else if(protocol == PROTOCOL_APA102)
    apa102_SendData(frameData, ledCount, spiClockFrequency);
  else if(protocol == PROTOCOL_ICND2110)
    icnd2110_SendData(frameData, ledCount, spiClockFrequency);
    
  digitalWriteFast(FRAME_TIMING, LOW);


  fraction++;
  if(fraction > 1) {
    fraction = 0;
    currentFrame++;
    if(currentFrame == imageWidth)
      currentFrame = 0;
  }

// Note: this takes ~20us
  // If we exceeded loop time, draw a dot on the LCD to indicate this.
  if((loopTime > frameDelay) && (currentTouchState == false)) {
    if(!drewUnderflowNotice) {
      tft.drawPixel(90,52, ILI9341_RED);
      drewUnderflowNotice = true;
    }
  }
  
  while(loopTime < frameDelay);
}
