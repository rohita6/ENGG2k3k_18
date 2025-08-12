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
