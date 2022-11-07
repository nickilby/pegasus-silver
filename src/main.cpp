#include "DHT.h"
#include "Arduino.h"
#include "Adafruit_Sensor.h"
#include "WiFiEspAT.h"
#include "wifi_secrets.h"

////// Wifi Setup
// Update firwmare fro WSP 8266 with thses instructions
// https://dcc-ex.com/reference/hardware/microcontrollers/wifi-mega.html

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
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
////// Wifi Setup

// DHT Sensors
#define DHT1PIN 2
#define DHT2PIN 3

// RELAY
#define RELAY1 4
#define RELAY2 5
// Neutral to Center terminal
// Live to bottom terminal = LOW/OFF NC
// Live to top terminal = HIGH/OFF NO

// RGB LEDs SENSOR 1
#define RGB_RED1 7
#define RGB_GREEN1 8
#define RGB_BLUE1 9

// RGB LEDs SENSOR 2
#define RGB_RED2 10
#define RGB_GREEN2 11
#define RGB_BLUE2 12

// INTERNAL FAN
#define FANINTERNAL 6

// SENSOR TYPE
#define DHT1TYPE DHT11
// DHT11   Pin1 = Ground, Pin2 = Signal, Pin3 = 5v

DHT dht1(DHT1PIN, DHT1TYPE);
DHT dht2(DHT2PIN, DHT1TYPE);

// SET TEMP THRESHOLDS
int MIN1 = 20;
int MAX1 = 25;

int MIN2 = 15;
int MAX2 = 22;

void setup() {

// RELAY PIN SETUP
  pinMode (4 , OUTPUT);
  pinMode (5 , OUTPUT);

// 12V FAN PIN SETUP
  pinMode (6 , OUTPUT);

// LED PIN SETUP
  pinMode (7 , OUTPUT);
  pinMode (8 , OUTPUT);
  pinMode (9 , OUTPUT);
  pinMode (10 , OUTPUT);
  pinMode (11 , OUTPUT);
  pinMode (12 , OUTPUT);

  Serial.begin(9600); 
  Serial3.begin(AT_BAUD_RATE);
  WiFi.init(Serial3);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println();
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  WiFi.setPersistent(); // set the following WiFi connection as persistent

//  uncomment this lines for persistent static IP. set addresses valid for your network
//  IPAddress ip(192, 168, 88, 150);
//  IPAddress gw(192, 168, 88, 1);
//  IPAddress nm(255, 255, 255, 0);
//  WiFi.config(ip, gw, gw, nm);

  Serial.println();
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  int status = WiFi.begin(ssid, pass);

  if (status == WL_CONNECTED) {
    Serial.println();
    Serial.println("Connected to WiFi network.");
  } else {
    WiFi.disconnect(); // remove the WiFi connection
    Serial.println();
    Serial.println("Connection to WiFi network failed.");
  }

// START THE SENSORS
  dht1.begin();
  dht2.begin();
}

void printWifiStatus() {

  // print the SSID of the network you're attached to:
  char ssid[33];
  WiFi.SSID(ssid);
  Serial.print("SSID: ");
  Serial.println(ssid);

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

void readSensor()
{
  float h1 = dht1.readHumidity();
  float t1 = dht1.readTemperature();
  float h2 = dht2.readHumidity();
  float t2 = dht2.readTemperature();

  if (isnan(t1) || isnan(h1)) {
    Serial.println("Failed to read from DHT #1");
  } else {
    Serial.print("Humidity 1: "); 
    Serial.print(h1);
    Serial.print(" %\t");
    Serial.print("Temperature 1: "); 
    Serial.print(t1);
    Serial.println(" *C");
  }
  Serial.println();
  if (isnan(t2) || isnan(h2)) {
    Serial.println("Failed to read from DHT #2");
  } else {
    Serial.print("Humidity 2: "); 
    Serial.print(h2);
    Serial.print(" %\t");
    Serial.print("Temperature 2: "); 
    Serial.print(t2);
    Serial.println(" *C");
  }
  // Serial.println(F("------------------------------------"));
}

void RGB_sensor1() // Internal Reading
{
  float t1 = dht1.readTemperature();
    if (t1 < MIN1) {
    digitalWrite(RGB_RED1, 0);
    digitalWrite(RGB_GREEN1, 0);
    digitalWrite(RGB_BLUE1, 255); // BLUE
    digitalWrite(RELAY1, LOW);
    Serial.println("SENSOR 1 COLD");
    Serial.println(F("FAN 1 OFF"));
  }
    else if (t1 > MAX1) {
    digitalWrite(RGB_RED1, 255); // RED
    digitalWrite(RGB_GREEN1, 0);
    digitalWrite(RGB_BLUE1, 0);
    digitalWrite(RELAY1, HIGH);
    Serial.println("SENSOR 1 HOT");
    Serial.println(F("FAN 1 ON"));
  }
    else {
    digitalWrite(RGB_RED1, 0);
    digitalWrite(RGB_GREEN1, 255); // Green
    digitalWrite(RGB_BLUE1, 0);
    digitalWrite(RELAY1, LOW);
    Serial.println("SENSOR 1 OK");
    Serial.println(F("FAN 1 OFF"));
  }
}


void RGB_sensor2()  // External Reading
{
  float t2 = dht2.readTemperature();
    if (t2 < MIN2) {
    digitalWrite(RGB_RED2, 0);
    digitalWrite(RGB_GREEN2, 0);
    digitalWrite(RGB_BLUE2, 255); // BLUE
    digitalWrite(RELAY2, LOW);
    digitalWrite(FANINTERNAL, LOW);
    Serial.println("SENSOR 2 COLD");
    Serial.println(F("FAN 2 OFF"));
  }
    else if (t2 > MAX2) {
    digitalWrite(RGB_RED2, 255); // RED
    digitalWrite(RGB_GREEN2, 0);
    digitalWrite(RGB_BLUE2, 0);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(FANINTERNAL, LOW);
    Serial.println("SENSOR 2 HOT");
    Serial.println(F("FAN 2 ON"));
  }
    else {
    digitalWrite(RGB_RED2, 0);
    digitalWrite(RGB_GREEN2, 255); // Green
    digitalWrite(RGB_BLUE2, 0);
    digitalWrite(RELAY2, LOW);
    digitalWrite(FANINTERNAL, HIGH);
    Serial.println("SENSOR 2 OK");
    Serial.println(F("FAN 2 OFF"));
  }
}

void loop()
{
    Serial.println(F("------------------------------------"));
    readSensor();
    Serial.println(F("------------------------------------"));
    RGB_sensor1();
    Serial.println(F("------------------------------------"));
    RGB_sensor2();
    Serial.println(F("------------------------------------"));
    Serial.println();
    printWifiStatus();
    delay(10000); // DELAY BEFORE RESTARTING LOOP
}
