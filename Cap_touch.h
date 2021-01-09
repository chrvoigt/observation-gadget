// see *selection[] for functions ... Tracking, Rating etc
#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// You can have up to 4 on one i2c bus, see wiring tips below
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

char *selection[] = {"nothing", "Path", "Category", "Satellites", "Web", "Datastats", "Serials", "1", "2", "3", "4", "5"};
bool track_flag = false;
bool rating_flag = false;
bool web_flag = false;
bool rating_set = false;
boolean apStatus = false; // Access Point Status

char *rating = '\0';
int cc = 0; // category count 

void initCap()
{
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5B))
  {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
}

void readCap(bool timeUp)
{
  // Get the currently touched pads
  currtouched = cap.touched();

  for (uint8_t i = 0; i < 12; i++)
  {
    //  if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)))
    {
      switch (i)
      {
      case 0:                                               // delete data files
        Serial.println("Delete Data Files selected ...");
        flashDelete ();
        onDisplay1 ("Your data"); 
        onDisplay2 ("are gone !");
        onDisplay3 (" ..."); 
        onDisplay4 (" New run? "); 
        break;
      case 1:                                              //  tracking on | off
        track_flag = !track_flag;
        if (track_flag)
        {
          Serial.println("Path ON");
          onDisplay1 ("Path ON"); 
        }
        else
        {
          Serial.println("Path OFF");
          onDisplay1 ("Path OFF"); 
        }
        break;
      case 2:                                               //  Selecting a Category;
        rating_flag = true;
        if (!track_flag)
        {
          Serial.println("Path is OFF & Rating selected!");
          onDisplay3 ("Cat [1-5]");
          onDisplay4 (" ");
        }
        else
        {
          Serial.println("Path is ON & Rating selected!");
          onDisplay3 ("Cat [1-5]");
          onDisplay4 (" ");
        }
        break;
      case 3:                                               //  Satelite Quality 
        sats = "0";
        onSerialGPS();
        Serial.print("Satellites: ");
        Serial.println(sats);
        onDisplay3 ("Satellites");
        onDisplay4 (sats); 
        break;
      case 4:                                               //  Web On, Off
        web_flag = !web_flag;
        break;
      case 5:                                              //  Data Statistics
        Serial.println("Datastats");
        pathCount ();
        cc = catCount ();
        onDisplay3 ("Statistics");
        onDisplay4 (String (cc)); 
        // whhen using SD Card => spiffToCard ();
        break;
      case 6:                                               //
        track_flag = false;
        Serial.println("Serial Output");
        onDisplay3 ("Serial");
        onDisplay4 ("Output"); 
        sats = "0";
        onSerialGPS();
        flashDir();
        flashReading();
        break;    
      default:
        if (rating_flag && (i > 6))
        {
          rating_set = !rating_set;
          rating = selection[i];
        }
      }
      delay(1);
    }
    // if it *was* touched and now *isnt*, here is the release option
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
    {
    }
  }  // end of for loop

//===============================================================
//                  Handling WEB server 
//===============================================================

// SERVER ON  if Access Point selected but AP is not yet running  
  if (web_flag && !apStatus)
  {
    apStatus = apStart();
    apRunning(apStatus);
    Serial.printf("\n");
    onDisplay2 ("Web is ON"); 
    onDisplay3 ("AP: Mapper"); 
    onDisplay4 ("URL: 4x44."); 
    
  }

// SERVER OFF  if Access Point is not selected but AP is still running  
if (!web_flag && apStatus)
  {
    apStatus = apStop();
    if (apStatus)
    {
      Serial.printf("Disconnected !");
    };
    Serial.printf("\n");
    onDisplay2 ("Web is OFF"); 
    onDisplay3 (" ");
    onDisplay4 (" ");
  }

if (web_flag)
  {
    server.handleClient(); //Handle client requests
  }

if (web_flag && timeUp)
  {
    Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
  }
  
 
//===============================================================
//                  Writing GPS if x secs are passed
//===============================================================

  if (track_flag && timeUp)
  {
    sats = "0";
    Serial.println("Calling GPS");
    onSerialGPS();  // calls readGPS(500)
    flashWriting(); // creates tracking file
  }
//===============================================================
//                  Confirming the selected category 
//===============================================================

  if (rating_flag && rating_set)
  {
    onDisplay4 (rating);
    sats = "0";
    onSerialGPS();
    flashRating(rating);
    rating_flag = !rating_flag;
    rating_set = !rating_set;
    rating = '\0';
  }
  lasttouched = currtouched; // reset our state

} // end of readCap()

/*
  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
  Serial.print("Filt: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.filteredData(i)); Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.baselineData(i)); Serial.print("\t");
  }
  Serial.println();
  
  // put a delay so it isn't overwhelming
  delay(200);
  */
