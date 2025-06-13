#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_DC D4
#define TFT_CS D8

// const char* ssid = "U+Net3EDC";
// const char* password = "HEF5D36@E2";

const char* ssid = "senWiFi_Free";
const char* password = "yh9862500##";

ESP8266WebServer server(80);
Servo power;
Servo tempup;
Servo tempdn;
DHT dht(D3, DHT11);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

int angle = 24;

float temp = 0.0;
float humid = 0.0;

void setup() {
  Serial.begin(115200);

  dht.begin();

  power.attach(D0);
  power.write(14);
  tempup.attach(D1);
  tempup.write(13);
  tempdn.attach(D2);
  tempdn.write(14);

  delay(1000);
  Serial.println("\nSerial started");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  server.on("/", []() {
    server.send(200, "text/html", makePage());
  });
  server.on("/poweron", []() {
    int i;
    for (i=0; i<=angle; i++) {
      power.write(14+i*2);
      delay(1);
    }
    delay(30);
    power.write(14);
    server.send(200, "text/plain", "power");
  });
  server.on("/tempupon", []() {
    int i;
    for (i=0; i<=angle; i++) {
      tempup.write(13+i*2);
      delay(1);
    }
    delay(30);
    tempup.write(13);
    server.send(200, "text/plain", "tempup");
  });
  server.on("/tempdnon", []() {
    int i;
    for (i=0; i<=angle+1; i++) {
      tempdn.write(14+i*2);
      delay(1);
    }
    delay(30);
    tempdn.write(14);
    server.send(200, "text/plain", "tempdn");
  });
  server.on("/tempdata", []() {
    server.send(200, "text/plain", String(temp));
  });
  server.on("/humiddata", []() {
    server.send(200, "text/plain", String(humid));
  });
  server.onNotFound(handleNotFound);

  // Start the server
  server.begin();
  Serial.println();
  Serial.println(WiFi.localIP().toString());

  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_BLUE); tft.setTextSize(2);
  tft.println("Control in");
  tft.setCursor(0,80);
  tft.println("temp: ");
  tft.setCursor(0,124);
  tft.println("humid: ");
  tft.setCursor(0,24);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.println(WiFi.localIP().toString());
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void loop() {
  server.handleClient();

  if (dht.readTemperature() != temp || dht.readHumidity() != humid) {
    temp = dht.readTemperature();
    humid = dht.readHumidity();
    tft.setCursor(92,80);
    tft.print(String(temp)+"C"); tft.println("  ");
    tft.setCursor(92,120);
    tft.print(String(humid)+"%"); tft.println("  ");
  }

}

String htmlPage;
String makePage() {
  String page = R"(<!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <meta name='viewport' content='initial-scale=1, width=device-width'>
    <title>Smart Controller</title>
    <style>
      @font-face {
        font-family: 'D2Coding';
        src: url('https://fastly.jsdelivr.net/gh/projectnoonnu/noonfonts_three@1.0/D2Coding.woff') format('woff');
        font-weight: normal;
        font-style: normal;
      }
      * {
        font-family: D2Coding;
      }
      html {
        font-size: 16px;
        min-height: 100vh;
        display: flex;
        justify-content: center;
        align-items: center;
      }
      @media (min-width: 992px) {
        html {
          font-size: 24px;
        }
      }
      body {
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        gap: 1rem;
      }
      h1 {
        font-size: 2.5rem;
        text-align: center;
      }
      button {
        padding: 0.75rem 1rem;
        border-style: none;
        border-radius: 0.75rem;
        background-color: #84a1d1;
        font-size: 1rem;
        color: #ffffff;
      }
      #buttonWrapper {
        display: flex;
        justify-content: center;
        gap: 1rem;
      }
      #tempButton {
        display: flex;
        flex-direction: column;
        gap: 1rem;
      }
      #dataWrapper {
        padding: 0.5rem;
      }
    </style>
  </head>
  <body>
  <h1>Smart<br>Controller</h1>
  <div id='buttonWrapper'>
    <button id='power'>power</button>
    <div id='tempButton'>
      <button id='tempup'>▲</button>
      <button id='tempdn'>▼</button>
    </div>
  </div>
  <div id='dataWrapper'>
    <p id='temp'></p>
    <p id='humid'></p>
  </div>
  <script>
    document.querySelector("#power").onclick = () => {
      fetch("poweron")
      .then(res => res.text())
      .then(data => console.log(data))
      .catch(e => console.error(e));
    };
    document.querySelector("#tempup").onclick = () => {
      fetch("tempupon")
      .then(res => res.text())
      .then(data => console.log(data))
      .catch(e => console.error(e));
    };
    document.querySelector("#tempdn").onclick = () => {
      fetch("tempdnon")
      .then(res => res.text())
      .then(data => console.log(data))
      .catch(e => console.error(e));
    };

    let temp = 0.0;
    let humid = 0.0;
    let tempText = document.querySelector("#temp");
    let humidText = document.querySelector("#humid");

    setInterval(() => {
      fetch("tempdata")
      .then(res => res.text())
      .then(data => {
        temp = data;
      }).catch(e => console.error(e));
      fetch("humiddata")
      .then(res => res.text())
      .then(data => {
        humid = data;
      }).catch(e => console.error(e));

      tempText.textContent = "온도: "+temp+" °C";
      humidText.textContent = "습도: "+humid+" %";
    }, 2000);
  </script>
  </body>
  </html>)";

  return page;
}