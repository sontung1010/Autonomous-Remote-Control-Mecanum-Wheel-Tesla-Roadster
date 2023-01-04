#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//DECLARE VARIABLES
float dt1 = 1.5;

//RF24
RF24 radio(7, 8);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//LEDs
int headlight1 = 9;
int headlight2 = 6;
int headlight3 = 5;
int headlight4 = 3;
int brakeLED1 = 4;
int brakeLED2 = 2;

//Photoresistor
int lightPh = A0;
int lightVal;

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

//SETUP
void setup() {
  //Radio communication  
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(9);

  //pinModes
  pinMode(headlight1, OUTPUT);
  pinMode(headlight2, OUTPUT);
  pinMode(headlight3, OUTPUT);
  pinMode(headlight4, OUTPUT);
  pinMode(brakeLED1, OUTPUT);
  pinMode(brakeLED2, OUTPUT);
  pinMode(lightPh, INPUT);
  
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
      setHeadlightBrightness();
      delay(dt1);
      turnOnBrakeLED();
      delay(dt1);
      break;
    case 2:
      setHeadlightBrightness();
      delay(dt1);
      turnOnBrakeLED();
      delay(dt1);
      break;
    case 3:
      setHeadlightBrightness();
      delay(dt1);
      turnOnBrakeLED();
      delay(dt1);
      break;
    case 4:
      setHeadlightBrightness_4();
      break;
  }
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Monitor the battery voltage
  int sensorValue = analogRead(A0);
//  float voltage = sensorValue * (5.0 / 1023.00) * 3; // Convert the reading values from 5v to suitable 12V i
//  // If voltage is below 11V turn on the LED
//  if (voltage < 11) {
//    digitalWrite(led, HIGH);
//  }
//  else {
//    digitalWrite(led, LOW);
//  }
}



//HEADLIGHT BRIGHTNESS FUNCTION
void setHeadlightBrightness() {
  analogWrite(headlight1, data.headlightPot);
  analogWrite(headlight2, data.headlightPot);
  analogWrite(headlight3, data.headlightPot);
  analogWrite(headlight4, data.headlightPot);
}
void setHeadlightBrightness_4() {
  lightVal = map(analogRead(lightPh), 1023, 0, 0, 255);
//  Serial.println(lightVal);
  analogWrite(headlight1, lightVal);
  analogWrite(headlight2, lightVal);
  analogWrite(headlight3, lightVal);
  analogWrite(headlight4, lightVal);
}

//BRAKE LIGHT FUNCTION
void turnOnBrakeLED() {
  if (data.j1PotY < 100 || data.j2PotX < 100 || data.j2PotX > 150) {
    digitalWrite(brakeLED1, HIGH);
    digitalWrite(brakeLED2, HIGH);
  }
  if (data.j1PotY >= 100 && data.j2PotX >= 100 && data.j2PotX <= 150) {
    digitalWrite(brakeLED1, LOW);
    digitalWrite(brakeLED2, LOW);
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
