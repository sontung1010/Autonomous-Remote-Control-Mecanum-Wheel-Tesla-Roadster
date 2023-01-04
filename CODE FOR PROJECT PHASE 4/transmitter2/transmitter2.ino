#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include <NewPing.h>
#include <Servo.h> 
#include "dht.h" 

//DECLARE VARIABLE 

//RF24
RF24 radio(7, 8);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001"; // Address

//Servo
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
  byte distance;
  byte distanceR = 0;
  byte distanceL = 0;
};
Data_Package data; //Create a variable with the above structure

void setup() {
  //Serial
  Serial.begin(9600);

  //Radio communication
  radio.begin();
  radio.openWritingPipe(address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(9);

  //Servo
  myservo.attach(4); 
}

void loop() {
  radio.stopListening();  //Set the module as trasnmitter
  data.distance = readPing();
  Serial.print("data.distance = ");
  Serial.println(data.distance);
  radio.write(&data, sizeof(Data_Package));   // Send the whole data from the structure to the receiver
}

//SENSOR FUNCTION
int readPing() {
  // Read temperature and humidity from DHT11 sensor
  readDHT = DHT.read11(dht11); // Reads the data from the sensor
  temp = DHT.temperature; // Gets the values of the temperature
  hum = DHT.humidity; // Gets the values of the humidity
  Serial.print("temp = ");
  Serial.println(temp);
  Serial.print("hum = ");
  Serial.println(hum);


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
