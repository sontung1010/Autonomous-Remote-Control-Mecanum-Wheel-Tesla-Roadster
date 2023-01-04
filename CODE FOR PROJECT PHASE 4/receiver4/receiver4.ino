#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//DECLARE VARIABLES
float dt1 = 1.5;
int dt2 = 150;

//RF24
RF24 radio(7, 8);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";
//const byte address[][6] = {"00001","00002","00003"};
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//LEDs
int leftFrontSignal = 4;
int leftRearSignal = 3;
int rightFrontSignal = 5;
int rightRearSignal = 6; 

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
};
Data_Package data; //Create a variable with the above structure
Data_Package data2;

//SETUP
void setup() {
  //Radio communication  
  radio.begin();
//  radio.openReadingPipe(1, address[1]); //adress used is "00002"
  radio.openReadingPipe(1, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(9);

  //pinModes
  pinMode(leftFrontSignal, OUTPUT);
  pinMode(leftRearSignal, OUTPUT);
  pinMode(rightFrontSignal, OUTPUT);
  pinMode(rightRearSignal, OUTPUT);
  
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
      setLeftSignal();
      delay(dt1);
      setRightSignal();
      delay(dt1);
      setEmergencySignal();
      delay(dt1);
      break;
    case 2:
      setLeftSignal();
      delay(dt1);
      setRightSignal();
      delay(dt1);
      setEmergencySignal();
      delay(dt1);
      break;
    case 3:
      setLeftSignal();
      delay(dt1);
      setRightSignal();
      delay(dt1);
      setEmergencySignal();
      delay(dt1);
      break;
    case 4:
      setLeftSignal_4();
      setRightSignal_4();
      break;
  }
}



//SET SIGNAL FUNCTION
void setLeftSignal() {
  if (data.leftSignalLEDState == 1) {
    digitalWrite(leftFrontSignal, HIGH);
    digitalWrite(leftRearSignal, HIGH);
    delay(dt2);
    digitalWrite(leftFrontSignal, LOW);
    digitalWrite(leftRearSignal, LOW);
    delay(dt2);
  }
  else if (data.leftSignalLEDState == 0) {
    digitalWrite(leftFrontSignal, LOW);
    digitalWrite(leftRearSignal, LOW);
  } 
}
void setLeftSignal_4() {
  if (data.leftSignalLEDState_4 == 1) {
    digitalWrite(leftFrontSignal, HIGH);
    digitalWrite(leftRearSignal, HIGH);
    delay(dt2);
    digitalWrite(leftFrontSignal, LOW);
    digitalWrite(leftRearSignal, LOW);
    delay(dt2);
  }
  else if (data.leftSignalLEDState_4 == 0) {
    digitalWrite(leftFrontSignal, LOW);
    digitalWrite(leftRearSignal, LOW);
  } 
}

void setRightSignal() {
  if (data.rightSignalLEDState == 1) {
    digitalWrite(rightFrontSignal, HIGH);
    digitalWrite(rightRearSignal, HIGH);
    delay(dt2);
    digitalWrite(rightFrontSignal, LOW);
    digitalWrite(rightRearSignal, LOW);
    delay(dt2);
  }
  else if (data.rightSignalLEDState == 0) {
    digitalWrite(rightFrontSignal, LOW);
    digitalWrite(rightRearSignal, LOW);
  }
}
void setRightSignal_4() {
  if (data.rightSignalLEDState_4 == 1) {
    digitalWrite(rightFrontSignal, HIGH);
    digitalWrite(rightRearSignal, HIGH);
    delay(dt2);
    digitalWrite(rightFrontSignal, LOW);
    digitalWrite(rightRearSignal, LOW);
    delay(dt2);
  }
  else if (data.rightSignalLEDState_4 == 0) {
    digitalWrite(rightFrontSignal, LOW);
    digitalWrite(rightRearSignal, LOW);
  }
}

void setEmergencySignal() {
  if (data.emergencyLEDState == 1) {
    digitalWrite(leftFrontSignal, HIGH);
    digitalWrite(leftRearSignal, HIGH);
    digitalWrite(rightFrontSignal, HIGH);
    digitalWrite(rightRearSignal, HIGH);
    delay(dt2);
    digitalWrite(leftFrontSignal, LOW);
    digitalWrite(leftRearSignal, LOW);
    digitalWrite(rightFrontSignal, LOW);
    digitalWrite(rightRearSignal, LOW);
    delay(dt2);
  }
  else if (data.emergencyLEDState == 0) {
    digitalWrite(leftFrontSignal, LOW);
    digitalWrite(leftRearSignal, LOW);
    digitalWrite(rightFrontSignal, LOW);
    digitalWrite(rightRearSignal, LOW);
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
  data.leftSignalLEDState_4 = 0;
  data.rightSignalLEDState_4 = 0;
}
