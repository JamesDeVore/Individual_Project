//REMEMBER TO HAVE THE OTHER TABS Open//packet parser and BLE BluefruitConfig

#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <TinyGPS++.h>
#include <Wire.h>
#include "RTClib.h"
#include "BluefruitConfig.h"

#define FACTORYRESET_ENABLE 1
#define MINIMUM_FIRMWARE_VERSION "0.6.6"
#define MODE_LED_BEHAVIOUR "MODE"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

//file name to save GPS data
String fileName = "GPSDATA.TXT";

//start the file Object
File myFile;

//** packet parser
// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t *data, const uint32_t numBytes);
extern uint8_t packetbuffer[];

//RTC object

RTC_PCF8523 rtc;

//GPS
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

//other helpers for blink without delay
unsigned long previousMillis = 0;

const long interval = 1000;
//manages what the unit is actually doing
int mode = 0;
//for the timer function
long number = 0;

void setup(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  //this is hopefully not necessary, but works bettwe with it
  //   while (!Serial);
  //   ; // required for Flora & Micro
  delay(500);

  Serial.begin(115200);

  Serial.println("Hey there, setup function has begun");

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if (!ble.begin(VERBOSE_MODE))
  {
    //error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println(F("OK!"));

  if (FACTORYRESET_ENABLE)
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if (!ble.factoryReset())
    {
      //error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  ble.verbose(false); // debug info is a little annoying after this point!
  Serial.println("Waiting on connection . . . ");
  /* Wait for connection */

  Serial.println(F("Connected to device"));

  // LED Activity command is only supported from 0.6.6
  if (ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION))
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  Serial.println(F("Switching to DATA mode!"));
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

  Serial.print("Initializing SD card...");
  ble.println("INitializing SD...");

  if (!SD.begin(10))
  {
    Serial.println("initialization failed!");
    while (1)
      ;
  }
  Serial.println("initialization done. Card found");
  ble.println("initialization done. Card found, starting GPS setup");
  //GPS set up function down below
  GPSSetup();
  ble.println("GPS set up complete");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5000);
  digitalWrite(LED_BUILTIN, LOW);
  ble.print("connected, about to loop");
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  unsigned long currentMillis = millis();
  //blink without delay
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    modeBlink();
  }

  Serial.println(mode);
  checkForInput();

  if (mode == 1)
  {
    readAndSendFile();
    mode = 0;
  }

  if (mode == 2)
  {
    streamData();
  }

  if (mode == 3)
  {
    GPSReadAndLog();
  }
  if (mode == 4)
  {
    if (SD.exists(fileName))
    {
      SD.remove(fileName);
    }
  }
}

void GPSSetup()
{

  Serial.begin(9600);
  Serial1.begin(GPSBaud);
  rtc.begin();
}

void streamData()
{
  //RTC date object
  DateTime now = rtc.now();

  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))
    {

      if (gps.location.isValid())
      {
        Serial.println("Valid");
        String dataString;
        //String latitude = String(gps.location.lat(), 6);
        //String longitude = String(gps.location.lng(), 6);

        String currentUnixTime = String(now.unixtime());
        String currentAltitude = String(gps.altitude.meters());
        String currentSpeed = String(gps.speed.mps());
        dataString.concat(String(gps.location.lat(), 6));
        dataString.concat(',');
        dataString.concat('N');
        dataString.concat(',');
        dataString.concat(String(gps.location.lng(), 6));
        dataString.concat(',');
        dataString.concat('W');
        dataString.concat(',');
        dataString.concat(currentAltitude);
        dataString.concat(',');
        dataString.concat(currentSpeed);
        dataString.concat(',');
        dataString.concat(currentUnixTime);
        dataString.concat('$');
        Serial.println(dataString);

        for (int i = 0; i < dataString.length(); i++)
        {
          ble.print(dataString[i]);
          delay(20);
        }
      }
    }
}
void readAndSendFile()
{
  myFile = SD.open(fileName);
  if (myFile)
  {
    Serial.println("opened dataFile, attempting to send data");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    ble.print("@");
    digitalWrite(LED_BUILTIN, LOW);
    delay(2000);

    // read from the file until there's nothing else in it:
    while (myFile.available())
    {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println(myFile.peek());
      ble.write(myFile.read());
      digitalWrite(LED_BUILTIN, LOW);
      delay(10);
    }
    // close the file:
    myFile.close();
    ble.print("#");
    Serial.println("DONE");
    mode = 0;
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  Serial.println("done reading the file");
}

void GPSReadAndLog()
{
  //RTC date object
  DateTime now = rtc.now();

  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))
    {

      if (gps.location.isValid())
      {
        String dataString;
        //String latitude = String(gps.location.lat(), 6);
        //String longitude = String(gps.location.lng(), 6);

        String currentUnixTime = String(now.unixtime());
        String currentAltitude = String(gps.altitude.meters());
        String currentSpeed = String(gps.speed.mph());
        dataString.concat(String(gps.location.lat(), 6));
        dataString.concat(',');
        dataString.concat('N');
        dataString.concat(',');
        dataString.concat(String(gps.location.lng(), 6));
        dataString.concat(',');
        dataString.concat('W');
        dataString.concat(',');
        dataString.concat(currentAltitude);
        dataString.concat(',');
        dataString.concat(currentSpeed);
        dataString.concat(',');
        dataString.concat(currentUnixTime);
        dataString.concat('$');
        ble.print(dataString);
        myFile = SD.open(fileName, FILE_WRITE);
        if (myFile)
        {
          //save the textstring to the SD card
          Serial.println(dataString);
          myFile.println(dataString);
          // close the file:
          myFile.close();
          ble.print("data saved");
        }
        else
        {
          // if the file didn't open, print an error:
          Serial.println("error opening test.txt");
        }
      }
      break;
    }
}

void checkForInput()
{
  // Check for user input
  char n, inputs[BUFSIZE + 1];

  if (Serial.available())
  {
    n = Serial.readBytes(inputs, BUFSIZE);
    inputs[n] = 0;
    // Send characters to Bluefruit
    Serial.print("Sending: ");
    Serial.println(inputs);

    // Send input data to host via Bluefruit
    ble.print(inputs);
  }

  // Echo received data
  while (ble.available())
  {
    int c = ble.read();
    mode = c;
    confirmBlink();
    setup();
  }
}

void modeBlink()
{
  for (int i = 0; i < mode; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
  }
}
void confirmBlink()
{
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}