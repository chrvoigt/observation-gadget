/**
 it’s important to not use delay() in any sketch which processes streaming serial input. 
 If you wish the slow down a display, try a different technique, like keeping track of the last time 
 the display was updated. You can add logic to avoid updating the display except, say, after 1000 milliseconds have elapsed.
 **/

#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// GPS doc under http://arduiniana.org/libraries/tinygpsplus/
static const int RXPin = 12, TXPin = 13; // Ublox 6m GPS module to pins 12 and 13
static const uint32_t GPSBaud = 9600;    // Ublox GPS default Baud Rate is 9600
TinyGPSPlus gps;                         // Create an Instance of the TinyGPS++ object called gps
SoftwareSerial ss(RXPin, TXPin);         // The serial connection to the GPS device

// Vienna Work, any reference point would do .. e.g. a fast food place, a lake, a train station
const double Home_LAT = 48.184245; // Your Home Latitude
const double Home_LNG = 16.330493; // Your Home Longitude

// DataString Vars
String dataString = "";
char buff[10];
char buffer[10];

int date, hours, minutes, seconds;
int time_diff = 1; // adaptation to time zones
double lat, lon;
float alt, speed, prec;
String sats, rtime;
String filename = "";

void initUBlox()
{
  ss.begin(GPSBaud); // Set Software Serial Comm Speed to 9600
}

static void readGPS(unsigned long ms) // This custom version of delay() ensures that the gps object is being "fed".
{
  gps.encode(ss.read());
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void buildString(String element)
{
  dataString = dataString + element + ',';
}

void onSerialGPS() // Includes writing to SPIFF
{
  dataString = "";
  readGPS(500);

  // Date & Time
  date = gps.date.value();
  hours = gps.time.hour() + time_diff;
  minutes = gps.time.minute();
  seconds = gps.time.second();
  rtime = dtostrf(gps.time.value(), 8, 0, buff); //  no time diff adjustment with rtime (raw time)

  buildString(dtostrf(date, 6, 0, buff));
  Serial.print("Date: ");
  Serial.println(date); // Raw date in DDMMYY format (u32)

  sprintf(buffer, "%02d:%02d:%02d", hours, minutes % 60, seconds % 60);
  buildString(buffer);
  Serial.print("Time: ");
  Serial.println(buffer);

  buildString(rtime);
  Serial.print("RawTime=");
  Serial.println(rtime);

  // Location
  lat = gps.location.lat();
  lon = gps.location.lng();
  alt = gps.altitude.meters();
  sats = dtostrf(gps.satellites.value(), 0, 0, buff);
  speed = gps.speed.kmph();
  prec = gps.hdop.value() / 100.00;

  buildString(dtostrf(lat, 4, 6, buff));
  Serial.print("LAT: ");
  Serial.println(lat, 6);

  buildString(dtostrf(lon, 4, 6, buff));
  Serial.print("LON: ");
  Serial.println(lon, 6);

  buildString(dtostrf(alt, 2, 2, buff));
  Serial.print("ALT: ");
  Serial.println(alt, 2);

  buildString(sats);
  Serial.print("SATS: ");
  Serial.println(sats);

  buildString(dtostrf(speed, 4, 4, buff));
  Serial.print("Speed: ");
  Serial.println(speed, 04);

  Serial.print("Precision: ");

  dataString = dataString + dtostrf(prec, 0, 2, buff);
  Serial.println(prec);

  /*
  // Distance between points => causes an exception if there is no SAT fix 
  // you can check it here (480 km)  https://www.distance.to/Vienna,AUT/Ilmenau,Th%C3%BCringen,DEU
  float dist_to_home = (float)TinyGPSPlus::distanceBetween(lat, lon, Home_LAT, Home_LNG);
  buildString(dtostrf(dist_to_home, 0, 2, buff));
  Serial.print("Distance to Ref. Point: ");
  Serial.println(dist_to_home);
  */

  Serial.print("DataString: ");
  Serial.println(dataString);
}

void initFlash()
{
  Serial.println("SPIFF Starting...");
  SPIFFS.begin();
  delay(500);
  
  filename = "/data/tdata.csv";
  if (SPIFFS.exists(filename)) {
    Serial.println("/data/tdata.csv existed already ...");
  }
  else {
    File t_DataFile = SPIFFS.open(filename, "a+"); // Open a file for reading and writing (appending), file is created if not existent ...)
    t_DataFile.println("Date,Time,RawTime,Latitude,Longitude,Altitude,Sats,SatSpeed,Precision"); // Writing the header
    Serial.println("/data/tdata.csv created ...");
    t_DataFile.close(); 
  }

  filename = "/data/rdata.csv";
  if (SPIFFS.exists(filename)) {
    Serial.println("/data/rdata.csv existed already ...");
  }
  else {
    File t_DataFile = SPIFFS.open(filename, "a+"); // Open a file for reading and writing (appending), file is created if not existent ...)
    t_DataFile.println("Date,Time,RawTime,Latitude,Longitude,Altitude,Sats,SatSpeed,Precision,Category"); // Writing the header
    Serial.println("/data/rdata.csv created ...");
    t_DataFile.close(); 
  }
}


void pathCount () {
  int tLines = 0;

  filename = "/data/tdata.csv";
  if (SPIFFS.exists(filename)) {
      File tfile = SPIFFS.open(filename,"r");
      while (tfile.available())
      {
        String tline = tfile.readStringUntil('\n');
        //Serial.println (tline);
        tLines = tLines + 1 ; 
      }
      tfile.close(); // Close the file
      tLines = tLines - 1 ; // discount header
      Serial.print ("Number of way points: ");
      Serial.println (tLines);
  }   
}

int catCount () {
  int rLines = 0;

  filename = "/data/rdata.csv";
  if (SPIFFS.exists(filename)) {
      File rfile = SPIFFS.open(filename,"r");
      while (rfile.available())
      {
        String rline = rfile.readStringUntil('\n');
        //Serial.println (rline);
        rLines = rLines + 1 ; 
      }
      rfile.close(); // Close the file
      rLines = rLines - 1 ; // discount header
      Serial.print ("Number of category points: ");
      Serial.println (rLines);
  }
  return rLines;
}

// SPIFF Doc here http://arduino.esp8266.com/Arduino/versions/2.3.0/doc/filesystem.html
void flashWriting()
{
  if (sats != "0" && lat != 0 && lon !=0)
  {
    filename = "/data/tdata.csv";
    Serial.print("Writing to:");
    Serial.println(filename);
    
    if (!SPIFFS.exists(filename)) {
      File t_DataFile = SPIFFS.open(filename, "a+"); // create and Open file
      t_DataFile.println("Date,Time,RawTime,Latitude,Longitude,Altitude,Sats,SatSpeed,Precision"); // Writing the header
      Serial.println("tdata - CSV header row writen ...");
      t_DataFile.close(); 
    }

    File t_DataFile = SPIFFS.open(filename, "a+");  // Open a file for reading and writing (appending), file is created if not existent ...)
    if (!t_DataFile)
      Serial.println(" failed to open.");           // Check for errors
    else
    {
      Serial.println(" is open.");
    }
    t_DataFile.println(dataString);                // Write some data
    t_DataFile.close();                            // Close the file
  }
  else
  {
    Serial.println("No SATELLITE Fix available (yet)!");
  }
}

void flashRating(char *rating)
{
  if (sats != "0")
  {
    filename = "/data/rdata.csv";
    Serial.print("Writing to:");
    Serial.println(filename);

    if (!SPIFFS.exists(filename)) {
      File t_DataFile = SPIFFS.open(filename, "a+"); // create and Open file
      t_DataFile.println("Date,Time,RawTime,Latitude,Longitude,Altitude,Sats,SatSpeed,Precision,Category"); // Writing the header
      Serial.println("rdata - CSV header row writen ...");
      t_DataFile.close(); 
    }
    
    File r_DataFile = SPIFFS.open(filename, "a+"); // Open a file for reading and writing (appending), file is created if not existent ...)
    if (!r_DataFile)
      Serial.println(" failed to open."); // Check for errors
    else
    {
      Serial.println(" is open.");
    }
    //buildString(rating);
    dataString = dataString + ',' + rating;
    r_DataFile.println(dataString); // Write GPS datastring with rating attached
    Serial.print("File size:  ");
    Serial.println(r_DataFile.size()); // Display the file size (26 characters + 4-byte floating point number + 6 termination bytes (2/line) = 34 bytes)
    r_DataFile.close();                // Close the file
  }
  else
  {
    Serial.println("No SATELLITE Fix available (yet)!");
  }
}

void flashDir()
{
  String str = "";
  Dir dir = SPIFFS.openDir("");
  while (dir.next())
  {
    str += dir.fileName();
    str += " :: ";
    str += dir.fileSize();
    str += "\r\n";
  }
  Serial.print(str);
}

void flashReading()
{
  Serial.println('**************');
  Serial.println('Tracking Data');
  sats = "0";
  filename = "/data/tdata.csv";
  Serial.print("Filename:");
  Serial.println(filename);
  File tr_DataFile = SPIFFS.open(filename, "r"); // Open the file again, this time for reading
  if (!tr_DataFile)
    Serial.println("file open failed"); // Check for errors
  while (tr_DataFile.available())
  {
    Serial.write(tr_DataFile.read()); // Read all the data from the file and show via Serial ...
  }
  tr_DataFile.close(); // Close the file

  Serial.println('**************');
  Serial.println('Rating Data');
  sats = "0";
  filename = "/data/rdata.csv";
  Serial.print("Filename:");
  Serial.println(filename);
  File rr_DataFile = SPIFFS.open(filename, "r"); // Open the file again, this time for reading
  if (!rr_DataFile)
    Serial.println("file open failed"); // Check for errors
  while (rr_DataFile.available())
  {
    Serial.write(rr_DataFile.read()); // Read all the data from the file and show via Serial ...
  }
  rr_DataFile.close();
}

void flashDelete () {
  filename = "/data/tdata.csv";
  if (SPIFFS.exists(filename)) {
    SPIFFS.remove(filename);
    Serial.println("/data/rdata.csv was removed ...");
  }
  else {
    Serial.println("There was no /data/tdata.csv to delete ...");
  }

  filename = "/data/rdata.csv";
  if (SPIFFS.exists(filename)) {
    SPIFFS.remove(filename);
    Serial.println("/data/rdata.csv was removed ...");
  }
  else {
    Serial.println("There was no /data/rdata.csv to delete ...");
  }
}
