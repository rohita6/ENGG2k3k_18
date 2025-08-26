#define TRIG_PIN1 5
#define ECHO_PIN1 18
#define TRIG_PIN2 22
#define ECHO_PIN2 23
float distance1 = 0;
float distance2 = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);
}

void sensors() {
  // Send a 10us pulse to trigger measurement for sensor 1
  digitalWrite(TRIG_PIN1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN1, LOW);
  // Send a 10us pulse to trigger measurement for sensor 2
  digitalWrite(TRIG_PIN2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN2, LOW);
  // Read the echo pulse
  long duration1 = pulseIn(ECHO_PIN1, HIGH);
  long duration2 = pulseIn(ECHO_PIN2, HIGH);

  // Calculate distance in cm (speed of sound: ~343 m/s)
  distance1 = duration1 * 0.0343 / 2;
  distance2 = duration2 * 0.0343 / 2;

  Serial.print("Distance1: ");
  Serial.print(distance1);
  Serial.println(" cm");
  //
  Serial.print("Distance2: ");
  Serial.print(distance2);
  Serial.println("cm");

  delay(1000);
}

void loop() {
  sensors();
}
