const int TRIG_PIN1 = 5;
const int ECHO_PIN1 = 18;
const int TRIG_PIN2 = 22;
const int ECHO_PIN2 = 23;

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "YourWiFiName";
const char* password = "YourWiFiPassword";

float distance1 = 0;
float distance2 = 0;

int ledPin = 2;      // GPIO2 onboard LED (blue)
bool ledState = false;
int counter = 0;

void handleRoot() {
  String page = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>ESP32 AJAX Demo</title>
    </head>
    <body>
      <h1>ESP32 AJAX Example</h1>
      <p>LED State: <span id="led">OFF</span></p>
      <button onclick="toggleLED()">Toggle LED</button>
      <p>Counter: <span id="count">0</span></p>
      <button onclick="getCounter()">Refresh Counter</button>

      <script>
        function toggleLED() {
          fetch('/toggle')
            .then(response => response.text())
            .then(data => {
              document.getElementById('led').innerText = data;
            });
        }

        function getCounter() {
          fetch('/counter')
            .then(response => response.text())
            .then(data => {
              document.getElementById('count').innerText = data;
            });
        }
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", page);
}

void handleToggle() {  
  ledState = !ledState;
  digitalWrite(ledPin, ledState ? HIGH : LOW);
  server.send(200, "text/plain", ledState ? "ON" : "OFF");
}

void handleCounter() {
  counter++;
  server.send(200, "text/plain", String(counter));
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

 //  LED control based on distance
  if (distance1 > 0 && distance1 < 50 || distance2 > 0 && distance2 < 50) {
    digitalWrite(ledPin, HIGH);   // Turn LED ON
  } else {
    digitalWrite(ledPin, LOW);    // Turn LED OFF
  }


  delay(1000);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN1, OUTPUT);//Sensors
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  WiFi.begin(ssid, password); // connect to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to WiFi! IP address: ");
  Serial.println(WiFi.localIP());

  // Define routes
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/counter", handleCounter);

  server.begin();
}



void loop() {
  sensors();//ping sensors
  server.handleClient();
}
