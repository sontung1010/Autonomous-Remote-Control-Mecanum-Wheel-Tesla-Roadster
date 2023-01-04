#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//DECLARE VARIABLES
int vehicleState;
int chargingState;
int wiperState;

//Joysticks
#define j1PotX A2
#define j1PotY A3
#define j2PotX A0
#define j2PotY A1

//Servo
int wheelAngle;
int turnSteer;
int sideSteer;
int hoodAngle;
int leftDoorAngle;
int rightDoorAngle;
int leftWindowAngle;
int rightWindowAngle;

//Buttons
int leftSignalButtonPin = 3;
int newLeftSignalButton;
int oldLeftSignalButton;
int rightSignalButtonPin = 4;
int newRightSignalButton;
int oldRightSignalButton;
int emergencyButtonPin = 5;
int newEmergencyButton;
int oldEmergencyButton;
int chargingButtonPin = 2;
int newChargingButton;
int oldChargingButton;
int wiperButtonPin = 6;
int newWiperButton;
int oldWiperButton;

//Potentiometers
#define vehicleStatePin A15
int vehicleStatePot;
#define vehicleSpeedPin A14
#define headlightPin A13
int headlightPot;
#define hoodPin A12
#define leftDoor A11
#define rightDoor A10
#define leftWindow A9
#define rightWindow A8

//LEDs
int redPin = 27;
int greenPin = 25;
int bluePin = 23;
int leftSignalLEDState = 0; 
int rightSignalLedState = 0;
int emergencyLEDState = 0;

//RF24
RF24 radio(49, 48);   // nRF24L01 (CE, CSN)
//const byte address[][6] = {"00001","00002","00003"}; // Address
const byte address[6] = "00001";

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
  byte leftSignalLEDState = 0;
  byte rightSignalLEDState = 0;
  byte emergencyLEDState = 0;
  byte chargingState = 0;
  byte hoodAngle;
  byte wiperState = 0;
  byte leftDoorAngle;
  byte rightDoorAngle;
  byte leftWindowAngle;
  byte rightWindowAngle;
  byte leftSignalLEDState_4;
  byte rightSignalLEDState_4;
};
Data_Package data; //Create a variable with the above structure

//SETUP
void setup() {
  //Serial
  Serial.begin(9600);

  //Radio communication
  radio.begin();
//  radio.openWritingPipe(address[1]); // address used is "00002"
  radio.openWritingPipe(address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(9);

  // Set initial default values
  data.j1PotX = 127; // Values from 0 to 255. When Joystick is in resting position, the value is in the middle, or 127. We actually map the pot value from 0 to 1023 to 0 to 255 because that's one BYTE value
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

  //pinModes
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(headlightPin, INPUT);
  pinMode(leftSignalButtonPin, INPUT);
  pinMode(rightSignalButtonPin, INPUT);
  pinMode(emergencyButtonPin, INPUT);
}

//EXECUTION
void loop() {
  radio.stopListening();  //Set the module as trasnmitter
  read_vehicleState();
  read_vehicleSpeed();
  read_headlightBrightness();
  read_joystick();
  read_dataSteeringWheel();
  read_leftSignalState();
  read_rightSignalState();
  read_emergencyState();
  read_chargingState();
  read_hoodAngle();
  read_wiperState();
  read_leftDoorAngle();
  read_rightDoorAngle();
  read_leftWindowAngle();
  read_rightWindowAngle();
  radio.write(&data, sizeof(Data_Package));   // Send the whole data from the structure to the receiver
}



//SWITCH CASE FUNCTION
void read_vehicleState() {
  vehicleStatePot = map(analogRead(vehicleStatePin), 0, 1023, 0, 4);
  if (vehicleStatePot >= 0 && vehicleStatePot < 1) {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
    vehicleState = 1;
  } 
  else if (vehicleStatePot >= 1 && vehicleStatePot <= 2) {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
    vehicleState = 2;
  } 
  else if (vehicleStatePot > 2 && vehicleStatePot <= 3) {
  digitalWrite(redPin, 255);
  digitalWrite(greenPin, 0);
  digitalWrite(bluePin, 255);
  vehicleState = 3;
  }
  else if (vehicleStatePot > 3 && vehicleStatePot <= 4) {
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);
  vehicleState = 4;
  }
  data.vehicleState = vehicleState;
}

//READ SIGNAL STATE FUNCTION
void read_leftSignalState() {
  newLeftSignalButton = digitalRead(leftSignalButtonPin);
  if (oldLeftSignalButton == 0 && newLeftSignalButton == 1) {
    if (data.leftSignalLEDState == 0) {
      data.leftSignalLEDState = 1;
    }
    else {
      data.leftSignalLEDState = 0;
    } 
  }
  oldLeftSignalButton = newLeftSignalButton;
}
void read_rightSignalState() {
  newRightSignalButton = digitalRead(rightSignalButtonPin);
  if (oldRightSignalButton == 0 && newRightSignalButton == 1) {
    if (data.rightSignalLEDState == 0) {
      data.rightSignalLEDState = 1;
    }
    else {
      data.rightSignalLEDState = 0;
    } 
  }
  oldRightSignalButton = newRightSignalButton;
}
void read_emergencyState() {
  newEmergencyButton = digitalRead(emergencyButtonPin);
  if (oldEmergencyButton == 0 && newEmergencyButton == 1) {
    if (data.emergencyLEDState == 0) {
      data.emergencyLEDState = 1;
    }
    else {
      data.emergencyLEDState = 0;
    } 
  }
  oldEmergencyButton = newEmergencyButton;
}

//READ CHARGING STATE FUNCTION
void read_chargingState() {
  newChargingButton = digitalRead(chargingButtonPin);
  if (oldChargingButton == 0 && newChargingButton == 1) {
    if (data.chargingState == 0) {
      data.chargingState = 1;
    }
    else {
      data.chargingState = 0;
    } 
  }
  oldChargingButton = newChargingButton;
}

//READ DOOR ANGLE
void read_leftDoorAngle() {
  data.leftDoorAngle = map(analogRead(leftDoor), 0, 1023, 0, 179);
}
void read_rightDoorAngle() {
  data.rightDoorAngle = map(analogRead(rightDoor), 0, 1023, 0, 179);
}

//READ WINDOW ANGLE
void read_leftWindowAngle() {
  data.leftWindowAngle = map(analogRead(leftWindow), 0, 1023, 0, 179);
}
void read_rightWindowAngle() {
  data.rightWindowAngle = map(analogRead(rightWindow), 0, 1023, 0, 179);
}

//READ WIPER STATE FUNCTION
void read_wiperState() {
  newWiperButton = digitalRead(wiperButtonPin);
  if (oldWiperButton == 0 && newWiperButton == 1) {
    if (data.wiperState == 0) {
      data.wiperState = 1;
    }
    else {
      data.wiperState = 0;
    } 
  }
  oldWiperButton = newWiperButton;
}

//READ HEADLIGHT FUNCTION
void read_headlightBrightness() {
  data.headlightPot = map(analogRead(headlightPin), 0, 1023, 0, 255);
}

//READ VEHICLE SPEED FUNCTION
void read_vehicleSpeed() {
  data.vehicleSpeed = map(analogRead(vehicleSpeedPin), 0, 1023, 0, 255);
}

//READ HOOD ANGLE FUNCTION
void read_hoodAngle() {
  data.hoodAngle = map(analogRead(hoodPin), 0, 1023, 0, 179);
}

//READ JOYSTICK FUNCTION
void read_joystick() {
  // Read all analog inputs and map them to one Byte value
  data.j1PotX = map(analogRead(j1PotX), 0, 1023, 0, 255); // Convert the analog read value from 0 to 1023 into a BYTE value from 0 to 255
  Serial.println(data.j1PotX);
  data.j1PotY = map(analogRead(j1PotY), 0, 1023, 0, 255);
  Serial.println(data.j1PotY);
  data.j2PotX = map(analogRead(j2PotX), 0, 1023, 0, 255); // Convert the analog read value from 0 to 1023 into a BYTE value from 0 to 255
  data.j2PotY = map(analogRead(j2PotY), 0, 1023, 0, 255);
}

//READ DATA FOR STEERING WHEEL FUNCTION
void read_dataSteeringWheel() {
  turnSteer = map(analogRead(j2PotX), 0, 1023, 0, 179);
  sideSteer = map(analogRead(j1PotX), 0, 1023, 0, 179);
  if (sideSteer < 80 || sideSteer > 100) {
    data.wheelAngle = sideSteer;
  }
  if (turnSteer < 80 || turnSteer > 100) {
    data.wheelAngle = turnSteer;
  }
}
