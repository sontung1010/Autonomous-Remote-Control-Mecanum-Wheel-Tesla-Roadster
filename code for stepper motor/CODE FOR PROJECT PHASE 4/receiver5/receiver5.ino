#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include <NewPing.h>
//#include <Servo.h> 
//#include "dht.h" 

//DECLARE VARIABLES
float dt1 = 1.5;

//RF24
RF24 radio(7, 8);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";
//const byte addresss[6] = "00002";
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//Servo motors
Servo servoWiper;
Servo servoM; 
Servo servoL;
Servo servoR;
int leftAngle = 0;
int rightAngle = 179;
int angleState = 1;

//Sensor
//#define TRIGGER_PIN_M  2
//#define ECHO_PIN_M     3
#define TRIGGER_PIN_R  6
#define ECHO_PIN_R     5
#define TRIGGER_PIN_L  3
#define ECHO_PIN_L     4
#define MAX_DISTANCE 600
//NewPing sonarM(TRIGGER_PIN_M, ECHO_PIN_M, MAX_DISTANCE);
NewPing sonarR(TRIGGER_PIN_R, ECHO_PIN_R, MAX_DISTANCE);
NewPing sonarL(TRIGGER_PIN_L, ECHO_PIN_L, MAX_DISTANCE);
int distance;
int distanceR = 100;
int distanceL = 100;
int distanceRF = 100;
int distanceLF = 100;
int distanceRB = 100;
int distanceLB = 100;
int distanceRFD = 100;
int distanceLFD = 100;
//int distanceM = 100;
int value = 1;

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
  radio.openReadingPipe(1,address);
//  radio.openWritingPipe(addresses[1]);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(9);

  //Servo
  servoWiper.attach(2); 
//  servoM.attach(5);
  servoL.attach(10);
  servoR.attach(9);
//  servoM.write(86);
  servoL.write(0);
  servoR.write(179); 
  
  //Serial
  Serial.begin(9600);
}

void loop() {
  checkConnection();  // Check whether there is data to be received
  radio.startListening(); //  Set the module as receiver
  if (radio.available()) {
    radio.read(&data, sizeof(Data_Package)); // Read the whole data and store it into the 'data' structure
    lastReceiveTime = millis(); // At this moment we have received the data
  }
  switch (data.vehicleState) {
    case 1:
      setWiperState();
      delay(dt1);
      break;
    case 2:
      setWiperState();
      delay(dt1);
      break;
    case 3:
      setWiperState();
      delay(dt1);
      break;
    case 4: 
      radio.stopListening();
      radio.openWritingPipe(address);
      measureServoDistance();
      read_Sensor();
      setAngle();
      radio.write(&data, sizeof(Data_Package));
      
  }
}



//WIPER SERVO MOTOR FUNCTION
void setWiperState() {
  if (data.wiperState == 1) {
    servoWiper.write(179);
    delay(300);
    servoWiper.write(0); 
    delay(300); 
  }
  else if (data.wiperState == 0) {
    servoWiper.write(0);
  }
}

//SENSOR FUNCTION
int read_PingR() {
  distance = sonarR.ping_cm();
  return distance;
}
int read_PingL() {
  distance = sonarL.ping_cm();
  return distance;
}
int lookRight() {
  servoR.write(90);
  delay(400);
  distanceR = read_PingR();
  delay(200);
  servoR.write(179);
}
int lookLeft() {
  servoL.write(90);
  delay(400);
  distanceL = read_PingL();
  delay(200);
  servoL.write(0);
}
int lookLeftFront() {
  servoL.write(45);
  delay(400);
  distanceLFD = read_PingL();
  delay(200);
  servoL.write(0);
}


void read_Sensor() {
  if (distanceLF > 45 && distanceRF > 45 && value == 1) {
    data.vehicleMovement = 1;
  }
  if (distanceLF < 45 && distanceRF > 45 && value == 1) {
    data.vehicleMovement = 2;
  }
  if (distanceLF > 45 && distanceRF < 45 && value == 1) {
    data.vehicleMovement = 3;
  }
}

void measureServoDistance() {
  switch (leftAngle) {
    case 0: 
      distanceLF = read_PingL();
      break;
    case 90:
      distanceL = read_PingL();
      break;
    case 179:
      distanceLB = read_PingL();
      break;
  }
  switch (rightAngle) {
    case 0: 
      distanceRB = read_PingR();
      break;
    case 90:
      distanceR = read_PingR();
      break;
    case 179:
      distanceRF = read_PingR();
      break;
  }
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
