#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <L298NX2.h>

//DECLARE VARIABLES
float dt1 = 1.5;

//RF24
RF24 radio(49, 48);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;



// Pin definition
const unsigned int EN_FL = 23;
const unsigned int IN1_FL = 25;
const unsigned int IN2_FL = 24;

const unsigned int IN1_FR = 27;
const unsigned int IN2_FR = 26;
const unsigned int EN_FR = 22;

const unsigned int EN_RL = 29;
const unsigned int IN1_RL = 31;
const unsigned int IN2_RL = 33;

const unsigned int IN1_RR = 30;
const unsigned int IN2_RR = 32;
const unsigned int EN_RR = 28;

// Initialize both motors
L298NX2 Fmotors(EN_FL, IN1_FL, IN2_FL, EN_FR, IN1_FR, IN2_FR);
L298NX2 Rmotors(EN_RL, IN1_RL, IN2_RL, EN_RR, IN1_RR, IN2_RR);

// Initial speed
unsigned short FLspeed = 128;
unsigned short FRspeed = 128;
unsigned short RLspeed = 128;
unsigned short RRspeed = 128;

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
  byte leftSignalLEDState_4;
  byte rightSignalLEDState_4;
  byte vehicleMovement;
};
Data_Package data; //Create a variable with the above structure

//SETUP
void setup() {
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
      Serial.println("case 2");
      delay(dt1);
      Serial.print("j1PotX = ");
      Serial.println(data.j1PotX);
      Serial.print("j1PotY = ");
      Serial.println(data.j1PotY);      
      runMotor_2();
      delay(dt1);
      break;
    case 3:
      runMotor_3();
      break;
    case 4:
      break;
  }
}


////STEPPER MOTOR FUNCTION
void runMotor_3() {
  // if (data.vehicleMovement == 1) {
  //   moveSidewaysRight();
  //   delay(10);
  // }
  // else {
  //   stopMoving();
  // }
  //Execute the steps
  Fmotors.setSpeedA(255);
  Fmotors.setSpeedB(255);
  Rmotors.setSpeedA(255);
  Rmotors.setSpeedB(255);
}
void runMotor_2() {
  // if (data.j1PotY > 160) {
  //   rotateLeft();
  //   // delay(dt1);
  // }
  // else if (data.j1PotY < 100) {
  //   rotateRight();
  //   // delay(dt1);
  // }
  if (data.j1PotX > 160) {
    turnLeft();
    // delay(dt1);
  }
  else if (data.j1PotX < 100) {
    turnRight();
    // delay(dt1);
  }
  else if (data.j2PotY < 30) {
    moveBackward();
    // delay(dt1);
  }
  else if (data.j2PotY > 220) {
    moveForward();
    // delay(dt1);
  }
  else if (data.j2PotX < 100) {
    rotateRight();
    // delay(dt1);
  }
  else if (data.j2PotX > 160) {
    rotateLeft();
    // delay(dt1);
  } 
  else {
    stopMoving();
    // delay(dt1);
  }
  // Execute the steps
  Fmotors.setSpeedA(FLspeed);
  Fmotors.setSpeedB(FRspeed);
  Rmotors.setSpeedA(RRspeed);
  Rmotors.setSpeedB(RLspeed);
}
void moveForward() {
  Fmotors.backwardA();
  Fmotors.backwardB();
  Rmotors.forwardA();
  Rmotors.backwardB();
}
void moveBackward() {
  Fmotors.forwardA();
  Fmotors.forwardB();
  Rmotors.backwardA();
  Rmotors.forwardB();
}
void turnRight() {
  Fmotors.backwardB();
  Rmotors.forwardA();
}
void turnLeft() {
  Fmotors.backwardA();
  Rmotors.backwardB(); 
}
void rotateLeft() {
  Fmotors.forwardA();
  Fmotors.backwardB();
  Rmotors.forwardA();
  Rmotors.forwardB();  
}
void rotateRight() {
  Fmotors.forwardB();
  Fmotors.backwardA();
  Rmotors.backwardA();
  Rmotors.backwardB();
}
void moveRightForward() {
  Fmotors.backwardA();
  Rmotors.forwardA();
}
void moveRightBackward() {
  
}
void moveLeftForward() {
  Fmotors.backwardB();
  Rmotors.backwardB();
}
void moveLeftBackward() {
  
}
void stopMoving() {
  Fmotors.stop();
  Rmotors.stop();
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
  data.leftSignalLEDState_4 = 0;
  data.rightSignalLEDState_4 = 0;
  
}
