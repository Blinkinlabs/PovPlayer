#include <SPI.h>
#include <SD.h>
#include "MBI6020.h"

//*************************************************
// Configuration:

// Modify these to 
const int LED_COUNT = 40;
const float FPS = 1500;

// Designed for use with a Teensy 3.6
//
// Wiring:
// -Connect Teensy 3.6 pin 11 to the 'INSDI' input on the converter PCB
// -Connect Teensy 3.6 pin 13 to the 'INCLK' input on the converter PCB
// -Connect Teensy 3.6 gnd pin to the 'GND' input on the converter PCB
//*************************************************


#define LOADBMP_IMPLEMENTATION
#include "readbmp.h"

#define MODE_TESTPATTERN 0
#define MODE_IMAGE  1
int mode;

// Decoded bitmap geometry
unsigned char *imageData;
unsigned int imageWidth = 0;
unsigned int imageHeight = 0;

unsigned int frameDelay;

// LED pixel buffer
LED_Data ledData[LED_COUNT];

// Test pattern to show on the LEDs, when an image cannot be loaded from the microSD card
void colorLoop() {  
  static float i = 0;
  static float j = 0;
  static float f = 0;
  static float k = 0;

  const float brightness = .4;
  
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    ledData[i].r = brightness*32768.0*(1+sin(i/2.0 + j/4.0       ));
    ledData[i].g = brightness*32768.0*(1+sin(i/1.0 + f/9.0  + 2.1));
    ledData[i].b = brightness*32768.0*(1+sin(i/3.0 + k/14.0 + 4.2));
  }

  
  j = j + .1;
  f = f + .1;
  k = k + .2;
}

// Read one row of pixel data from the image, and store it in the LED framebuffer.
void imageLoop() {
  static int currentFrame = 0;

  for (uint8_t led = 0; led < LED_COUNT; led++) {
    if(led < imageHeight) {
    
      const unsigned int pos = led*imageWidth*3 + currentFrame*3;
    
      ledData[led].r = imageData[pos + 0] << 7;
      ledData[led].g = imageData[pos + 1] << 7;
      ledData[led].b = imageData[pos + 2] << 7;
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

void setup() {
  // Initialize USB serial (for debug messages)
  Serial.begin(9600);
  
  // initialize SPI:
  SPI.begin();

  // This routine copied from ARC3730 controller
  for(int i = 0; i < 15; i++) {
    // Send 16-bit grayscale data [111111], all zeros, except the last two LEDs have some red
    send16bitGrayscaleData(ledData, LED_COUNT);
    delayMicroseconds(250);
    // Send config48 data [100011], for all channels
    send16bitConfigurationData(LED_COUNT);
    delay(12);
    
    // Send 16-bit grayscale data [111111], all zeros, except the last three LEDs have some red
    send16bitGrayscaleData(ledData, LED_COUNT);
    delayMicroseconds(250);
    // Send 8-bit dot correction data [110011], all ones
    send8bitDotCorrectionDataHigh(LED_COUNT);
    delay(12);
  }
  
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("failed to initialize SD card!");
    return;
  }

  unsigned int retval;
  retval = loadbmp_decode_file("POV.BMP", &imageData, &imageWidth, &imageHeight, LOADBMP_RGB);

  Serial.print("Return value:");
  Serial.println(retval);
  Serial.print("  width:");
  Serial.println(imageWidth);
  Serial.print("  height:");
  Serial.println(imageHeight);

  if(retval == LOADBMP_NO_ERROR)
    mode = MODE_IMAGE;
  else
    mode = MODE_TESTPATTERN;

  frameDelay = (1/FPS)*1000000;
}

void loop() {
  elapsedMicros loopTime;
  
  if(mode == MODE_TESTPATTERN)
    colorLoop();
  else
    imageLoop();

  send16bitGrayscaleData(ledData, LED_COUNT);

  while(loopTime < frameDelay);
}
