#define TRIG_PIN 5
#define ECHO_PIN 18
#define MOTOR_OPEN_PIN 12 // IN1A
#define MOTOR_CLOSE_PIN 14 // IN2A

void openBridge() { 
  digitalWrite(MOTOR_OPEN_PIN, HIGH);
  digitalWrite(MOTOR_CLOSE_PIN, LOW);
}

void closeBridge() {
  digitalWrite(MOTOR_OPEN_PIN, LOW);
  digitalWrite(MOTOR_CLOSE_PIN, HIGH);
}

void stopMotor() {
  digitalWrite(MOTOR_OPEN_PIN, LOW);
  digitalWrite(MOTOR_CLOSE_PIN, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Setup motor pins
  pinMode(MOTOR_OPEN_PIN, OUTPUT);
  pinMode(MOTOR_CLOSE_PIN, OUTPUT);
}

void loop() {
  // Sensor code from the previous example
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  
  
  float distance = duration * 0.0343 / 2;

  // Control Logic based on distance
  // Replace the placeholder values (e.g., 50cm, 10cm) with your final project values
  if (distance < 50) { // If a ship is approaching (distance is less than 50 cm)
    Serial.println("Ship approaching, opening bridge...");
    openBridge();
  } else if (distance > 100) { // If the ship has passed (distance is greater than 100 cm)
    Serial.println("Ship has passed, closing bridge...");
    closeBridge();
  } else {
    stopMotor();
  }

  delay(1000); // Wait for a second before the next measurement
}
