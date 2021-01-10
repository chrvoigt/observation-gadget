// date:

// third party libraries
#include <TinyGPS++.h>        // Tiny GPS Plus Library
#include <SoftwareSerial.h>   // Software Serial Library so we can use other Pins for communication with the GPS module
#include <Wire.h>             // I2C library
#include <Adafruit_GFX.h>     // Graphic library
#include <Adafruit_SSD1306.h> // OLED library
#include <FS.h>               // SPIFF file system
#include "Adafruit_MPR121.h"  // Capacity touch library
//#include <LOLIN_I2C_BUTTON.h>                         // not needed

// tracker components
#define SDCARD false
#define UBLOX true
#define SPIFF true // SPIFF - Flash Memory - handled in GPS_UBlox.h

// definitions
#define DEBUG true
#define OLED_RESET -1

// tracking frequency
unsigned long oldMillis = 0;
unsigned long currMillis;
const unsigned long trackingFreq = 4 * 1000UL;
bool wayPoint = false;

// features
#include "GPS_UBlox.h"
#include "Display.h"
#include "SDCard.h"
#include "Web.h"
#include "Cap_touch.h"


//===============================================================
//                  SETUP
//===============================================================

void setup()
{
  if (DEBUG)
    Serial.begin(115200);
  initCap();
  initDisplay();
  if (SPIFF)
    initFlash();
  if (SDCARD)
    initCard();
  if (UBLOX)
    initUBlox();
  initAP();

  delay(20);
}

//===============================================================
//                  LOOP
//===============================================================

// main loop includes a timer, every x secs GPS coordinates are written to Serial & flash memory
// frequency can be changed via 'trackingFreq''

void loop()
{
  currMillis = millis();
  if (currMillis - oldMillis > trackingFreq)
  {
    Serial.print(trackingFreq / 1000);
    Serial.println(" secs passed ...");
    wayPoint = true;
    oldMillis = currMillis;
  }
  else
  {
    wayPoint = false;
  }
  readCap(wayPoint);

  // for a GPS backup or later on other types of data requiring SDcard
  // writeCard();
}
