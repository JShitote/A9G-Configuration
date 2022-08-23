/*
  Project: ESP32 development board, Ai Thinker A6 GSM GPRS module, DHT11
  Function: Temperature and humidity readings over SMS.
  Ai Thinker A6 GSM GPRS module powered by power supply 5V DC 2A

  Check for more DIY projects on acoptex.com

  Ai Thinker A6 GSM GPRS module -> ESP32 development board
  GND           GND
  U_RXD         TX2 (GPIO17)
  U_TXD         RX2 (GPIO16)

  There are three serial ports on the ESP known as U0UXD, U1UXD and U2UXD.

  U0UXD is used to communicate with the ESP32 for programming and during reset/boot.
  U1UXD is unused and can be used for your projects. Some boards use this port for SPI Flash access though
  U2UXD is unused and can be used for your projects.

  /*
  Example using the SparkFun HX711 breakout board with a scale
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 19th, 2014
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This is the calibration sketch. Use it to determine the calibration_factor that the main example uses. It also
  outputs the zero_factor useful for projects that have a permanent mass on the scale in between power cycles.

  Setup your scale and start the sketch WITHOUT a weight on the scale
  Once readings are displayed place the weight on the scale
  Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
  Use this calibration_factor on the example sketch

  This example assumes pounds (lbs). If you prefer kilograms, change the Serial.print(" lbs"); line to kg. The
  calibration factor will be significantly different but it will be linearly related to lbs (1 lbs = 0.453592 kg).

  Your calibration factor may be very positive or very negative. It all depends on the setup of your scale system
  and the direction the sensors deflect from zero state
  This example code uses bogde's excellent library:"https://github.com/bogde/HX711"
  bogde's library is released under a GNU GENERAL PUBLIC LICENSE
  Arduino pin 2 -> HX711 CLK
  3 -> DOUT
  5V -> VCC
  GND -> GND

  Most any pin on the Arduino Uno will be compatible with DOUT/CLK.

  The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.

*/
#include <stdio.h>
#include <string.h>
#include "DHT.h"            //library for DHT sensor
#include "HX711.h"

// Uncomment one of the lines below for whatever DHT sensor type you're using
#define DHTTYPE DHT11      // DHT 11
//#define DHTTYPE DHT21    // DHT 21 (AM2301)
//#define DHTTYPE DHT22    // DHT 22  (AM2302), AM2321
//#define DEBUG true
#define calibration_factor -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define LOADCELL_DOUT_PIN  19
#define LOADCELL_SCK_PIN  18

HX711 scale;

const int DHTPin = 2;      // DHT11 connected to D4 (GPIO4) pin of ESP32 development board
DHT dht(DHTPin, DHTTYPE);  // create an instance of the dht class called DHT
float h, t, f;             // Variables for temperature and humidity
const int MQ6 = 4;

const String apiKey = "FE1A89S55LRZNQXI";
String textMessage;        // Variable to store text message

void setup()
{
  Serial.begin(115200);                      // Initialise serial commmunication at 115200 bps
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial2.begin(115200, SERIAL_8N1, 16, 17); // Initialise serial commmunication at 115200 bps on U2UXD serial port
  dht.begin();                             // Initialize DHT sensor
  pinMode(DHTPin, INPUT);                  // Set DHTPin as INPUT
  delay(2000);              // Give time to your GSM module logon to GSM network

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
  Serial.println("Readings:");

  //A9G Tests
  Serial.println("GSMmodule ready...");  //Print test in Serial Monitor
  Serial2.println("AT\r"); // AT command to set module to SMS mode
  delay(100); // Set delay for 100 ms
  Serial2.println("AT+CMGF=1\r"); // AT command to set module to SMS mode
  delay(100); // Set delay for 100 ms
  Serial2.println("AT+CNMI=2,2,0,0,0\r");// Set module to send SMS data to serial out upon receipt
  delay(100); // Set delay for 100 ms
  Serial2.print("AT+CMGD=1,4\r");//delete all the message in the storage
  delay(100); // Set delay for 100 ms
  Serial2.print("AT+CGATT=1\r");//check if the chip is registered to the network
  delay(100); // Set delay for 100 ms
  Serial2.print("AT+CGACT=1,1\r");//check if the chip is registered to the network
  delay(100); // Set delay for 100 ms

  delay(100); // Set delay for 100 ms
  //Serial2.print("ATD*99***1#\r");//PPP session
  Serial2.print("AT+GPS=1\r");//setup for gps
  delay(100); // Set delay for 100 ms
  Serial2.print("AT+CGPSRST=1\r");//reset gps
  delay(100); // Set delay for 100 ms
  Serial2.print("AT+GPSRD=1\r");//Get location in NMEA format
  delay(100); // Set delay for 100 ms
  Serial2.print("AT+CGPSRST=1\r");//reset gps
  delay(100); // Set delay for 100 ms
  Serial2.print("AT+LOCATION=1\r");//Get location in longitude and latitude format
  delay(100); // Set delay for 100 ms
  delay(2000); // Set delay for 2 seconds
}

void loop()
{
  if (Serial.available() > 0)
    Serial2.write(Serial.read());
  if (Serial2.available() > 0)
    Serial.write(Serial2.read());
  if (Serial2.available() > 0) {
    textMessage = Serial2.readString();
    Serial.print(textMessage);
    delay(10);

  }

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  t = dht.readTemperature(); //Temperature in Celsius
  h = dht.readHumidity(); //Humidity
  f = dht.readTemperature(true); //Temperature in Fahrenheit
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  int MQ6Val = digitalRead(MQ6);
  Serial.println("Gas Concentration:  ");
  Serial.print(MQ6Val);

  float weight = (scale.get_units(), 1);
  Serial.println("Weight:  ");
  Serial.print(weight);

  if (textMessage.indexOf("Status") >= 0) {
    textMessage = "";
    String message1 = "Temperature (Celsius): " + String(t);
    //String message1 = "Temperature (Fahrenheit): " + String(f);
    String message2 = " Humidity: " + String(h);
    String message3 = " GasConc: " + String(MQ6Val);
    String message4 = " Weight: " + String(weight);
    String message = message1 + message2 + message3;
    sendSMS(message);
  }
  Serial2.print("AT+CGATT=1\r");//check if the chip is registered to the network
  delay(500);
  Serial2.print("AT+CGACT=1,1\r");//check if the chip is registered to the network
  delay(500);
  Serial2.print("AT+CGDCONT=1,\"IP\",\"safaricom\"\r");//APN configuration
  delay(10000);
  String cmdString = "AT+HTTPGET=\"http://api.thingspeak.com/update.json?api_key=" + apiKey + "&field1=" + t + "&field2=" + h + "&field3=" + MQ6Val + "&field4=" + weight + "\"";
  Serial2.println(cmdString);
}

// Function that sends SMS
void sendSMS(String message) {
  // REPLACE xxxxxxxxxxxx WITH THE RECIPIENT'S NUMBER
  // REPLACE ZZZ WITH THE RECIPIENT'S COUNTRY CODE
  Serial2.print("AT+CMGS = \"+254793627314\"\r");
  delay(100);
  // Send the SMS
  Serial2.print(message);
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  Serial2.println((char)26);
  delay(100);
  Serial2.println();
  // Give module time to send SMS
  delay(5000);


}

/*String sendData(String command, const int timeout, boolean debug)
  {
    String response = "";
    Serial.println(command);
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (Serial.available())
        {
            char c = Serial.read();
            response += c;
        }
    }
    if (debug)
    {
        Serial2.print(response);
    }
    return response;
  }*/
