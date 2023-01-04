#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <AccelStepper.h>
#include <NewPing.h>
#include <Servo.h> 
#include "dht.h" 

//DECLARE VARIABLES
float dt1 = 1.5;

//RF24
RF24 radio(49, 48);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";
//const byte addresss[6] = "00002";
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//Sensor
#define TRIGGER_PIN  2
#define ECHO_PIN     3
#define MAX_DISTANCE 600
//#define dht11 5 // DHT11 temperature and humidity sensor
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
//dht DHT; // Creats a DHT object
//int readDHT, temp, hum;
//float speedOfSound, distance, duration;
int distance;
int distanceR = 100;
//int distanceRF;
int distanceL = 100;
//int distanceLF;
int distanceF = 100;
int value = 1;
//boolean goesForward = false;

//Servo
Servo myservo; 
int servoAngle = 86;

//Stepper motors
// Define the stepper motors and the pins the will use
AccelStepper LeftBackWheel(1, 26, 27);   // (Type:driver, STEP, DIR) - Stepper1
AccelStepper LeftFrontWheel(1, 30, 31);  // Stepper2
AccelStepper RightBackWheel(1, 36, 37);  // Stepper3
AccelStepper RightFrontWheel(1, 22, 23); // Stepper4
int wheelSpeed;
int order;

//boolean goesForward=false;
//int distance = 100;

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
//  byte distance;
//  byte distanceR = 0;
//  byte distanceL = 0;
//  byte vehicleMovement;
};
Data_Package data; //Create a variable with the above structure

//SETUP
void setup() {
  //Radio communication  
  radio.begin();
  radio.openReadingPipe(1, address);
//  radio.openReadingPipe(1, addresss);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(9);

  //Stepper motors
  // Set initial seed values for the steppers
  LeftFrontWheel.setMaxSpeed(6000);
  LeftBackWheel.setMaxSpeed(6000);
  RightFrontWheel.setMaxSpeed(6000);
  RightBackWheel.setMaxSpeed(6000);

  //Servo
  myservo.attach(4);  
  myservo.write(86); 

//  distanceF = readPing();
  
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
      setStepperMotorSpeed_2();
      Serial.print("wheelSpeed = ");
      Serial.println(wheelSpeed);
      delay(dt1);
      runStepperMotor_2();
      delay(dt1);
      break;
    case 3:
//      activateSensor();
//      delay(dt1);
//      radio.openReadingPipe(1, addresss);
//      Serial.print("data.vehicleMovement = ");
//      Serial.println(data.vehicleMovement);
//      Serial.println("Go into setStepperMotorSpeed_3()");
      setStepperMotorSpeed_3();
      measureWhichDirection();
      measureWhichDistance();
//      Serial.println("Go into delay dt1");
//      delay(dt1);
//      Serial.println("Go into runStepperMotor_3()");
//      setSensorDirection();
//      servoAngle = 179;
      runStepperMotor_3();
      Serial.print("DistanceF = ");
      Serial.println(distanceF);
//      Serial.println("Go into delay dt1");
//      delay(dt1);
//      Serial.println("BREAK");
//      delay(1000);
      break;
  }
}



int readPing() {
  distance = sonar.ping_cm(); 
  return distance;
}
int lookRight() {
  myservo.write(0); 
  delay(400);
  distanceR = readPing();
  delay(200);
  myservo.write(86); 
  delay(200);
}
int lookLeft() {
  myservo.write(179); 
  delay(400);
  distanceL = readPing();
  delay(200);
  myservo.write(86); 
//  delay(200);
}

//STEPPER MOTOR FUNCTION
void setStepperMotorSpeed_3() {
  wheelSpeed = map(data.vehicleSpeed, 0, 255, 100, 6000);
}
void measureWhichDirection() {
  if (distanceF < 16 && value == 1) {
    lookRight();
    lookLeft();
//    Serial.println(distanceR);
//    Serial.println(distanceL);
//    delay(5000);
    if (distanceR > distanceL) {
      myservo.write(0);
      servoAngle = 0;
//      Serial.println("Done write(0)");
//      delay(5000);
    }
    else if (distanceR <= distanceL) {
      myservo.write(179);
      servoAngle = 179;
//      Serial.println("Done write(179)");
//      delay(5000);
    }
  }
  else if (distanceF > 16 && value == 1) {
    myservo.write(86);
    servoAngle = 86;
//    Serial.println("Done write(86)");
//    delay(5000);
  }
}
void measureWhichDistance() {
  switch (servoAngle) {
    case 86:
      distanceF = readPing();
//      Serial.println("Done readPing(86)");
//      delay(1000);
      break;
    case 179:
      distanceL = readPing();
      value = 2;
//      Serial.println("Done readPing(179)");
//      Serial.print("New distanceL = ");
//      Serial.println(distanceL);
//      delay(5000);
      break;
    case 0:
      distanceR = readPing();
      value = 2;
//      Serial.println("Done readPing(0)");
//      Serial.print("New distanceR = ");
//      Serial.println(distanceR);
//      delay(5000);
      break;
  }
}

void runStepperMotor_3() {
  if (servoAngle == 179 && distance >= 50) {
    moveSidewaysLeft();
//    Serial.println("Done case 1");
//    delay(5000);
  }
  if (servoAngle == 179 && distance < 50) {
    myservo.write(86);
    servoAngle = 86;
    delay(300);
    distanceF = readPing();
    delay(200);
    value = 1;
//    Serial.println("Done case 2");
//    delay(100);
  }
  if (servoAngle == 0 && distanceR >= 50) {
    moveSidewaysRight();
//    Serial.println("Done case 3");
//    delay(5000);
  }
  if (servoAngle == 0 && distanceR < 50) {
    myservo.write(86);
    servoAngle = 86;
    delay(300);
    distanceF = readPing();
    delay(200);
    value = 1;
//    Serial.println("Done case 4");
//    delay(100);
  }
  if (servoAngle == 86 && distanceF >= 16) {
    moveForward();
//    Serial.println("Done case 5");
//    delay(5000);
  }
  //Execute the steps
  LeftFrontWheel.runSpeed();
  LeftBackWheel.runSpeed();
  RightFrontWheel.runSpeed();
  RightBackWheel.runSpeed();
}
//void setSensorDirection() {
//  distanceF = readPing();
//  if (distanceF <= 16) {
//    lookRightFront();
//    lookLeftFront();
//    Serial.println(distanceRF);
//    Serial.println(distanceLF);
////    delay(99999999999999999);
//    if (distanceRF > distanceLF) {
//      myservo.write(0);
//      servoAngle = 0;
//      distanceR = readPing();
//      if (distanceR >= 50) {
////        moveSidewaysRight();
//        
//      }
//    }
//    else if (distanceRF <= distanceLF) {
//      myservo.write(179);
//    }
//  }
//  else {
//    order = 5;
//  }
//}
//void runStepperMotor_3() {
//  switch (servoAngle) {
//    case 0:
//      moveSidewaysRight();
//      break;
//    case 179:
//      moveSidewaysLeft();
//      break;
//    case 86: 
//      moveForward();
//      break;
//  }
//  //Execute the steps
//  LeftFrontWheel.runSpeed();
//  LeftBackWheel.runSpeed();
//  RightFrontWheel.runSpeed();
//  RightBackWheel.runSpeed();
//}

void setStepperMotorSpeed_2() {
  wheelSpeed = map(data.vehicleSpeed, 0, 255, 100, 6000);
}
void runStepperMotor_2() {
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
