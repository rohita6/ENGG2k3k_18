/*  Automatic Bridge Control using Ultrasonic Sensor + L298N

    - ENA -> GPIO 2
    - IN1 -> GPIO 4
    - IN2 -> GPIO 16
    - Ultrasonic: TRIG -> GPIO 5, ECHO -> GPIO 18
*/


#define ENA 2
#define IN1 4
#define IN2 16
#include <WiFi.h>
// GPIO pins






void setup() {
  Serial.begin(115200);



  // Motor driver pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  //LED setup

}


void loop() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 150); 
}