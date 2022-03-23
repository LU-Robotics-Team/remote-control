//Firmware for Lingnan University Robocon robot wireless controller
//Receives control commands to robot over Xbee using the Serial Transfer library 
//Calculates motor speeds and sends to Sabertooth motor controller

//As of 23 March 2022 (Redboard Turbo board definition vs 1.8.6), 
//must update board definitions for Sparkfun Redboard Turbo, see:
//https://github.com/sparkfun/Arduino_Boards/pull/96
 
#include <SerialTransfer.h>
#include <Sabertooth.h>
#include "wiring_private.h" // pinPeripheral() function D2-TX, D3-RX

const int CTRL_MAX = 127;
struct CTRL {
  int driveSpeed;
  int strafeSpeed;
  int turnSpeed;
} ctrlData;

Sabertooth FRONT_ST(128, Serial1);
Sabertooth REAR_ST(129, Serial1);

SerialTransfer ctrlTransfer;

//Uart XBEE (&sercom2, 3, 2, SERCOM_RX_PAD_1, UART_TX_PAD_2);
//void SERCOM2_Handler()
//{
//  XBEE.IrqHandler();
//}

void setup() {
//XBEE.begin(9600); //XBee
SerialUSB.begin(9600);
Serial1.begin(9600);//
//ctrlTransfer.begin(XBEE);
ctrlTransfer.begin(Serial1);
}

void loop() {

  if(ctrlTransfer.available()) {
    int recSize = 0;
    recSize = ctrlTransfer.rxObj(ctrlData, recSize);
    
    int denominator = max(abs(ctrlData.driveSpeed) + abs(ctrlData.strafeSpeed) + abs(ctrlData.turnSpeed), CTRL_MAX);
    int frontLeft = (ctrlData.driveSpeed + ctrlData.strafeSpeed + ctrlData.turnSpeed)* CTRL_MAX / denominator;
    int backLeft = (ctrlData.driveSpeed - ctrlData.strafeSpeed + ctrlData.turnSpeed)* CTRL_MAX / denominator;
    int frontRight = (ctrlData.driveSpeed - ctrlData.strafeSpeed - ctrlData.turnSpeed) * CTRL_MAX / denominator;
    int backRight = (ctrlData.driveSpeed + ctrlData.strafeSpeed - ctrlData.turnSpeed) * CTRL_MAX / denominator;
    
    char dat1[32];
    sprintf(dat1, "Drive:%i,Strafe:%i,Turn:%i\r\n",ctrlData.driveSpeed,ctrlData.strafeSpeed,ctrlData.turnSpeed);
    SerialUSB.print(dat1);

    SerialUSB.print(frontLeft);
    SerialUSB.print("    ");
    SerialUSB.println(frontRight);
    SerialUSB.println("    ");
    SerialUSB.println("    ");
    SerialUSB.print(backLeft);
    SerialUSB.print("    ");
    SerialUSB.println(backRight);
    
  }
  else if(ctrlTransfer.status<0) {
    SerialUSB.print("Error");
  }

}
