#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>

#define WIFI_SSID "Parr WiFi"
#define WIFI_PASSWORD "rfnD9qrW4vAD"

WebServer server(80);

const int RED_PIN = 6;
const int GREEN_PIN = 7;
const int BLUE_PIN = 8;

int lastR = 0;
int lastG = 0;
int lastB = 0;

void setColor(int r, int g, int b) {
  ledcWrite(0, 255 - r);
  ledcWrite(1, 255 - g);
  ledcWrite(2, 255 - b);
}

void sendHtml() {
  String response = R"rawliteral(
    <!DOCTYPE html>
    <html>
      <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Kat ESP32</title>
        <script src="https://cdn.jsdelivr.net/npm/@jaames/iro@5"></script>
        <style>
          body { font-family: sans-serif; text-align: center; background: #f4f4f4; }
          h1 { color: #333; }
          #color-picker { margin: 20px auto; display: flex; justify-content: center; align-items: center; }
        </style>
      </head>
      <body>
        <h1>ESP32 RGB LED Controller</h1>
        <div id="color-picker"></div>
        <script>
          const colorPicker = new iro.ColorPicker("#color-picker", {
            width: 200,
            color: "#00ff00",
            borderWidth: 1,
            borderColor: "#fff",
            handleRadius: 9,
            handleBorderWidth: 2,
            handleBorderColor: "#fff"
          });
          
          let lastSentColor = null;
          let pingInterval = null;
          let currentColor = colorPicker.color.rgb;

          function startPinging() {
            if (!pingInterval) {
              pingInterval = setInterval(() => {
            
                if (!lastSentColor ||
                    currentColor.r !== lastSentColor.r ||
                    currentColor.g !== lastSentColor.g ||
                    currentColor.b !== lastSentColor.b) {
                
                  lastSentColor = { ...currentColor };
                  fetch(`/setcolor?r=${currentColor.r}&g=${currentColor.g}&b=${currentColor.b}`);
                } else {
                
                  clearInterval(pingInterval);
                  pingInterval = null;
                }
              }, 30);
            }
          }

          colorPicker.on('color:change', function(color) {
            currentColor = color.rgb;
            startPinging();
          });
        </script>
      </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", response);
}

void setup() {
  Serial.begin(115200);

  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  
  ledcAttachPin(RED_PIN, 0);
  ledcAttachPin(GREEN_PIN, 1);
  ledcAttachPin(BLUE_PIN, 2);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", sendHtml);

  server.on("/setcolor", []() {
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();
    lastR = r;
    lastG = g;
    lastB = b;
    setColor(r, g, b);
    sendHtml();
  });
  
  server.on("/off", []() {
    setColor(0, 0, 0);
    sendHtml();
  });
  
  server.on("/on", []() {
    setColor(lastR, lastG, lastB);
    sendHtml();
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
