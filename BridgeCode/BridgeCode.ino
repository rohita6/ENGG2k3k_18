#define TRIG_PIN 5
#define ECHO_PIN 18
#include WiFi.h
float distance = 0

//SSID of your network
char ssid[] = "yourNetwork";
//password of your WPA Network
char pass[] = "secretPassword";

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, pass);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("This is working");
  }
  Serial.println("\nConnected!");
}

void loop() {
  // Send a 10us pulse to trigger measurement
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo pulse
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance in cm (speed of sound: ~343 m/s)
  distance = duration * 0.0343 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(1000);
}
