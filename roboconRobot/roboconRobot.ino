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

const int CTRL_MAX = 50;
const int LEFT = 1;
const int RIGHT = 2;

struct CTRL {
  int driveSpeed;
  int strafeSpeed;
  int turnSpeed;
} ctrlData;

SerialTransfer ctrlTransfer;

//Create Serial2 port on sercom2. D2->RX, D3->TX
//See https://learn.sparkfun.com/tutorials/adding-more-sercom-ports-for-samd-boards
Uart Serial2 (&sercom2, 3, 2, SERCOM_RX_PAD_1, UART_TX_PAD_2);
void SERCOM2_Handler()
{
  Serial2.IrqHandler();
}

Sabertooth ST(128, Serial2);


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

    int denominator = max(abs(ctrlData.driveSpeed) + abs(ctrlData.strafeSpeed), CTRL_MAX);
    int leftSpeed = (ctrlData.driveSpeed + ctrlData.strafeSpeed) * CTRL_MAX / denominator;
    int rightSpeed = (ctrlData.driveSpeed - ctrlData.strafeSpeed) * CTRL_MAX / denominator;
    
    ST.motor(LEFT, leftSpeed);
    ST.motor(RIGHT, rightSpeed);

    if (DEBUG == true) {
      char dat1[32];
      sprintf(dat1, "Drive:%i,Strafe:%i,Turn:%i\r\n", ctrlData.driveSpeed, ctrlData.strafeSpeed, ctrlData.turnSpeed);
      SerialUSB.println(dat1);
      SerialUSB.print(leftSpeed);
      SerialUSB.print("    ");
      SerialUSB.println(rightSpeed);
     }

  }
  else if (ctrlTransfer.status < 0) {
    if (DEBUG == true) {
      SerialUSB.print("Error");
    }
  }
}
