#include <SPI.h>
#include <SD.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include "MBI6020.h"
#include "APA102.h"
#include "LED_DATA.h"

//*************************************************
// POV Player
//
// Designed for use with a Teensy 3.6
//
// Default configuration:

const int MAX_LED_COUNT = 500;
int ledCount = 240;
int fps = 800;
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
//*************************************************

#define LOADBMP_IMPLEMENTATION
#include "readbmp.h"

#define MODE_TESTPATTERN 0
#define MODE_IMAGE  1
int mode;

#define PROTOCOL_MBI6020 0
#define PROTOCOL_APA102 1
int protocol = PROTOCOL_APA102;

// Decoded bitmap geometry
unsigned char *imageData;
unsigned int imageWidth = 0;
unsigned int imageHeight = 0;

unsigned int frameDelay;

//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC,TFT_MOSI, TFT_SCK, TFT_RESET, TFT_MISO);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//XPT2046_Touchscreen ts(TOUCH_CS);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// LED pixel buffer
LED_Data ledData[MAX_LED_COUNT];


// Test pattern to show on the LEDs, when an image cannot be loaded from the microSD card
void colorLoop() {  
  static float i = 0;
  static float j = 0;
  static float f = 0;
  static float k = 0;

  const float brightness = .4;
  
  for (size_t i = 0; i < ledCount; i++) {
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
  static int currentFrame = 0;

  for (size_t led = 0; led < ledCount; led++) {
    if(led < imageHeight) {
    
      const unsigned int pos = led*imageWidth*3 + currentFrame*3;
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

  currentFrame++;
  
  if(currentFrame == imageWidth)
    currentFrame = 0;
}

void drawButton(char *label, int x, int y) {
  tft.setTextSize(2);
  tft.setCursor(x+5, y+10);
  tft.print(label);
  
  const int buttonWidth = 100;
  const int buttonHeight = 30;
  tft.drawRect(x,y,buttonWidth,buttonHeight, ILI9341_WHITE);
}

bool touchesButton(int x, int y, int touchX, int touchY) {
  const int buttonWidth = 100;
  const int buttonHeight = 30;
  return ((touchX >= x)
          && (touchX <= x + buttonWidth)
          && (touchY >= y)
          && (touchY <= y + buttonHeight));
}

void drawStats() {
  tft.fillRect(0,0,119,119, ILI9341_BLACK);
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


  // Also write it to serial, for debugging
  Serial.println("");
  Serial.print("LEDs:");
  Serial.println(ledCount);
  Serial.print("Protocol:");
  if(protocol == PROTOCOL_MBI6020)
    Serial.println("MBI6020");
  else if(protocol == PROTOCOL_APA102)
    Serial.println("APA102");
  Serial.print("Speed:");
  Serial.print((int)fps);
  Serial.println("fps");
  Serial.print("Image:");
  Serial.println(fileName);
  Serial.print("Width:");
  Serial.println(imageWidth);
  Serial.print("Height:");
  Serial.println(imageHeight);
}

void drawScreen() {
  tft.fillScreen(ILI9341_BLACK);

  // Draw some buttons
  drawButton("Speed +", 10,120);
  drawButton("Speed -", 10,170);
  drawButton("APA102", 10,220);
  drawButton("MBI6020", 10,270);

  // Draw the image  
  if(mode == MODE_IMAGE) {    
    uint16_t xOffset = 121;
    uint16_t yOffset = 1;
    
    for(int x = 0; x < imageWidth; x++) {
      for(int y = 0; y < imageHeight; y++) {
        const uint16_t color = tft.color565(
          imageData[y*imageWidth*3 + x*3 + 0],
          imageData[y*imageWidth*3 + x*3 + 1],
          imageData[y*imageWidth*3 + x*3 + 2]
          );
        tft.drawPixel(x + xOffset, y + yOffset, color);
      }
    }

    // And a frame around the image
    const uint16_t borderColor = tft.color565(0,0,255);
    tft.drawRect(xOffset - 1, yOffset - 1, imageWidth + 2, imageHeight + 2, borderColor);
  }

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
  // It seems that the display doesn't fully turn on if not reset twice- not sure why.
  delay(100);
  tft.begin();
  
  Serial.println("Starting TS");
  ts.begin();

  Serial.println("Starting LED output");
  if(protocol == PROTOCOL_MBI6020)
    mbi6020_begin();
  else if(protocol == PROTOCOL_APA102)
    apa102_begin();

  Serial.println("Starting SD");
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("failed to initialize SD card!");
  }

  Serial.println("Loading Image");
  unsigned int retval;
  retval = loadbmp_decode_file(fileName, &imageData, &imageWidth, &imageHeight, LOADBMP_RGB);

  if(retval == LOADBMP_NO_ERROR) {
    Serial.println("Image loaded");
    
    mode = MODE_IMAGE;

    // Set the led count based on the image height
    ledCount = min(MAX_LED_COUNT, imageHeight);
  }
  else {
    Serial.println("Error loading image, switching to test pattern");
    mode = MODE_TESTPATTERN;
  }

  frameDelay = 1000000/fps;

  // Draw some stats
  drawScreen();
}

void loop() {
  elapsedMicros loopTime;

  if(ts.touched()) {
    TS_Point p = ts.getPoint();

    // Determined experimentally
    const int mappedX = map(p.y,3800,300,0,240);
    const int mappedY = map(p.x,3900,370,0,320);

    Serial.print(p.x);
    Serial.print(",");
    Serial.println(p.y);

    if(touchesButton(10,120,mappedX,mappedY)) {
      // Speed +
      if(fps >= 100)
        fps += 100;
      else if(fps >= 10)
        fps += 10;
      else
        fps += 1;

      frameDelay = 1000000/fps;
      drawStats();
    }
    else if(touchesButton(10,170,mappedX,mappedY)) {
      // Speed -
      if(fps > 100)
        fps -= 100;
      else if(fps > 10)
        fps -= 10;
      else if(fps > 1)
        fps -= 1;

      frameDelay = 1000000/fps;
      drawStats();
    }
    else if(touchesButton(10,220,mappedX,mappedY)) {
      // APA102
      protocol = PROTOCOL_APA102;
      apa102_begin();
      drawStats();
    }
    else if(touchesButton(10,270,mappedX,mappedY)) {
      // MBI6020
      protocol = PROTOCOL_MBI6020;
      mbi6020_begin();
      drawStats();
    }

    // Return here to avoid tripping the loop time indicator
    return;
  }

  if(mode == MODE_TESTPATTERN)
    colorLoop();
  else
    imageLoop();

  if(protocol == PROTOCOL_MBI6020)
    send16bitGrayscaleData(ledData, ledCount);
  else if(protocol == PROTOCOL_APA102)
    apa102_SendData(ledData, ledCount);

  // If we exceeded loop time, draw a dot on the LCD to indicate this.
  if(loopTime > frameDelay)
    tft.drawPixel(90,45, ILI9341_RED);
  
  while(loopTime < frameDelay);
}
