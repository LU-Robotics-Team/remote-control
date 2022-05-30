//Firmware for Lingnan University Robocon robot wireless controller
//Receives control commands to robot over Xbee using the Serial Transfer library
//Calculates motor speeds and sends to Sabertooth motor controller

//As of 23 March 2022 (Redboard Turbo board definition vs 1.8.6),
//must update board definitions for Sparkfun Redboard Turbo, see:
//https://github.com/sparkfun/Arduino_Boards/pull/96

#include <SerialTransfer.h>
#include <Sabertooth.h>
#include "wiring_private.h" // pinPeripheral() function D2-TX, D3-RX

const bool DEBUG = true;

const int LEFT = 1;
const int RIGHT = 2;
const int GRIP = 1;
const int VERT = 2;
const int TURN_MIN = -60;
const int TURN_MAX = 60; //Maximum turn speed; 0 to 127
const int DRIVE_MIN = -90; //Minimum control speed; 0 to 127
const int DRIVE_MAX = 90;
const int STICK_MIN = 0; //Minimum value of joystick potentiometer
const int STICK_MAX = 1024;
const int DBY_MIN = 524; // Deadband lower bound of Y joystick
const int DBY_MAX = 536;
const int DBX_MIN = 514; //Deadband lower bound of X joystick
const int DBX_MAX = 526;
const int GRIP_MAX = 50;
const int VERT_MAX = 50;

int driveSpeed;
int turnSpeed;

//Main struct to hold data to send over XBee
struct CTRL {
  int stickX;
  int stickY;
  int padU;
  int padR;
  int padL;
  int padD;
} ctrlData;

SerialTransfer ctrlTransfer;

//Create Serial2 port on sercom2. D2->RX, D3->TX
//See https://learn.sparkfun.com/tutorials/adding-more-sercom-ports-for-samd-boards
Uart Serial2 (&sercom2, 3, 2, SERCOM_RX_PAD_1, UART_TX_PAD_2);
void SERCOM2_Handler()
{
  Serial2.IrqHandler();
}

Sabertooth ST1(128, Serial2);
Sabertooth ST2(129, Serial2);


void setup() {
  if (DEBUG == true) {
    SerialUSB.begin(9600); //debugging
  }
  Serial1.begin(9600); //XBEE
  Serial2.begin(9600); //Sabertooth

  pinPeripheral(2, PIO_SERCOM);
  pinPeripheral(3, PIO_SERCOM_ALT);

  ctrlTransfer.begin(Serial1);
}

void loop() {

  if (ctrlTransfer.available()) {
    int recSize = 0;
    recSize = ctrlTransfer.rxObj(ctrlData, recSize);

    //Read value of joystick in Y direction, if inside deadband read 0
    if (ctrlData.stickY > DBY_MIN && ctrlData.stickY < DBY_MAX) {
      driveSpeed = 0;
    }

    else {
      driveSpeed = map(ctrlData.stickY, STICK_MIN, STICK_MAX, DRIVE_MIN, DRIVE_MAX);
    }

    //Read value of joystick in X direction, if inside deadband read 0
    if (ctrlData.stickX > DBX_MIN && ctrlData.stickX < DBX_MAX) {
      turnSpeed = 0;
    }

    else {
      turnSpeed = map(ctrlData.stickX, STICK_MIN, STICK_MAX, TURN_MAX, TURN_MIN); //Make left negative, right positive
    }

    int denominator = max(abs(driveSpeed) + abs(turnSpeed), DRIVE_MAX);
    int leftSpeed = (driveSpeed + turnSpeed) * DRIVE_MAX / denominator;
    int rightSpeed = (driveSpeed - turnSpeed) * DRIVE_MAX / denominator;

    ST1.motor(LEFT, leftSpeed);
    ST1.motor(RIGHT, rightSpeed);


    if (ctrlData.padU != ctrlData.padR) {
      if (ctrlData.padU == LOW) {
      ST2.motor(VERT, VERT_MAX);
      }
      else {
        ST2.motor(VERT, -VERT_MAX);
      }
    }
    else {
      ST2.motor(VERT, 0);
    }

    if (ctrlData.padL != ctrlData.padD) {
      if (ctrlData.padL == LOW) {
      ST2.motor(GRIP, GRIP_MAX);
      }
      else {
        ST2.motor(GRIP, -GRIP_MAX);
      }
    }
    else {
      ST2.motor(GRIP, 0);
    }

    if (DEBUG == true) {
      char dat1[32];
      sprintf(dat1, "Drive:%i,Turn:%i\r\n", driveSpeed, turnSpeed);
      SerialUSB.println(dat1);
      SerialUSB.print(leftSpeed);
      SerialUSB.print("    ");
      SerialUSB.println(rightSpeed);
      SerialUSB.println(ctrlData.padU);
      SerialUSB.println(ctrlData.padL);
      SerialUSB.println(ctrlData.padR);
      SerialUSB.println(ctrlData.padD);
    }

  }
  else if (ctrlTransfer.status < 0) {
    if (DEBUG == true) {
      SerialUSB.print("Error");
    }
  }
}
