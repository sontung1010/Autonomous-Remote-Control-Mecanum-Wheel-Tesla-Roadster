#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include <NewPing.h>
#include <Servo.h> 
#include "dht.h" 

//DECLARE VARIABLES
float dt1 = 1.5;

//RF24
RF24 radio(7, 8);   // nRF24L01 (CE, CSN)
const byte addresses[][6] = {"00001","00002"};
//const byte addresss[6] = "00002";
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//Servo motors
Servo servoWiper;
Servo myservo;

//Sensor
#define TRIGGER_PIN 2
#define ECHO_PIN 3
#define MAX_DISTANCE 400
#define dht11 5 // DHT11 temperature and humidity sensor
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
dht DHT; // Creats a DHT object
int readDHT, temp, hum;
float speedOfSound, distance, duration;
int distanceF;
int distanceR;
int distanceL;

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
  byte vehicleMovement;
};
Data_Package data; //Create a variable with the above structure

//SETUP
void setup() {
  //Radio communication  
  radio.begin();
  radio.openReadingPipe(1,addresses[0]);
  radio.openWritingPipe(addresses[1]);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(9);

  //Servo
  servoWiper.attach(6);
  myservo.attach(4);  
  myservo.write(115);
  
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
      radio.stopListening();
      Serial.println(data.j1PotX);
//      data.distance = readPing();
      send_vehicleMovement();
      Serial.print("data.vehicleMovement = ");
      Serial.println(data.vehicleMovement);
//      data.distanceR = lookRight();
//      data.distanceL = lookLeft();
      radio.write(&data, sizeof(Data_Package));
      break;
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
void send_vehicleMovement() {
  distanceF = readPing();
  if(distanceF <= 16) {
//    stopOrder();
    data.vehicleMovement = 1;
    Serial.print("data.vehicleMovement = ");
    Serial.println(data.vehicleMovement);
    delay(100);
//    moveBackwardOrder();
    data.vehicleMovement = 2;
    Serial.print("data.vehicleMovement = ");
    Serial.println(data.vehicleMovement);
    delay(300);
//    stopOrder();
    data.vehicleMovement = 1;
    Serial.print("data.vehicleMovement = ");
    Serial.println(data.vehicleMovement);
    delay(200);
    distanceR = lookRight();
    delay(200);
    distanceL = lookLeft();
    delay(200);

    if(distanceR >= distanceL) {
//      moveSidewaysRightOrder();
      do {
        data.vehicleMovement = 3;
        Serial.print("data.vehicleMovement = ");
        Serial.println(data.vehicleMovement);
        distanceF = readPing();
//      delay(6000);
      } while (distanceF < 16);
//      data.vehicleMovement = 3;
//      Serial.print("data.vehicleMovement = ");
//      Serial.println(data.vehicleMovement);
//      delay(6000);
//      stopOrder();
      data.vehicleMovement = 1;
      Serial.print("data.vehicleMovement = ");
      Serial.println(data.vehicleMovement);
    } 
    else {
//      moveSidewaysLeftOrder();
      do {
        data.vehicleMovement = 4;
        Serial.print("data.vehicleMovement = ");
        Serial.println(data.vehicleMovement);
        distanceF = readPing();
      } while (distanceF < 16);
//      data.vehicleMovement = 4;
//      Serial.print("data.vehicleMovement = ");
//      Serial.println(data.vehicleMovement);
//      delay(6000);
//      stopOrder();
      data.vehicleMovement = 1;
      Serial.print("data.vehicleMovement = ");
      Serial.println(data.vehicleMovement);
    }
  }
  else {
//    moveForwardOrder();
      data.vehicleMovement = 5;
      Serial.print("data.vehicleMovement = ");
      Serial.println(data.vehicleMovement);
  }
}
int readPing() {
  // Read temperature and humidity from DHT11 sensor
  readDHT = DHT.read11(dht11); // Reads the data from the sensor
  temp = DHT.temperature; // Gets the values of the temperature
  hum = DHT.humidity; // Gets the values of the humidity
//  Serial.print("temp = ");
//  Serial.println(temp);
//  Serial.print("hum = ");
//  Serial.println(hum);


  speedOfSound = 331.4 + (0.6 * temp) + (0.0124 * hum); // Calculate speed of sound in m/s

  duration = sonar.ping_median(10); // 10 interations - returns duration in microseconds
  duration = duration/1000000; // Convert mircroseconds to seconds
  distance = (speedOfSound * duration)/2;
  distance = distance * 100; // meters to centimeters

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println("cm");

  return distance;
}
int lookRight() {
  myservo.write(50); 
  delay(300);
  int distance = readPing();
  delay(100);
  myservo.write(115); 
  return distance;
}
int lookLeft() {
  myservo.write(170); 
  delay(300);
  int distance = readPing();
  delay(100);
  myservo.write(115); 
  return distance;
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
