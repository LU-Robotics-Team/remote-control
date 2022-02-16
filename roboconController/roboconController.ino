//Firmware for Lingnan University Robocon robot wireless controller
//Sends control commands to robot over Xbee using the Serial Transfer library
//For use with the Sparkfun wireless joystick kit configured with a joystick and four momentary buttons

#include <SerialTransfer.h> //Library to easily handle large amounts of data transfer

const int sendRate = 100; //ms, Time between messages, used to set data rate
const int JOYSTICK_X = A2;
const int JOYSTICK_Y = A3;
const int L_TRIGGER = 6;
const int R_TRIGGER = 3;
const int TURN_MAX = 15; //Maximum turn speed; 0 to 127
const int CTR_MIN = -127; //Minimum control speed; 0 to 127
const int CTR_MAX = 127;
const int STICK_MIN = 0; //Minimum value of joystick potentiometer
const int STICK_MAX = 1024;
const int DBY_MIN = 524; // Deadband lower bound of Y joystick
const int DBY_MAX = 536;
const int DBX_MIN = 514; //Deadband lower bound of X joystick
const int DBX_MAX = 526;
int lastSend = 0; //Time when last data sent

//Main struct to hold data to send over XBee
struct CTRL {
  int driveSpeed;
  int strafeSpeed;
  int turnSpeed;
} ctrlData;

SerialTransfer ctrlTransfer;

void setup()
{
  Serial1.begin(9600); //Serial connection to Xbee

  ctrlTransfer.begin(Serial1); //Begin Serial communication using SerialTransfer object
  delay(10);
  Serial1.print("W7001\r\n"); //Set the XBee bit in enable register 0x70
  pinMode(L_TRIGGER, INPUT_PULLUP);
  pinMode(R_TRIGGER, INPUT_PULLUP);
}

void loop()
{
  int stickX = analogRead(JOYSTICK_X);
  int stickY = analogRead(JOYSTICK_Y);
  bool turnL = digitalRead(L_TRIGGER);
  bool turnR = digitalRead(R_TRIGGER);

  //Read value of joystick in Y direction, if inside deadband read 0
  if (stickY > DBY_MIN && stickY < DBY_MAX) {
    ctrlData.driveSpeed = 0;
  }
  else {
    ctrlData.driveSpeed = map(stickY, STICK_MIN, STICK_MAX, CTR_MIN, CTR_MAX);
  }

  //Read value of joystick in Y direction, if inside deadband read 0
  if (stickX > DBX_MIN && stickX < DBX_MAX) {
    ctrlData.strafeSpeed = 0;
  }

  else {
    ctrlData.strafeSpeed = map(stickX, STICK_MIN, STICK_MAX, CTR_MAX, CTR_MIN); //Make left negative, right positive
  }

  //If left button pushed (read0), turn left
  if (turnL == 0 && turnR == 1) {
    ctrlData.turnSpeed = -TURN_MAX;
  }
  //If right button pushed (read0), turn right
  if (turnL == 1 && turnR == 0) {
    ctrlData.turnSpeed = TURN_MAX;
  }
  //If both buttons pushed or not pushed, do not turn
  if (turnL == turnR) {
    ctrlData.turnSpeed = 0;
  }

  //Send all data in ctrlData struct every sendRate milliseconds
  if (millis() - lastSend > sendRate) {
    int sendSize = 0; //record number of bytes in message to be sent
    lastSend = millis();
    sendSize = ctrlTransfer.txObj(ctrlData, sendSize);
    ctrlTransfer.sendData(sendSize);
  }
}
