#include <L298NX2.h>
#include <NewPing.h>

#define MAX_DISTANCE 500

#define SONAR_NUM 4      // Number of sensors.

unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
unsigned long currentTimer = 0;
unsigned long previousTimer = 0;

NewPing sonar[SONAR_NUM] = {   // Sensor object array.
  NewPing(48, 49, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping. 
  NewPing(46, 47, MAX_DISTANCE), 
  NewPing(44, 45, MAX_DISTANCE),
  NewPing(40, 41, MAX_DISTANCE)
};

int n1 = random(150, 350);
int n2 = random(150, 175);

int dist[12];
int command = 0;

float temp = 23.7; //Temp in C degree
float factor = sqrt(1 + temp / 273.15) / 60.368;

const unsigned int EN_FL = 2;
const unsigned int IN1_FL = 25;
const unsigned int IN2_FL = 24;

const unsigned int IN1_FR = 27;
const unsigned int IN2_FR = 26;
const unsigned int EN_FR = 3;

const unsigned int EN_RL = 6;
const unsigned int IN1_RL = 31;
const unsigned int IN2_RL = 33;

const unsigned int IN1_RR = 30;
const unsigned int IN2_RR = 32;
const unsigned int EN_RR = 5;

L298NX2 Fmotors(EN_FL, IN1_FL, IN2_FL, EN_FR, IN1_FR, IN2_FR);
L298NX2 Rmotors(EN_RL, IN1_RL, IN2_RL, EN_RR, IN1_RR, IN2_RR);


int speed = 255;
// int directionA = L298N::FORWARD;
int value = 0;
int distanceL;
int distanceR;
int distanceFL;
int distanceFR;
// int distance;

unsigned short FLspeed = speed;
unsigned short FRspeed = speed;
unsigned short RLspeed = speed;
unsigned short RRspeed = speed;

void setup() {
  Serial.begin(9600);
  setSpeed();
  
  previousTimer = millis();
}

void loop() {
  delay(2000);
  if (value == 0) {
    for (int i = 0;i < 12; i++) {
      dist[i] = (float)sonar[0].ping_median(5) * factor;
      Serial.println(dist[i]);
      moveForward();
      delay(300);
      moveBackward();
      delay(80);
      // Serial.println("move forward");
      stopMoving();
      delay(200);
    }
    // value = 1;
    stopMoving();
    delay(300);
    for (int i = 0; i < 12; i++) {
      if (dist[i] < 35) {
        command = 0;
      }
      else {
        command = 1;
      }
    }
  }
  value = 1;
  // stopMoving();
  // delay(300);
  // for (int i = 0; i < 10; i++) {
  //   if (dist[i] < 35) {
  //     command = 0;
  //   }
  //   else {
  //     command = 1;
  //   }
  // }
  if (command == 1) {
    moveBackward();
    delay(600);
    moveForward();
    delay(80);
    stopMoving();
    delay(200);
    moveRightSideways();
    delay(1200);
    moveLeftSideways();
    delay(80);
    stopMoving();
    // command = 0;
  }
  command = 0;
  
  // moveLeftSideways();
}

void setSpeed() {
  FLspeed = speed;
  FRspeed = speed;
  RLspeed = speed;
  RRspeed = speed;
  Fmotors.setSpeedA(FLspeed);
  Fmotors.setSpeedB(FRspeed);
  Rmotors.setSpeedA(RRspeed);
  Rmotors.setSpeedB(RLspeed);
}

void moveForward() {
  Fmotors.setSpeedA(255);
  Fmotors.setSpeedB(255);
  Rmotors.setSpeedA(255);
  Rmotors.setSpeedB(255);
  Fmotors.backwardA();
  Fmotors.backwardB();
  Rmotors.forwardA();
  Rmotors.backwardB();
}
void moveBackward() {
  Fmotors.setSpeedA(255);
  Fmotors.setSpeedB(255);
  Rmotors.setSpeedA(255);
  Rmotors.setSpeedB(255);
  Fmotors.forwardA();
  Fmotors.forwardB();
  Rmotors.backwardA();
  Rmotors.forwardB();
}
void moveLeftSideways() {
  Fmotors.setSpeedA(255);
  Fmotors.setSpeedB(255);
  Rmotors.setSpeedA(255);
  Rmotors.setSpeedB(255);
  Fmotors.forwardA();
  Fmotors.backwardB();
  Rmotors.forwardB();
  Rmotors.forwardA(); 
}
void moveRightSideways() {
  Fmotors.setSpeedA(255);
  Fmotors.setSpeedB(255);
  Rmotors.setSpeedA(255);
  Rmotors.setSpeedB(255);
  Fmotors.backwardA();
  Fmotors.forwardB();
  Rmotors.backwardB();
  Rmotors.backwardA(); 
}
void rotateLeft() {
  Fmotors.setSpeedA(255);
  Fmotors.setSpeedB(255);
  Rmotors.setSpeedA(255);
  Rmotors.setSpeedB(255);
  Fmotors.backwardA();
  Fmotors.forwardB();
  Rmotors.forwardA();
  Rmotors.forwardB();  
}
void rotateRight() {
  Fmotors.setSpeedA(255);
  Fmotors.setSpeedB(255);
  Rmotors.setSpeedA(255);
  Rmotors.setSpeedB(255);
  Fmotors.backwardB();
  Fmotors.forwardA();
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