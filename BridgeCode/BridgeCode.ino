#include <WiFi.h>


const int TRIG_PIN1 = 15;
const int ECHO_PIN1 = 2;
const int TRIG_PIN2 = 4;
const int ECHO_PIN2 = 16;

float distance1 = 0;
float distance2 = 0;

//wifi credentials
const char* ssid = "KaiCenatCentral";
const char* password = "skibiditoilet";


WiFiServer server(80);

String output26State = "off";
String output27State = "off";

const int output26 = 26;
const int output27 = 27;


void sensors() {//SensorCode
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

String generateButton(int pin, const String& state) {// for WebPage
  String html = "<p>GPIO " + String(pin) + " - State " + state + "</p>";
  if (state == "off") {
    html += "<p><a href=\"/" + String(pin) + "/on\"><button class=\"button\">ON</button></a></p>";
  } else {
    html += "<p><a href=\"/" + String(pin) + "/off\"><button class=\"button button2\">OFF</button></a></p>";
  }
  return html;
}

void sendWebPage(WiFiClient& client) {// for WebPage
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.println(R"rawliteral(
    <!DOCTYPE html><html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">
      <style>
        html { font-family: Helvetica; text-align: center; margin: 0px auto; }
        .button { background-color: #4CAF50; border: none; color: white; 
                  padding: 16px 40px; font-size: 30px; cursor: pointer; }
        .button2 { background-color: #555555; }
      </style>
    </head>
    <body>
      <h1>ESP32 Web Server</h1>
  )rawliteral");

  client.println(generateButton(output26, output26State));
  client.println(generateButton(output27, output27State));

  client.println(R"rawliteral(
    </body></html>
  )rawliteral");

  client.println(); // End of HTTP response
}

void handleRequest(String& header, WiFiClient& client) {// for WebPage
  if (header.indexOf("GET /26/on") >= 0) {
    output26State = "on";
    digitalWrite(output26, HIGH);
  } else if (header.indexOf("GET /26/off") >= 0) {
    output26State = "off";
    digitalWrite(output26, LOW);
  } else if (header.indexOf("GET /27/on") >= 0) {
    output27State = "on";
    digitalWrite(output27, HIGH);
  } else if (header.indexOf("GET /27/off") >= 0) {
    output27State = "off";
    digitalWrite(output27, LOW);
  }
  sendWebPage(client);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN1, OUTPUT);//Sensors
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  Serial.print("Setting Access Point");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.print(IP);

  server.begin();
}

unsigned long previousTime = 0;
const long interval = 500; // run every 500ms

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
  if (currentTime - previousTime >= interval) {
      previousTime = currentTime;
      sensors();
  }

}
