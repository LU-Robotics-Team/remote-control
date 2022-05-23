//Firmware for Lingnan University Robocon robot wireless controller
//Sends control commands to robot over Xbee using the Serial Transfer library
//For use with the Sparkfun wireless joystick kit configured with a joystick and four momentary buttons

#include <SerialTransfer.h> //Library to easily handle large amounts of data transfer

const int sendRate = 100; //ms, Time between messages, used to set data rate
const int LEFT_XPIN = A2;
const int LEFT_YPIN = A3;
const int LEFT_TRIGPIN = 6;
const int RIGHT_TRIGPIN = 3;

int lastSend = 0; //Time when last data sent

//Main struct to hold data to send over XBee
struct CTRL {
  int stickX;
  int stickY;
  int trigL;
  int trigR;
} ctrlData;

SerialTransfer ctrlTransfer;

void setup()
{
  Serial1.begin(9600); //Serial connection to Xbee

  ctrlTransfer.begin(Serial1); //Begin Serial communication using SerialTransfer object
  delay(10);
  Serial1.print("W7001\r\n"); //Set the XBee bit in enable register 0x70
  pinMode(LEFT_TRIGPIN, INPUT_PULLUP);
  pinMode(RIGHT_TRIGPIN, INPUT_PULLUP);
}

void loop()
{
  ctrlData.stickX = analogRead(LEFT_XPIN);
  ctrlData.stickY = analogRead(LEFT_YPIN);
  ctrlData.trigL = digitalRead(LEFT_TRIGPIN);
  ctrlData.trigR = digitalRead(RIGHT_TRIGPIN);
  

  //Send all data in ctrlData struct every sendRate milliseconds
  if (millis() - lastSend > sendRate) {
    int sendSize = 0; //record number of bytes in message to be sent
    lastSend = millis();
    sendSize = ctrlTransfer.txObj(ctrlData, sendSize);
    ctrlTransfer.sendData(sendSize);
  }
}
