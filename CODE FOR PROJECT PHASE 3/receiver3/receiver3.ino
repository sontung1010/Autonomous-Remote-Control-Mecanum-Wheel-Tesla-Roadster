#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

//DECLARE VARIABLES
float dt1 = 1.5;

//RF24
RF24 radio(49, 48);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//Servo motors
Servo servoWheel;
int newWheelAngle = 0;
Servo servoCharger;
int openChargerAngle = 0;
int closeChargerAngle = 179;
Servo servoHood1;
int servoHood1_Angle;
Servo servoHood2;
int servoHood2_Angle;
Servo servoLeftDoor;
Servo servoRightDoor;
Servo servoLeftWindow;
Servo servoRightWindow;
Servo servoGasPedal;

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
  //Radio communication  
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(9);

  //Servo
  servoWheel.attach(46);   // Attach the signal pin of servoWheel to pin 9
  servoCharger.attach(47);
  servoHood1.attach(44);
  servoHood2.attach(45);
  servoLeftDoor.attach(43);
  servoRightDoor.attach(42); 
  servoLeftWindow.attach(41);
  servoRightWindow.attach(40); 
  servoGasPedal.attach(38);
  
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
      setSteeringWheelMotor();
      delay(dt1);
      setLeftDoorAngle();
      delay(dt1);
      setRightDoorAngle();
      delay(dt1);
      setLeftWindowAngle();
      delay(dt1);
      setRightWindowAngle();
      delay(dt1);
      setChargerHatch();
      delay(dt1);
      setHoodMotor();
      delay(dt1);
      break;
    case 2:
      setSteeringWheelMotor();
      delay(dt1);
      setLeftDoorAngle();
      delay(dt1);
      setRightDoorAngle();
      delay(dt1);
      setLeftWindowAngle();
      delay(dt1);
      setRightWindowAngle();
      delay(dt1);
      setChargerHatch();
      delay(dt1);
      setHoodMotor();
      delay(dt1);
      break;
  }
}



//WHEEL SERVO MOTOR FUNCTION
void setSteeringWheelMotor() {
  newWheelAngle = data.wheelAngle;
  servoWheel.write(newWheelAngle);
}

//PEDAL SERVO MOTOR FUNCTION
void setGasPedal() {
  if (data.j1PotX > 160 || data.j1PotX < 100 || data.j1PotY > 160 || data.j2PotX < 100 || data.j2PotX > 160) {
    servoGasPedal.write(179);
  }
  else {
    servoGasPedal.write(0);
  }
}

//DOOR SERVO MOTOR FUNCTION
void setLeftDoorAngle() {
  servoLeftDoor.write(179 - data.leftDoorAngle);
}
void setRightDoorAngle() {
  servoRightDoor.write(data.rightDoorAngle);
}

//WINDOW SERVO MOTOR FUNCTION
void setLeftWindowAngle() {
  servoLeftWindow.write(179 - data.leftWindowAngle);
}
void setRightWindowAngle() {
  servoRightWindow.write(data.rightWindowAngle);
}

//CHARGER SERVO MOTOR FUNCTION
void setChargerHatch() {
  if (data.chargingState == 1) {
    servoCharger.write(openChargerAngle);
  }
  else if (data.chargingState == 0) {
    servoCharger.write(closeChargerAngle);
  }
}

//HOOD SERVO MOTOR FUNCTION
void setHoodMotor() {
  servoHood1_Angle = data.hoodAngle;
  servoHood2_Angle = 179 - data.hoodAngle;
  servoHood1.write(servoHood1_Angle);
  servoHood2.write(servoHood2_Angle); 
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
