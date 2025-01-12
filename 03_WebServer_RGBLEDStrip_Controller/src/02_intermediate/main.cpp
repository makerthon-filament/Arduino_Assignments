#include <SPI.h>
#include <WiFiNINA.h>
#include <NeoPixelConnect.h>
#include <Arduino_LSM6DSOX.h>
#define SECRET_SSID ""
#define SECRET_OPTIONAL_PASS ""

const char SSID[] = SECRET_SSID;
const char PASS[] = SECRET_OPTIONAL_PASS;
#define NUM_OF_SUB_PIXEL 5
#define SUB_PIXEL_CONTROL_PIN 20
NeoPixelConnect myLEDstripSUB(SUB_PIXEL_CONTROL_PIN, NUM_OF_SUB_PIXEL, pio0, 0);
unsigned char redColor = 0, greenColor = 0, blueColor = 0;
int temperature_deg = 0;
unsigned long setTimerIntervalForReadTempValue = 2000;
unsigned long readTempValuePreviousMillis = 0;
int status = WL_IDLE_STATUS;
WiFiServer server(80);
void printWifiStatus();
String generateHtmlResponse();

void setup()
{
  Serial.begin(9600);
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(SSID);
    status = WiFi.begin(SSID, PASS);
    delay(10000);
  }
  server.begin();
  printWifiStatus();
}
void loop()
{
  if (IMU.temperatureAvailable())
  {
    if ((unsigned long)(millis() - readTempValuePreviousMillis) >= setTimerIntervalForReadTempValue)
    {
      readTempValuePreviousMillis = millis();
      IMU.readTemperature(temperature_deg);
      Serial.print("LSM6DSOX Temperature = ");
      Serial.print(temperature_deg);
      Serial.println(" °C");
    }
  }
  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("new client");
    String currentLine = "";
    bool currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        if (c == '\n')
        {
          if (currentLineIsBlank)
          {
            if (currentLine.startsWith("GET /setColor?color="))
            {
              String colorValue = currentLine.substring(currentLine.indexOf('=') + 1, currentLine.indexOf(' ', currentLine.indexOf('=') + 1));
              Serial.print("Received color: ");
              Serial.println(colorValue);
              colorValue.replace("%23", "#");
              colorValue.replace(" ", "");
              if (colorValue.startsWith("#") && (colorValue.length() == 7 || colorValue.length() == 4))
              {
                colorValue.replace("#", "");
                long hexValue = strtol(colorValue.c_str(), NULL, 16);
                redColor = (hexValue >> 16) & 0xFF;
                greenColor = (hexValue >> 8) & 0xFF;
                blueColor = hexValue & 0xFF;
                myLEDstripSUB.neoPixelFill(redColor, greenColor, blueColor, true);
                analogWrite(LEDR, 255 - redColor);
                analogWrite(LEDG, 255 - greenColor);
                analogWrite(LEDB, 255 - blueColor);
                Serial.print("Color set to: R=");
                Serial.print(redColor);
                Serial.print(" G=");
                Serial.print(greenColor);
                Serial.print(" B=");
                Serial.println(blueColor);
              }
              else
              {
                Serial.println("Invalid color value received.");
              }
            }
            String htmlResponse = generateHtmlResponse();
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();
            client.print(htmlResponse);
            delay(1);
            break;
          }
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          currentLine += c;
          currentLineIsBlank = false;
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
}
// Function to generate HTML response
String generateHtmlResponse()
{
  String htmlResponse = "<!DOCTYPE HTML>";
  htmlResponse += "<html>";
  htmlResponse += "<head>";
  htmlResponse += "<title>IoT-WiFi & Web Server RGB LED Control Project</title>";
  htmlResponse += "<style>";
  htmlResponse += "body { text-align: center; }";
  htmlResponse += ".container {margin: 0 auto; text-align: center; margin-top: 25px;}";
  htmlResponse += "button {color: white; width: 100px; height: 100px;";
  htmlResponse += "border-radius: 50%; margin: 20px; border: none; font-size: 20px; outline: none; transition: all 0.2s;}";
  htmlResponse += ".red{background-color: rgb(196, 39, 39);}";
  htmlResponse += ".green{background-color: rgb(39, 121, 39);}";
  htmlResponse += ".blue {background-color: rgb(5, 87, 180);}";
  htmlResponse += ".white {background-color: rgb(200, 200, 200);}";
  htmlResponse += ".off{background-color: grey;}";
  htmlResponse += "button:hover{cursor: pointer; opacity: 0.7;}";
  htmlResponse += "#colorPickerContainer { text-align: center; margin-top: 20px; }";
  htmlResponse += "#colorPicker { width: 200px; height: 200px; }";
  htmlResponse += ".sensor-data { font-size: 36px; }";
  htmlResponse += "</style>";
  htmlResponse += "</head>";
  htmlResponse += "<body>";
  htmlResponse += "<h1>IoT-WiFi & Web Server RGB LED Control Project</h1>";
  htmlResponse += "<hr />";
  htmlResponse += "<p class='sensor-data'>Onboard Temp[&deg;C]: ";
  htmlResponse += String(temperature_deg);
  htmlResponse += "</p>";
  // Color control buttons
  htmlResponse += "<h2>R, G, B, W Color Control</h2>";
  htmlResponse += "<div class='container'>";
  htmlResponse += "<button class='red' type='button' onclick='setColor(\"#FF0000\")'>R</button>";
  htmlResponse += "<button class='green' type='button' onclick='setColor(\"#00FF00\")'>G</button>";
  htmlResponse += "<button class='blue' type='button' onclick='setColor(\"#0000FF\")'>B</button>";
  htmlResponse += "<button class='white' type='button' onclick='setColor(\"#FFFFFF\")'>W</button>";
  htmlResponse += "<button class='off' type='button' onclick='setColor(\"#000000\")'>OFF</button>";
  htmlResponse += "</div>";
  // Color picker
  htmlResponse += "<h2>Color Picker</h2>";
  htmlResponse += "<div id='colorPickerContainer'>";
  htmlResponse += "<input type='color' id='colorPicker' value='#000000'>";
  htmlResponse += "</div>";
  // JavaScript to handle color setting
  htmlResponse += "<script>";
  htmlResponse += "let lastColor = '';";
  htmlResponse += "let isRequesting = false;";
  htmlResponse += "function setColor(color) {";
  htmlResponse += " document.getElementById('colorPicker').value = color;";
  htmlResponse += " if (color !== lastColor && !isRequesting) {";
  htmlResponse += " lastColor = color;";
  htmlResponse += " isRequesting = true;";
  htmlResponse += " var xhr = new XMLHttpRequest();";
  htmlResponse += " xhr.open('GET', '/setColor?color=' + encodeURIComponent(color), true);";
  htmlResponse += " xhr.send();";
  htmlResponse += " xhr.onload = function() {";
  htmlResponse += " console.log('Response received:', xhr.responseText);";
  htmlResponse += " isRequesting = false;";
  htmlResponse += " };";
  htmlResponse += " xhr.onerror = function() {";
  htmlResponse += " console.error('Request failed');";
  htmlResponse += " isRequesting = false;";
  htmlResponse += " };";
  htmlResponse += " }";
  htmlResponse += "}";
  htmlResponse += "document.getElementById('colorPicker').addEventListener('input', function() {";
  htmlResponse += " setColor(this.value);";
  htmlResponse += "});";
  htmlResponse += "</script>";
  htmlResponse += "</body>";
  htmlResponse += "</html>";
  return htmlResponse;
}

void printWifiStatus()
{
  // Wi-Fi 네트워크 이름 출력
  Serial.println("📡 ===== Wi-Fi 상태 정보 =====");
  Serial.print("📶 네트워크 이름 (SSID): ");
  Serial.println(WiFi.SSID());

  // 현재 할당된 IP 주소 출력
  IPAddress ip = WiFi.localIP();
  Serial.print("🌐 IP 주소: ");
  Serial.println(ip);

  // Wi-Fi 신호 강도 (RSSI) 출력
  long rssi = WiFi.RSSI();
  Serial.print("📊 신호 강도 (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

  // 웹 브라우저에서 웹 서버 확인 URL 출력
  Serial.println("================================");
  Serial.println("🌍 웹 브라우저에서 웹 서버에 접속하려면 다음 주소를 입력하세요:");
  Serial.print("➡️ http://");
  Serial.println(ip);
  Serial.println("================================");
}
