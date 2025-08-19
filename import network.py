import network
import socket

# Wi-Fi credentials
ssid = "MyNetworkSSID"
password = "MySecretPassword"

# Connect to Wi-Fi
wifi = network.WLAN(network.STA_IF)
wifi.active(True)
wifi.connect(ssid, password)

while not wifi.isconnected():
    pass  # Wait until connected

ip = wifi.ifconfig()[0]  # Get assigned IP
print("Connected to Wi-Fi")
print("IP Address:", ip)

# Create a socket bound to your IP and chosen port
port = 8080  # Your custom port
addr = (ip, port)
s = socket.socket()
s.bind(addr)
s.listen(1)

print(f"Web server running at http://{ip}:{port}")

# Simple HTTP server loop
while True:
    cl, client_addr = s.accept()
    print("Client connected from:", client_addr)

    request = cl.recv(1024)  # Read request
    print("Request:", request)

    response = """HTTP/1.1 200 OK
Content-Type: text/html

<html>
    <head><title>ESP32 Server</title></head>
    <body>
        <h1>Hello from ESP32!</h1>
        <p>IP: {}</p>
        <p>Port: {}</p>
    </body>
</html>
""".format(ip, port)

    cl.send(response)
    cl.close()




import machine
import time

trig = machine.Pin(9, machine.Pin.OUT)
echo = machine.Pin(10, machine.Pin.IN)

# Ensure trigger pin is low
trig.value(0)
time.sleep_us(2)

while True:
    # Send a 10 microsecond high pulse to trigger the sensor
    trig.value(1)
    time.sleep_us(10)
    trig.value(0)

    # Wait for echo to go high
    while echo.value() == 0:
        start = time.ticks_us()

    # Wait for echo to go low
    while echo.value() == 1:
        end = time.ticks_us()

    # Calculate duration and distance
    duration = time.ticks_diff(end, start)
    distance = (duration * 0.0343) / 2

    # Print the distance in cm
    print("Distance:", distance, "cm")

    time.sleep(0.5)  # Half-second delay between measurements
