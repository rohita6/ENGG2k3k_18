#include <WiFi.h>

int count = 0;

// --- Ultrasonic sensor pins ---
const int TRIG_PIN1 = 32;  // Sensor 1
const int ECHO_PIN1 = 33;
const int TRIG_PIN2 = 18;  // Sensor 2
const int ECHO_PIN2 = 19;

// --- Motor control pins ---
const int ENA = 2;
const int IN1 = 4;
const int IN2 = 16;

// --- LED pins ---
const int output26 = 26;  // RED LED
const int output27 = 27;  // BLUE LED

// --- Control variables ---
bool manualOverride = false;
bool manualMode = false;
bool shipDetected = false;

// --- Distance variables ---
float distance1 = 0, distance2 = 0, avgDistance = 0;

// --- Height limits (you can adjust these) ---
const float OPEN_LIMIT = 25.0;   // cm
const float CLOSE_LIMIT = 5.0;   // cm

// --- WiFi credentials ---
const char* ssid = "Group_18_AP";
const char* password = "123";

WiFiServer server(80);

String output26State = "CLOSE";
unsigned long previousTime = 0;
const long interval = 500;

// --- Motor control functions ---
void openBridge() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 150); // speed control
}

void closeBridge() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 150);
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

// --- Distance reading ---
void sensors() {
  // Sensor 1
  digitalWrite(TRIG_PIN1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN1, LOW);
  long duration1 = pulseIn(ECHO_PIN1, HIGH, 30000);
  distance1 = duration1 * 0.0343 / 2.0;

  // Sensor 2
  digitalWrite(TRIG_PIN2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN2, LOW);
  long duration2 = pulseIn(ECHO_PIN2, HIGH, 30000);
  distance2 = duration2 * 0.0343 / 2.0;

  // Compute average
  if (distance1 > 0 && distance2 > 0) {
    avgDistance = (distance1 + distance2) / 2.0;
  } else {
    avgDistance = -1;
  }

  Serial.println("------------------------------------------------");
  Serial.print("Sensor 1: ");
  if (distance1 > 0 && distance1 < 400) Serial.print(distance1); else Serial.print("Out of range");
  Serial.println(" cm");

  Serial.print("Sensor 2: ");
  if (distance2 > 0 && distance2 < 400) Serial.print(distance2); else Serial.print("Out of range");
  Serial.println(" cm");

  Serial.print("Average Lift Height: ");
  if (avgDistance > 0) Serial.print(avgDistance); else Serial.print("N/A");
  Serial.println(" cm");
  Serial.println("------------------------------------------------");
}

// --- Web controls ---
String generateControls(int pin, const String& state) {
  String html = "<p>PIN " + String(pin) + " - Bridge Status " + state + "</p>";
  String nextAction = (state == "CLOSE") ? "OPEN" : "CLOSE";
  String buttonColor = (state == "CLOSE") ? "button" : "button button2";
  html += "<p><a href=\"/" + String(pin) + "/" + nextAction + "\"><button class=\"" + buttonColor + "\">" + nextAction + "</button></a></p>";

  String nextMode = manualMode ? "AUTO" : "MANUAL";
  String modeColor = manualMode ? "button button2" : "button";
  html += "<p><a href=\"/MODE/" + nextMode + "\"><button class=\"" + modeColor + "\">" + nextMode + " MODE</button></a></p>";
  return html;
}

// --- Web page display ---
void sendWebPage(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.println(R"rawliteral(
  <!DOCTYPE html><html><head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
  html { font-family: Helvetica; text-align: center; margin: 0px auto; }
  .button { background-color: #00ff37; border: none; color: white;
            padding: 16px 40px; font-size: 30px; cursor: pointer; }
  .button2 { background-color: #ff0019; }
  </style></head><body>
  <h1>ESP32 Bridge Control</h1>
  )rawliteral");

  client.println(generateControls(output26, output26State));

  // Show sensor info
  client.println("<p><b>Sensor 1:</b> " + String(distance1, 2) + " cm</p>");
  client.println("<p><b>Sensor 2:</b> " + String(distance2, 2) + " cm</p>");
  client.println("<p><b>Average Height:</b> " + String(avgDistance, 2) + " cm</p>");

  if (shipDetected)
    client.println("<p>Ship Detected: <span style='color:green;'>TRUE</span></p>");
  else
    client.println("<p>Ship Detected: <span style='color:red;'>FALSE</span></p>");

  client.println("</body></html>");
  client.println();
}

// --- Handle requests ---
void handleRequest(String& header, WiFiClient& client) {
  if (header.indexOf("GET /26/OPEN") >= 0) {
    output26State = "OPEN";
    digitalWrite(output26, HIGH);
    digitalWrite(output27, LOW);
    openBridge();
    manualOverride = true;
  } else if (header.indexOf("GET /26/CLOSE") >= 0) {
    output26State = "CLOSE";
    digitalWrite(output26, LOW);
    digitalWrite(output27, HIGH);
    closeBridge();
    manualOverride = true;
  } else if (header.indexOf("GET /MODE/MANUAL") >= 0) {
    manualMode = true;
    manualOverride = true;
    Serial.println("Switched to MANUAL mode.");
  } else if (header.indexOf("GET /MODE/AUTO") >= 0) {
    manualMode = false;
    manualOverride = false;
    Serial.println("Switched to AUTO mode.");
  }

  sendWebPage(client);
}

// --- Setup ---
void setup() {
  Serial.begin(115200);

  // Pin setup
  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);
  stopMotor();

  // Start WiFi
  WiFi.softAP(ssid, password);
  Serial.print("Access Point IP: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

// --- Loop ---
void loop() {
  unsigned long currentTime = millis();

  WiFiClient client = server.available();
  if (client) {
    String header = "";
    while (client.connected() && client.available()) {
      char c = client.read();
      header += c;
    }
    if (header.length() > 0) handleRequest(header, client);
    client.stop();
  }

  // Read sensors periodically
  if (currentTime - previousTime >= interval) {
    previousTime = currentTime;
    sensors();
  }

  // --- Length-based control ---
  if (avgDistance > 0) {
    // Stop if bridge fully opened or closed
    if (output26State == "OPEN" && avgDistance >= OPEN_LIMIT) {
      Serial.println("Bridge fully open - stopping motor");
      stopMotor();
    } else if (output26State == "CLOSE" && avgDistance <= CLOSE_LIMIT) {
      Serial.println("Bridge fully closed - stopping motor");
      stopMotor();
    }
  }

  // --- AUTO mode ship detection logic ---
  if (!manualMode) {
    if ((distance1 > 0 && distance1 < 50) || (distance2 > 0 && distance2 < 50)) {
      Serial.println("Ship approaching, opening bridge...");
      shipDetected = true;
      digitalWrite(output26, HIGH);
      digitalWrite(output27, LOW);
      openBridge();
    } else if ((distance1 > 100) && (distance2 > 100)) {
      Serial.println("No ship detected, closing bridge...");
      shipDetected = false;
      digitalWrite(output26, LOW);
      digitalWrite(output27, HIGH);
      closeBridge();
    }
  }
}
