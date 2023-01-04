#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <AccelStepper.h>
#include <NewPing.h>
#include <Servo.h> 

//DECLARE VARIABLES
float dt1 = 1.5;

//RF24
RF24 radio(49, 48);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//Sensor
#define TRIGGER_PIN  2
#define ECHO_PIN     3
#define MAX_DISTANCE 600
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
int distance;
int distanceR = 100;
int distanceL = 100;
int distanceF = 100;
int value = 1;

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
//  byte vehicleMovement;
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

  //Stepper motors
  // Set initial seed values for the steppers
  LeftFrontWheel.setMaxSpeed(6000);
  LeftBackWheel.setMaxSpeed(6000);
  RightFrontWheel.setMaxSpeed(6000);
  RightBackWheel.setMaxSpeed(6000);

  //Servo
  myservo.attach(5);  
  myservo.write(86); 
  
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
      delay(dt1);
      runStepperMotor_2();
      delay(dt1);
      break;
    case 3:
      setStepperMotorSpeed();
      Serial.println(data.j2PotY);
      delay(dt1);
      runStepperMotor_3();
      delay(dt1);
      break;
    case 4:
      radio.stopListening();
      radio.openWritingPipe(address);
      radio.write(&data, sizeof(Data_Package));
      setStepperMotorSpeed();
      measureWhichDirection();
      measureWhichDistance();
      runStepperMotor_4();
      break;
  }
}



//SENSOR FUNCTION
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
void setStepperMotorSpeed() {
  wheelSpeed = map(data.vehicleSpeed, 0, 255, 100, 6000);
}
void measureWhichDirection() {
  if (distanceF < 16 && value == 1) {
    lookRight();
    lookLeft();
    if (distanceR > distanceL) {
      myservo.write(0);
      delay(100);
      servoAngle = 0;
    }
    else if (distanceR <= distanceL) {
      myservo.write(179);
      servoAngle = 179;
    }
  }
  else if (distanceF > 16 && value == 1) {
    myservo.write(86);
    servoAngle = 86;
  }
}
void measureWhichDistance() {
  switch (servoAngle) {
    case 86:
      distanceF = readPing();
      break;
    case 179:
      distanceL = readPing();
      value = 2;
      break;
    case 0:
      distanceR = readPing();
      value = 2;
      break;
  }
}
void runStepperMotor_4() {
  if (servoAngle == 179 && distanceL >= 30) {
    data.leftSignalLEDState_4 = 1;
    moveSidewaysLeft();
  }
  if (servoAngle == 179 && distanceL < 30) {
    myservo.write(86);
    servoAngle = 86;
    delay(300);
    distanceF = readPing();
    delay(200);
    value = 1;
  }
  if (servoAngle == 0 && distanceR >= 30) {
    data.rightSignalLEDState_4 = 1;
    moveSidewaysRight();
  }
  if (servoAngle == 0 && distanceR < 30) {
    myservo.write(86);
    servoAngle = 86;
    delay(300);
    distanceF = readPing();
    delay(200);
    value = 1;
  }
  if (servoAngle == 86 && distanceF >= 16) {
    moveForward();
  }
  //Execute the steps
  LeftFrontWheel.runSpeed();
  LeftBackWheel.runSpeed();
  RightFrontWheel.runSpeed();
  RightBackWheel.runSpeed();
}
void runStepperMotor_3() {
  if (data.j1PotX < 100 & data.j1PotY > 160) {
    moveRightForward();
    delay(dt1);
  }
  else if (data.j1PotX > 160 & data.j1PotY > 160) {
    moveLeftForward();
    delay(dt1);
  }
  else if (data.j1PotX < 100 & data.j1PotY < 100) {
    moveRightBackward();
    delay(dt1);
  }
  else if (data.j1PotX > 160 & data.j1PotY < 100) {
    moveLeftBackward();
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
  RightFrontWheel.setSpeed(wheelSpeed);
  RightBackWheel.setSpeed(0);
}
void moveLeftForward() {
  LeftFrontWheel.setSpeed(0);
  LeftBackWheel.setSpeed(wheelSpeed);
  RightFrontWheel.setSpeed(-wheelSpeed);
  RightBackWheel.setSpeed(0);
}
void moveLeftBackward() {
  LeftFrontWheel.setSpeed(-wheelSpeed);
  LeftBackWheel.setSpeed(0);
  RightFrontWheel.setSpeed(0);
  RightBackWheel.setSpeed(wheelSpeed);
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
  data.leftSignalLEDState_4 = 0;
  data.rightSignalLEDState_4 = 0;
  
}
