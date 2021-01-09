
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//SSID and Password to your ESP Access Point
const char *ssid = "Mapper";
IPAddress apIP(44, 44, 44, 44);
ESP8266WebServer server(80); //Server on port 80

String getContentType(String filename)
{ // convert the file extension to the MIME type
  if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".csv"))
    return "text/csv";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".jpeg"))
    return "image/jpeg";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  return "text/plain";
}

bool handleFileRead(String path)
{ // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
    path += "index.html";                    // If a folder is requested, send the index file
  String contentType = getContentType(path); // Get the MIME type
  if (SPIFFS.exists(path))
  {                                                     // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    delay(1);
    file.close(); // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false; // If the file doesn't exist, return false
}

void apRunning(boolean apResult)
{
  if (apResult == true)
  {
    Serial.println("Access Point ready");
  }
  else
  {
    Serial.println("Access Point not available");
  }
}


boolean apStart()
{
  /* You can remove the password parameter if you want the AP to be open. */
  boolean apResult = WiFi.softAP(ssid);
  if (apResult == true)
  {
    Serial.println("Access Point ready");
  }
  else
  {
    Serial.println("Access Point failed!");
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin(); // Actually start the server
  Serial.println("HTTP server started");

  return apResult;
}


boolean apStop()
{
  boolean apResult = WiFi.softAPdisconnect(true);
  return apResult;
}

void initAP()
{
  delay(10);
  Serial.println("Configuring access point...");
  Serial.println('\n');

  //set-up the custom IP address
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); // subnet FF FF FF 00
  apStop();
}
