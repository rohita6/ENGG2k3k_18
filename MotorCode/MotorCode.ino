/*  Automatic Bridge Control using Ultrasonic Sensor + L298N

    - ENA -> GPIO 2
    - IN1 -> GPIO 4
    - IN2 -> GPIO 16
    - Ultrasonic: TRIG -> GPIO 5, ECHO -> GPIO 18
*/

#define TRIG_PIN 32
#define ECHO_PIN 33

#define ENA 2
#define IN1 4
#define IN2 16

// Motor control functions
void openBridge() { 
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 50);  // PWM speed control (0–255)
}

void closeBridge() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 50);
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

void setup() {
  Serial.begin(115200);

  // Sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Motor driver pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  stopMotor(); // make sure motor is stopped initially
}

void loop() {
  // --- Ultrasonic sensor measurement ---
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0343 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // --- Control logic ---
  if (distance > 0 && distance < 50) { 
    Serial.println("Ship approaching, opening bridge...");
    openBridge();
  } else if (distance > 100) { 
    Serial.println("Ship has passed, closing bridge...");
    closeBridge();
  } else {
    stopMotor();
  }

  delay(500); // wait before next reading
}
/* ///For MVP
#define ENA 2
#define IN1 4
#define IN2 16

void setup() {
  // Motor driver pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  stopMotor(); // start stopped
}

void loop() {
  // Forward (open bridge)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 200);   // adjust speed (0–255)
  delay(5000);             // run 5s

  // Stop
  stopMotor();
  delay(5000);             // wait 5s

  // Backward (close bridge)
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 200);
  delay(5000);             // run 5s

  // Stop again
  stopMotor();
  delay(5000);             // wait 5s before repeating
}

// Function to stop motor
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}
*/
