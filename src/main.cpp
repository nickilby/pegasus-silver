#include "DHT.h"
#include "Arduino.h"
#include "Adafruit_Sensor.h"
#include "WiFiEspAT.h"
#include "wifi_secrets.h"
#include "aWOT.h"
#include "SPI.h"

//// Setup Timing

#define eventInterval1 15000 // 15 Seconds
#define eventInterval2 300000 // 1 Mintues  - 300000 = 5 minutes - 600000 = 10 minutes
#define eventInterval3 10000 // 10 Seconds
unsigned long previousTime1 = 0;
unsigned long previousTime2 = 0;
unsigned long previousTime3 = 0;

////// Start Wifi Setup
WiFiServer server(80);
Application app;
// Update firwmare for ESP 8266 with thses instructions with Mega R3 Built in Wifi board
// https://dcc-ex.com/reference/hardware/microcontrollers/wifi-mega.html

/////// Please enter your sensitive data in the Secret tab/wifi_secrets.h
const char ssid[] = SECRET_SSID;    // your network SSID (name)
const char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// Emulate Serial1 on pins 6/7 if not present
#if defined(ARDUINO_ARCH_AVR) && !defined(HAVE_HWSERIAL1)
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#define AT_BAUD_RATE 9600
#else
#define AT_BAUD_RATE 115200
#endif

////// Overide Switch

// Overide Function - Manually switch to AC or Free Air
#define OVERIDEAC 7
#define OVERIDEFREEAIR 27

////// DHT Sensors

// Hot Aisle
#define DHT1PIN 28
// Cold Aisle 
#define DHT2PIN 36
// External
#define DHT3PIN 44

float t1 = 0;
float h1 = 0;
float t2 = 0;
float h2 = 0;
float t3 = 0;
float h3 = 0;

////// SENSOR TYPE

#define DHT1TYPE DHT11
// DHT11   Pin1 = Ground, Pin2 = Signal, Pin3 = 5v

DHT dht1(DHT1PIN, DHT1TYPE);
DHT dht2(DHT2PIN, DHT1TYPE);
DHT dht3(DHT3PIN, DHT1TYPE);

//// SET TEMPERATURE THRESHOLDS

// Cold Aisle
int MIN1 = 10; // Fan Turns On above this
int MAX1 = 23; // Could turn on 2nd Fan is this is reached
// Hot Aisle
int MIN2 = 18; // Louvre Opens Up above this
int MAX2 = 35; // Room Getting to hot, fall back to AC.
// External
int MIN3 = 1; // 
int MAX3 = 15; // AC is On above this / Free Cooling below this

//// SET HUMIDITY THRESHOLDS

int HMIN1 = 35; // Freecooling is only possible between these 2 values
int HMAX1 = 75; // On AC above this value - Not to pull in outside humid air

/////// RELAYs
// Fan #1
#define RELAY1 46
// Fan #2
#define RELAY2 38
// Actuator
#define RELAY3 30
// AC Unit
#define RELAY4 22
// Neutral to Center terminal
// Live to bottom terminal = LOW/OFF NC
// Live to top terminal = HIGH/OFF NO

int fan1 = 0;
int fan2 = 0;
int actuator = 0;
int ac = 0;
int switchValue1 = 1;
int switchValue2 = 1;

//// Not Used at the Moment Should consider WS2815 as only require 1 data pin not 3

// // RGB LEDs SENSOR 1
// #define RGB_RED1 9
// #define RGB_GREEN1 10
// #define RGB_BLUE1 11

// // RGB LEDs SENSOR 2
// #define RGB_RED2 12
// #define RGB_GREEN2 13
// #define RGB_BLUE2 14

////// Template Prometheus Scrape Page

String readString;

void indexCmd(Request &req, Response &res)
{
  Serial.println("Request for index");
  res.set("Content-Type", "text/html");
  res.println("<html>");
  res.println("<head>");
  res.println("  <meta http-equiv=\"refresh\" content=\"5\">");
  res.println("</head>");
  res.println("<body>");
  res.println("  <H1> Hot Aisle Temp: " + String(t1) + "</p>");
  res.println("  <H1> Cold Aisle Temp: " + String(t2) + "</p>");
  res.println("  <H1> External Temp: " + String(t3) + "</p>");
  res.println("</body>");
  res.println("</html>");
}

void contolCmd(Request &req, Response &res)
{
  Serial.println("Request for index");
  res.set("Content-Type", "text/html");
  res.println("<html>");
  res.println("<head>");
  res.println("<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />");
  res.println("<TITLE>Server Room Cooling Mode</TITLE>");
  res.println("</head>");
  res.println("<body>");
  res.println("<H1>Server Room Cooling Mode!</H1>");
  res.println("<hr />");
  res.println("<H2>Manauly Set the Cooling Mode of the Room!!</H2>");
  res.println("<h4>AC - State: " +  String(ac) + "</h4>");
  res.println("<h4>Fan1 - State: " +  String(fan1) + "</h4>");
  res.println("<h4>Fan2 - State: " +  String(fan2) + "</h4>");
  res.println("<h4>Actuator - State: " +  String(actuator) + "</h4>");
    if(ac == 0){
      res.println("<a href=\"/ac_on\"\">AC On</a>");
      // ac_on;
    }
    else if(ac == 1){
      res.println("<a href=\"/ac_off\"\">AC Off</a>");
      // room_mode();                                                              
    }
    if(fan1 == 0){
      res.println("<a href=\"/free_air_on\"\">Free Air On</a>");
      // freecooling_turbo();
    }
    else if(fan1 == 1){
      res.println("<a href=\"/free_air_off\"\">Free Air Off</a>");
      // room_mode();                                                          
    }
  res.println("<a href=\"/?auto\"\">Auto Mode</a>");
  res.println("<p>Created by Nic Kilby</p> ");
  res.println("</BODY>");
  res.println("</HTML>");
}

void metricsCmd(Request &req, Response &res)
{
  Serial.println("Request for metrics");
  res.set("Content-Type", "text/plain");
  res.print("# HELP temperature is the last temperature reading in degrees celsius\n");
  res.print("# TYPE temp gauge\n");
  res.print("temperature{instance=\"Hot Aisle\"} " + String(t1) + "\n");
  res.print("temperature{instance=\"Cold Aisle\"} " + String(t2) + "\n");
  res.print("temperature{instance=\"External\"} " + String(t3) + "\n");
  res.print("# HELP humidity is the last relative humidity reading as a percentage\n");
  res.print("# TYPE humidity gauge\n");
  res.print("humidity{instance=\"Hot Aisle\"} " + String(h1) + "\n");
  res.print("humidity{instance=\"Cold Aisle\"} " + String(h2) + "\n");
  res.print("humidity{instance=\"External\"} " + String(h3) + "\n");
  res.print("# HELP Status returns of the Relay Pins\n");
  res.print("# TYPE Relay status gauge\n");
  res.print("relay{instance=\"Fan1\"} " + String(fan1) + "\n");
  res.print("relay{instance=\"Fan2\"} " + String(fan2) + "\n");
  res.print("relay{instance=\"Actuator\"} " + String(actuator) + "\n");
  res.print("relay{instance=\"AC\"} " + String(ac) + "\n");
}

void printWifiStatus() 
{
  // print the SSID of the network you're attached to:
  char ssid[33];
  WiFi.SSID(ssid);
  Serial.print("SSID: ");
  Serial.println(ssid);

  // print your board's IP address and gateway
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  IPAddress gw = WiFi.gatewayIP();
  Serial.print("Gateway: ");
  Serial.println(gw);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void readSensor() // Read the 3 sensors
{
  h1 = dht1.readHumidity();
  t1 = dht1.readTemperature();
  h2 = dht2.readHumidity();
  t2 = dht2.readTemperature();
  h3 = dht3.readHumidity();
  t3 = dht3.readTemperature();
  if (isnan(h1) || isnan(t1))
  {
    Serial.println("Failed to read from DHT #1 Hot Aisle");
    return;
  }
  if (isnan(h2) || isnan(t2))
  {
    Serial.println("Failed to read from DHT #2 Cold Aisle");
    return;
  }
  if (isnan(h3) || isnan(t3))
  {
    Serial.println("Failed to read from DHT #3 External");
    return;
  }
}

void serialPrintReadings()
{
    Serial.print("Humidity Hot Aisle: "); 
    Serial.print(h1);
    Serial.print(" %\t");
    Serial.print("Temperature Hot Aisle: "); 
    Serial.print(t1);
    Serial.println(" *C");
    Serial.print("Humidity Cold Aisle: "); 
    Serial.print(h2);
    Serial.print(" %\t");
    Serial.print("Temperature Cold Aisle: "); 
    Serial.print(t2);
    Serial.println(" *C");
    Serial.print("Humidity External: "); 
    Serial.print(h3);
    Serial.print(" %\t");
    Serial.print("Temperature External: "); 
    Serial.print(t3);
    Serial.println(" *C");
}

void readPins() // Read the Status of the pins to populate Prometheus data
{
  switchValue1 = digitalRead(7);
  switchValue2 = digitalRead(27);
  fan1 = digitalRead(46);
  fan2 = digitalRead(38);
  actuator = digitalRead(30);
  ac = digitalRead(22);
}

void setup() 
{
// RELAY PIN SETUP
  pinMode (46 , OUTPUT);
  pinMode (38 , OUTPUT);
  pinMode (30 , OUTPUT);
  pinMode (22 , OUTPUT);

// SWITCH PIN SETUP
  pinMode(7 , INPUT_PULLUP );
  pinMode(27 , INPUT_PULLUP );

// // LED PIN SETUP // Not in use at the moment
//   pinMode (9 , OUTPUT);
//   pinMode (10 , OUTPUT);
//   pinMode (11 , OUTPUT);
//   pinMode (12 , OUTPUT);
//   pinMode (13 , OUTPUT);
//   pinMode (14 , OUTPUT);

  Serial.begin(9600); 
  Serial3.begin(AT_BAUD_RATE);
  WiFi.init(Serial3);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println();
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  WiFi.setPersistent(); // Set the following WiFi connection as persistent to survive reboots

//  Uncomment these lines for persistent static IP
  IPAddress ip(10, 128, 83, 8);
  IPAddress dns(10, 128, 83, 1);
  IPAddress gw(10, 128, 83, 1);
  IPAddress nm(255, 255, 255, 128);
  WiFi.config(ip, dns, gw, nm);

  Serial.println();
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  int status = WiFi.begin(ssid, pass);

  if (status == WL_CONNECTED) {
    Serial.println();
    Serial.println("Connected to WiFi network.");
    Serial.print("To access the server, enter \"http://");
    Serial.print(ip);
    Serial.println("/metrics\" in web browser.");
  } else {
    WiFi.disconnect(); // remove the WiFi connection
    Serial.println();
    Serial.println("Connection to WiFi network failed.");
  }

// START THE SENSORS
  dht1.begin();
  dht2.begin();
  dht3.begin();

// WebServer
  app.get("/", &indexCmd);
  app.get("/metrics", &metricsCmd);
  app.get("/contol", &contolCmd);
  server.begin();
}

///// ACTIONS
void ac_on() // AC Mode
{
  digitalWrite(RELAY1, LOW); // Internal Fan1 Off
  digitalWrite(RELAY2, LOW); // Internal Fan2 Off
  digitalWrite(RELAY3, LOW); // Louvre Closed = 0
  digitalWrite(RELAY4, LOW); // AC On
}
void freecooling() // Free Cooling Mode
{
  digitalWrite(RELAY1, HIGH); // Internal Fan1 On
  digitalWrite(RELAY2, LOW); // Internal Fan2 Off
  digitalWrite(RELAY3, HIGH); // Louvre Open = 1
  digitalWrite(RELAY4, HIGH); // AC Off
}
void freecooling_turbo() // Free Cooling Mode with extra power
{
  digitalWrite(RELAY1, HIGH); // Internal Fan1 On
  digitalWrite(RELAY2, HIGH); // Internal Fan2 On
  digitalWrite(RELAY3, HIGH); // Louvre Open = 1
  digitalWrite(RELAY4, HIGH); // AC Off
}
void passive_cooling() // Free Cooling without fans
{
  digitalWrite(RELAY1, LOW); // Internal Fan1 Off
  digitalWrite(RELAY2, LOW); // Internal Fan2 Off
  digitalWrite(RELAY3, HIGH); // Louvre Open = 1
  digitalWrite(RELAY4, HIGH); // AC Off;
}

void room_mode() // Control the AC Units
{
  if (t3 >= MAX3) // If it is too warm outside - AC ON
  {
    ac_on();
    Serial.print("External Temperature is Above ");
    Serial.print(MAX3);
    Serial.println(F(" *C - Room in AC Cooling Mode"));
  } 

  else if (h3 >= HMAX1) // If it is too humid outside - AC ON
  {
    ac_on();
    Serial.print("External Humdity is Above ");
    Serial.print(HMAX1);
    Serial.println(F(" % - Room in AC Cooling Mode"));
  } 

  else if (t1 >= MAX2) // If the Hot Aisle too hot - AC ON
  {
    ac_on();
    Serial.print("Hot Aisle Temperature is Above ");
    Serial.print(MAX2);
    Serial.println(F(" *C - Room in AC Cooling Mode"));
  }

  else if (t2 <= MIN1) // If the Cold Aisle too cold - Turn off fans
  {
    passive_cooling();
    Serial.print("Cold Aisle Temperature is Below ");
    Serial.print(MIN1);
    Serial.println(F(" *C - Room in Passive Cooling Mode"));
  }
  
  else // All Conditions are Correct to use Free Colling
  {
    freecooling();
    Serial.println(F("Room in Free Cooling Mode"));
  }
}

void loop()
{
  // Keep track of time elaspsed
  unsigned long currentTime = millis();
  // Monitor readings and Provide Metrics
  WiFiClient client = server.available();
  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
  if (millis() >= previousTime1 + eventInterval1)
  {
    Serial.println(F("------------------------------------"));
    Serial.println();
    readSensor();
    readPins();
    printWifiStatus();
    serialPrintReadings();

    Serial.print("switchValue1 =  ");
    Serial.println( switchValue1 );
    Serial.print("switchValue2 =  ");
    Serial.println( switchValue2 );
    previousTime1 = currentTime;
  }
  if (readString.indexOf("/?ac_on") > 0) 
    ac_on();
  if (readString.indexOf("/?ac_off") > 0) 
    room_mode();
  if (readString.indexOf("/?free_air_on") > 0) 
    freecooling_turbo();
  if (readString.indexOf("/?free_air_off") > 0) 
    room_mode();
  if (readString.indexOf("/?auto") > 0) 
    room_mode();
  // Manual Overide Switch Poistions
  if (millis() >= previousTime3 + eventInterval3)
  {
    if (switchValue1 == 0)
    {
      ac_on();
      Serial.print("-Manual Overide AC On-");
      Serial.println("-switchValue1 == 0-");
    }
    else if (switchValue2 == 0)
    { 
      freecooling_turbo();
      Serial.print("-Manual Overide FreeCooling On-");
      Serial.println("-switchValue2 == 0-");
    }
    else if (millis() >= previousTime2 + eventInterval2)
    {
      room_mode();
      Serial.println(F("------------------------------------"));
      Serial.println("-Auto Mode-");
      Serial.println(F("------------------------------------"));
      previousTime2 = currentTime;
    }
    previousTime3 = currentTime;
  }
}