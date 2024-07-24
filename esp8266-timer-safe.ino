// Using an ESP8266 as a replacement controller for a digital safe board
// providing a web-UI for control
//
// Original version (c) 2021 Stephen Harris
// This version modified to act as a timer safe (c) 2023 Stephen Harris
//
// MIT License

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include "html.h"
#include "version.h"

// We store the timer in SPIFFs because this is wear levelled and
// so should last a lot longer; a 1Mb SPIFFs writing a small file
// once every 5 minutes should last forever
#include <FS.h>

// This is pin D6 on most boards; this is the pin that needs to be
// connected to the relay
#define default_pin D6

// We need to store some values in EEPROM.
//   safe UI username
//   safe UI password
//   WiFi SSID
//   WiFi password
//   XXXX NOT ANY MORE XXXX lock password (in this case it's the timer value)
//   Safe Name
//   Pin to open solenoid
//
// For simplicitly we'll limit them to 100 characters each and sprinkle
// them through the EEPROM at 128 byte offsets
// It's not very efficient, but we have up to 4096 bytes to play with
// so...
//
// We put a magic string in front to detect if the values are good or not

#define EEPROM_SIZE 1024
#define maxpwlen 100
#define eeprom_magic "PSWD:"
#define eeprom_magic_len 5

#define ui_username_offset   0
#define ui_pswd_offset       128
#define ui_wifi_ssid_offset  256
#define ui_wifi_pswd_offset  384
#define lock_timer_offset    512
#define safename_offset      640
#define pin_offset           768

/////////////////////////////////////////

// Global variables

// To get SPIFFS info
FSInfo fs_info;

boolean wifi_connected;
boolean allow_updates = false;

// These can be read at startup time from EEPROM
String ui_username;
String ui_pswd;
String wifi_ssid;
String wifi_pswd;
String safename;
String pinstr;
int    pin;

// The timer and if the timer is hidden
int timer=0;
int hidden=0;

// Create the webserver structure for port 80
ESP8266WebServer server(80);

/////////////////////////////////////////

// Read/write EEPROM values

String get_pswd(int offset)
{ 
  char d[maxpwlen];
  String pswd;

  for (int i=0; i<maxpwlen; i++)
  { 
    d[i]=EEPROM.read(offset+i);
  }

  pswd=String(d);
  
  if (pswd.startsWith(eeprom_magic))
  { 
    pswd=pswd.substring(eeprom_magic_len);
  }
  else
  {
    pswd="";
  }

  return pswd;
}

void set_pswd(String s, int offset, bool commit=true)
{ 
  String pswd=eeprom_magic + s;

  for(int i=0; i < pswd.length(); i++)
  {
    EEPROM.write(offset+i,pswd[i]);
  }
  EEPROM.write(offset+pswd.length(),0);
  if (commit)
    EEPROM.commit();
}

// There's got to be a better way of doing this...
void read_timer()
{
  String timer_str="0";
  String hidden_str="0";
  File file = SPIFFS.open("timer","r");
  if (!file)
  {
    Serial.println("No timer file");
  }
  else
  {
    timer_str=file.readStringUntil(' ');
    hidden_str=file.readString();
  }
  timer=timer_str.toInt();
  hidden=hidden_str.toInt();
  Serial.println("Timer loaded at " + String(timer) + " with " + String(hidden));
  file.close();
}

void save_timer()
{
  File file=SPIFFS.open("timer", "w");
  String timer_str = String(timer) + " " + String(hidden);
  file.println(timer_str);
  file.close();
  Serial.println("Saved timer " + timer_str);
}

/////////////////////////////////////////

// These functions manipulate the safe state.
// They look like they don't take any params, because they use the
// global server state variable to read the request.  This only works
// 'cos the main loop is single threaded!

void status()
{
  if (hidden==0)
    send_text(String(timer));
  else
    send_text("-1");
}


void addtime()
{
  String d = server.arg("laddtime");
  int low=d.toInt();
  if (low<0) low=0;

  d = server.arg("haddtime");
  int high=d.toInt();
  if (high<0) high=0;
  if (high==0) { high=low; }
  // Because of human interaction, it's likely the current time() is
  // sufficient to seed the RNG
  randomSeed(time(NULL));
  int t = random(low,high+1);
  int newtimer = timer+t;
  // If newtimer is less than timer then we've wrapped the int!!
  if (newtimer < timer)
  {
    Serial.println("Timer wrap around prevented");
    newtimer=2147483647;  // INT_MAX
  }
  timer = newtimer;

  if (server.arg("hide") != "")
    hidden=1;

  save_timer();

  if (hidden==0)
    send_text("Added " + String(t) + " seconds.  Lock now set to " + String(timer));
  else
    send_text("Added time to lock");
}

void opensafe()
{
  if (timer != 0 )
  {
    send_text("Can not open; safe is locked");
  }
  else
  {
    String d = server.arg("duration");
    int del=d.toInt();
    if (del==0) { del=5; }
    digitalWrite(LED_BUILTIN,LOW);
    digitalWrite(pin,HIGH);
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    send_text("Unlocking safe for " + String(del) + " seconds<br>");
    while(del--)
    {
      delay(1000);
      server.sendContent(String(del) + "...\n");
      if (del%10 == 0) server.sendContent("<br>");
    }
    digitalWrite(pin,LOW);
    digitalWrite(LED_BUILTIN,HIGH);
    server.sendContent("Completed");
    server.sendContent("");
  }
}

void enable_update(bool enable)
{
  if (enable)
  {
    if (timer == 0)
    {
      allow_updates = true;
      send_text("Updates can be sent using BasicOTA to: " + WiFi.localIP().toString());
    }
    else
    {
      allow_updates = false;
      send_text("Can not perform update while safe is locked");
    }
  }
  else
  {
    allow_updates = false;
    send_text("Update server disabled");
  }
}

void set_ap()
{
  if (server.hasArg("setwifi"))
  {
    Serial.println("Setting WiFi client");
    safename=server.arg("safename");
    safename.replace(".local","");
    if (safename != "")
    {
      Serial.println("  Setting mDNS name to "+safename);
      set_pswd(safename,safename_offset);
    }

    pinstr=server.arg("pin");
    if (pinstr != "")
    {
      Serial.println("  Setting active pin to "+pinstr);
      set_pswd(pinstr,pin_offset);
    }

    if (server.arg("ssid") != "" && server.arg("password") != "")
    {
      Serial.println("Setting WiFi client:");
      set_pswd(server.arg("ssid"),ui_wifi_ssid_offset,false);
      set_pswd(server.arg("password"),ui_wifi_pswd_offset);
    }

    send_text("Restarting in 5 seconds");
    delay(5000);
    ESP.restart();
  }

  String page = change_ap_html;
         page.replace("##safename##", safename);
         page.replace("##pin##", String(pin));
         page.replace("##VERSION##", VERSION);
  send_text(page);
}

void set_auth()
{
  Serial.println("Setting Auth details");
  ui_username=server.arg("username");
  ui_pswd=server.arg("password");
  set_pswd(ui_username,ui_username_offset);
  set_pswd(ui_pswd,ui_pswd_offset);
  send_text("Password reset");
}

/////////////////////////////////////////

boolean handleRequest()
{ 
  String path=server.uri();
  if (!wifi_connected)
  {
    // If we're in AP mode then all requests must go to change_ap
    // and there's no authn required
    ui_username = "";
    ui_pswd = "";
    path="/change_ap.html";
  }

  Serial.println("New client for >>>"+path+"<<<");

  for(int i=0;i<server.args();i++)
  {
    Serial.println("Arg " + String(i) + ": " + server.argName(i) + " --- " + server.arg(i));
  }

  // Ensure username/password have been passed
  if (ui_username != "" && !server.authenticate(ui_username.c_str(), ui_pswd.c_str()))
  {
    Serial.println("Bad authentication; login needed");
    server.requestAuthentication();
    return true;
  }

       if (path == "/")                 { send_text(index_html); }
  else if (path == "/status.html")      { send_text(status_html); }
  else if (path == "/menu_frame.html")  { send_text(menu_frame_html); }
  else if (path == "/top_frame.html")   { send_text(top_frame_html); }
  else if (path == "/change_auth.html") { send_text(change_auth_html); }
  else if (path == "/change_ap.html")   { set_ap(); }
  else if (path == "/enable_update")    { enable_update(1); }
  else if (path == "/disable_update")   { enable_update(0); }
  else if (path == "/safe/")
  {
         if (server.hasArg("status"))     { status(); }
    else if (server.hasArg("open"))       { opensafe(); }
    else if (server.hasArg("setauth"))    { set_auth(); }
    else if (server.hasArg("laddtime"))    { addtime(); }
    else return false;
  }
  else
  {
    Serial.println("File not found");
    return false;
  }
  return true;
}

/////////////////////////////////////////

void send_text(String s)
{
  server.send(200,"text/html", s);
}

/////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println("Starting...");

  SPIFFS.begin();
  SPIFFS.info(fs_info);

  Serial.print("Total size of SPIFFS is: ");
  Serial.println(fs_info.totalBytes);

  Serial.print("Used size of SPIFFS is: ");
  Serial.println(fs_info.usedBytes);

  // Get the EEPROM contents into RAM
  EEPROM.begin(EEPROM_SIZE);

  Serial.println("Getting passwords from EEPROM");

  // Try reading the values from the EEPROM
  ui_username = get_pswd(ui_username_offset);
  ui_pswd     = get_pswd(ui_pswd_offset);
  wifi_ssid   = get_pswd(ui_wifi_ssid_offset);
  wifi_pswd   = get_pswd(ui_wifi_pswd_offset);
  safename    = get_pswd(safename_offset);
  pinstr      = get_pswd(pin_offset);

  // Load the timer from SPIFFS
  read_timer();

  if (safename == "")
    safename="safe";

  if (pinstr != "")
    pin=pinstr.toInt();
  else
    pin=default_pin;

  // Set the safe state
  pinMode(pin,OUTPUT);
  digitalWrite(pin,LOW);

  // Ensure LED is off
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);

  // This is a debugging line; it's only sent to the serial
  // port which can only be accessed when the safe is unlocked.
  // We don't exposed passwords!
  Serial.println("Found in EEPROM:");
  Serial.println("  UI Username >>>"+ ui_username + "<<<");
  Serial.println("  UI Password >>>"+ ui_pswd + "<<<");
  Serial.println("  Wifi SSID   >>>"+ wifi_ssid + "<<<");
  Serial.println("  Wifi Pswd   >>>"+ wifi_pswd + "<<<");
  Serial.println("  Safename    >>>"+ safename + "<<<");
  Serial.println("  Relay Pin   >>>"+ String(pin) + "<<<");
  Serial.println("  Timer       >>>"+ String(timer) + "<<<");
  Serial.println("  Hidden      >>>"+ String(hidden) + "<<<");

  // Connect to the network
  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  wifi_connected=false;
  if (wifi_ssid != "")
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_pswd);
    Serial.print("Connecting to ");
    Serial.print(wifi_ssid); Serial.println(" ...");

    // Wait for the Wi-Fi to connect.  Give up after 60 seconds and
    // let us fall into AP mode
    int i = 0;
    while (WiFi.status() != WL_CONNECTED && i < 60)
    {
      Serial.print(++i); Serial.print(' ');
      delay(1000);
    }
    Serial.println('\n');
    wifi_connected = (WiFi.status() == WL_CONNECTED);
  }

  if (wifi_connected)
  {
    Serial.println("Connection established!");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname:\t");
    Serial.println(WiFi.hostname());
  }
  else
  {
    // Create an Access Point that mobile devices can connect to
    unsigned char mac[6];
    char macstr[7];
    WiFi.softAPmacAddress(mac);
    sprintf(macstr, "%02X%02X%02X", mac[3], mac[4], mac[5]);
    String AP_name="Safe-"+String(macstr);
    Serial.println("No connection; creating access point: "+AP_name);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_name);
  }

  //initialize mDNS service.
  MDNS.begin(safename);
  MDNS.addService("http", "tcp", 80);
  Serial.println("mDNS responder started");

  // This structure just lets us send all requests to the handler
  // If we can't handle it then send a 404 response
  server.onNotFound([]()
  {
    if (!handleRequest())
      server.send(404,"text/html","Not found");
  });

  // Start TCP (HTTP) server
  server.begin();
  
  Serial.println("TCP server started");

  // Configure the OTA update service, but don't start it yet!
  ArduinoOTA.setHostname(safename.c_str());

  if (ui_pswd != "")
    ArduinoOTA.setPassword(ui_pswd.c_str());

  ArduinoOTA.onStart([]()
  {
    Serial.println("Starting update");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd update");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  Serial.println("OTA service configured");
}

// A very simple timer loop; we just remember the last time
// and if the current time is different then we can reduce the
// time left

time_t last_time=time(NULL);

void loop()
{
  MDNS.update();
  server.handleClient();
  if (allow_updates)
    ArduinoOTA.handle();
  if (timer > 0)
  {
    time_t now=time(NULL);
    if (now != last_time)
    {
      last_time=now;
      // Serial.println("Tick");
      timer--;
      if (timer<0)
        timer=0;
      if (timer==0)
        hidden=0;
      if (timer%300 == 0)
        save_timer();
    }
  }
}
