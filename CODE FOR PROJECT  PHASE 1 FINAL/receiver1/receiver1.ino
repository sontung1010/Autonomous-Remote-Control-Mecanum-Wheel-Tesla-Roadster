#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <AccelStepper.h>

//DECLARE VARIABLES
float dt1 = 1.5;

//RF24
RF24 radio(49, 48);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//Stepper motors
// Define the stepper motors and the pins the will use
AccelStepper LeftBackWheel(1, 32, 33);   // (Type:driver, STEP, DIR) - Stepper1
AccelStepper LeftFrontWheel(1, 30, 31);  // Stepper2
AccelStepper RightBackWheel(1, 38, 39);  // Stepper3
AccelStepper RightFrontWheel(1, 22, 23); // Stepper4
int wheelSpeed;

//CONSTRUCT DATA PACKAGE
// Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package {
  byte j1PotX;
  byte j1PotY;
  byte j2PotX;
  byte j2PotY;
  byte vehicleState;
  byte vehicleSpeed;
  byte wheelAngle;
  byte headlightPot;
  byte leftSignalLEDState;
  byte rightSignalLEDState;
  byte emergencyLEDState;
  byte chargingState;
  byte hoodAngle;
  byte wiperState;
  byte leftDoorAngle;
  byte rightDoorAngle;
  byte leftWindowAngle;
  byte rightWindowAngle;
};
Data_Package data; //Create a variable with the above structure

//SETUP
void setup() {
  //Stepper motors
  // Set initial seed values for the steppers
  LeftFrontWheel.setMaxSpeed(6000);
  LeftBackWheel.setMaxSpeed(6000);
  RightFrontWheel.setMaxSpeed(6000);
  RightBackWheel.setMaxSpeed(6000);

  //Radio communication  
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(9);
  
  //Serial
  Serial.begin(9600);
}

//EXECUTION
void loop() {
  checkConnection();  // Check whether there is data to be received
  radio.startListening(); //  Set the module as receiver
  if (radio.available()) {
    radio.read(&data, sizeof(Data_Package)); // Read the whole data and store it into the 'data' structure
    lastReceiveTime = millis(); // At this moment we have received the data
  }
  switch (data.vehicleState) {
    case 2:
      setStepperMotorSpeed();
      Serial.print("wheelSpeed = ");
      Serial.println(wheelSpeed);
      delay(dt1);
      runStepperMotor();
      delay(dt1);
      break;
  }
}



//STEPPER MOTOR FUNCTIONS
void setStepperMotorSpeed() {
  wheelSpeed = map(data.vehicleSpeed, 0, 255, 100, 6000);
}
void runStepperMotor() {
  if (data.j1PotX > 160) {
    moveSidewaysLeft();
    delay(dt1);
  }
  else if (data.j1PotX < 100) {
    moveSidewaysRight();
    delay(dt1);
  }
  else if (data.j1PotY > 160) {
    moveForward();
    delay(dt1);
  }
  else if (data.j1PotY < 100) {
    moveBackward();
    delay(dt1);
  }
//  else if (data.j2PotX < 100 & data.j2PotY > 160) {
//    moveRightForward();
//  }
//  else if (data.j2PotX > 160 & data.j2PotY > 160) {
//    moveLeftForward();
//  }
//  else if (data.j2PotX < 100 & data.j2PotY < 100) {
//    moveRightBackward();
//  }
//  else if (data.j2PotX > 160 & data.j2PotY < 100) {
//    moveLeftBackward();
//  }
  else if (data.j2PotX < 100) {
    rotateRight();
    delay(dt1);
  }
  else if (data.j2PotX > 160) {
    rotateLeft();
    delay(dt1);
  }
  else {
    stopMoving();
    delay(dt1);
  }
  // Execute the steps
  LeftFrontWheel.runSpeed();
  LeftBackWheel.runSpeed();
  RightFrontWheel.runSpeed();
  RightBackWheel.runSpeed();
}

void moveForward() {
  LeftFrontWheel.setSpeed(wheelSpeed);
  LeftBackWheel.setSpeed(wheelSpeed);
  RightFrontWheel.setSpeed(-wheelSpeed);
  RightBackWheel.setSpeed(-wheelSpeed);
}
void moveBackward() {
  LeftFrontWheel.setSpeed(-wheelSpeed);
  LeftBackWheel.setSpeed(-wheelSpeed);
  RightFrontWheel.setSpeed(wheelSpeed);
  RightBackWheel.setSpeed(wheelSpeed);
}
void moveSidewaysRight() {
  LeftFrontWheel.setSpeed(wheelSpeed);
  LeftBackWheel.setSpeed(-wheelSpeed);
  RightFrontWheel.setSpeed(wheelSpeed);
  RightBackWheel.setSpeed(-wheelSpeed);
}
void moveSidewaysLeft() {
  LeftFrontWheel.setSpeed(-wheelSpeed);
  LeftBackWheel.setSpeed(wheelSpeed);
  RightFrontWheel.setSpeed(-wheelSpeed);
  RightBackWheel.setSpeed(wheelSpeed);
}
void rotateLeft() {
  LeftFrontWheel.setSpeed(-wheelSpeed);
  LeftBackWheel.setSpeed(-wheelSpeed);
  RightFrontWheel.setSpeed(-wheelSpeed);
  RightBackWheel.setSpeed(-wheelSpeed);
}
void rotateRight() {
  LeftFrontWheel.setSpeed(wheelSpeed);
  LeftBackWheel.setSpeed(wheelSpeed);
  RightFrontWheel.setSpeed(wheelSpeed);
  RightBackWheel.setSpeed(wheelSpeed);
}
void moveRightForward() {
  LeftFrontWheel.setSpeed(wheelSpeed);
  LeftBackWheel.setSpeed(0);
  RightFrontWheel.setSpeed(0);
  RightBackWheel.setSpeed(-wheelSpeed);
}
void moveRightBackward() {
  LeftFrontWheel.setSpeed(0);
  LeftBackWheel.setSpeed(-wheelSpeed);
  RightFrontWheel.setSpeed(-wheelSpeed);
  RightBackWheel.setSpeed(0);
}
void moveLeftForward() {
  LeftFrontWheel.setSpeed(0);
  LeftBackWheel.setSpeed(wheelSpeed);
  RightFrontWheel.setSpeed(wheelSpeed);
  RightBackWheel.setSpeed(0);
}
void moveLeftBackward() {
  LeftFrontWheel.setSpeed(-wheelSpeed);
  LeftBackWheel.setSpeed(0);
  RightFrontWheel.setSpeed(0);
  RightBackWheel.setSpeed(-wheelSpeed);
}
void stopMoving() {
  LeftFrontWheel.setSpeed(0);
  LeftBackWheel.setSpeed(0);
  RightFrontWheel.setSpeed(0);
  RightBackWheel.setSpeed(0);
}

//CHECK CONNECTION FUNCTION
void checkConnection() {
  // Check whether we keep receving data, or we have a connection between the two modules
  currentTime = millis();
  if ( currentTime - lastReceiveTime > 1000 ) { // If current time is more then 1 second since we have recived the last data, that means we have lost connection
    resetData(); // If connection is lost, reset the data. It prevents unwanted behavior, for example if a drone jas a throttle up, if we lose connection it can keep flying away if we dont reset the function
  }
}

//RESET DATA FUNCTION
void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  data.j1PotX = 127;
  data.j1PotY = 127;
  data.j2PotX = 127;
  data.j2PotY = 127;
  data.vehicleState = 1;
  data.vehicleSpeed = 100;
  data.wheelAngle = 88;
  data.headlightPot = 0;
  data.leftSignalLEDState = 0;
  data.rightSignalLEDState = 0;
  data.emergencyLEDState = 0;
  data.chargingState = 0;
  data.hoodAngle = 0;
  data.wiperState = 0;
  data.leftDoorAngle = 0;
  data.rightDoorAngle = 0;
  data.leftWindowAngle = 0;
  data.rightWindowAngle = 0;
}
