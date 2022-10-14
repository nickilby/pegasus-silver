#include "DHT.h"
#include <Arduino.h>
#include <Adafruit_Sensor.h>

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
int MIN = 23;
int MAX = 25;

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
  Serial.println("Initialising System ");
  delay(1000);
  Serial.println("Initialising System . ");
  delay(1000);
  Serial.println("Initialising System . . ");
  delay(1000);
  Serial.println("Initialising System . . . ");
  delay(1000);
  Serial.println("Initialising System . . . .");
  delay(1000);
  Serial.println("System Initialised Complete");
  delay(2000);

// START THE SENSORS
  dht1.begin();
  dht2.begin();
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

void RGB_sensor1()
{
  float t1 = dht1.readTemperature();
    if (t1 < MIN) {
    digitalWrite(RGB_RED1, 0);
    digitalWrite(RGB_GREEN1, 0);
    digitalWrite(RGB_BLUE1, 255); // BLUE
    digitalWrite(RELAY1, LOW);
    Serial.println("SENSOR 1 COLD");
    Serial.println(F("FAN 1 OFF"));
  }
    else if (t1 > MAX) {
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


void RGB_sensor2()
{
  float t2 = dht2.readTemperature();
    if (t2 < MIN) {
    digitalWrite(RGB_RED2, 0);
    digitalWrite(RGB_GREEN2, 0);
    digitalWrite(RGB_BLUE2, 255); // BLUE
    digitalWrite(RELAY2, LOW);
    digitalWrite(FANINTERNAL, LOW);
    Serial.println("SENSOR 2 COLD");
    Serial.println(F("FAN 2 OFF"));
  }
    else if (t2 > MAX) {
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
    delay(30000); // DELAY BEFORE RESTARTING LOOP
}
