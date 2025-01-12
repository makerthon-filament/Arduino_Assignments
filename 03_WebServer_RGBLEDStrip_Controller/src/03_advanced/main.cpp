#include <SPI.h>
#include <WiFiNINA.h>
#include <NeoPixelConnect.h>
#include <DHT.h>
#include <Wire.h>

// WiFi 및 핀 설정
#define SECRET_SSID ""
#define SECRET_OPTIONAL_PASS ""
WiFiServer server(80);
#define DEBUG_MODE_ENABLE 1

// DHT 센서 설정
#define DHT_PIN 2
#define DHTTYPE DHT11
DHT myDHT(DHT_PIN, DHTTYPE);

// LED 스트립 설정
#define NUM_OF_SUB_PIXEL 5
#define PIXEL_CONTROL_PIN 20
NeoPixelConnect myLEDstripSUB(PIXEL_CONTROL_PIN, NUM_OF_SUB_PIXEL, pio0, 0);

// 전역 변수
unsigned char redColor = 0, greenColor = 0, blueColor = 0;
String lampColor = "Black";
float fHumidity, fTemperature;
unsigned long setTimerIntervalForReadTempValue = 2000;
unsigned long readTempValuePreviousMillis = 0;

// 함수 프로토타입 선언
void connectToWiFi();
void readSensorData();
void handleClientRequests();
void processRequest(String currentLine, WiFiClient &client);
void handleSetColor(String currentLine);
void handleLedOn();
void handleLedOff();
void sendData(WiFiClient &client);
String generateHtmlResponse();
String getHeadContent();
String getCss();
String getJavaScript();
String getBodyContent();
void printWifiStatus();

void setup()
{
  Serial.begin(9600);
  myDHT.begin();
  connectToWiFi();
  server.begin();
  printWifiStatus();
}
void loop()
{
  if (millis() - readTempValuePreviousMillis >= setTimerIntervalForReadTempValue)
  {
    readTempValuePreviousMillis = millis();
    readSensorData();
  }
  handleClientRequests();
}

void connectToWiFi()
{
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 5)
  {
    Serial.print("Attempting to connect to: ");
    Serial.println(SECRET_SSID);
    WiFi.begin(SECRET_SSID, SECRET_OPTIONAL_PASS);
    delay(5000);
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi connected successfully!");
  }
  else
  {
    Serial.println("WiFi connection failed.");
  }
}

void readSensorData()
{
  delay(2000);
  fHumidity = myDHT.readHumidity();
  fTemperature = myDHT.readTemperature();
#if (DEBUG_MODE_ENABLE)
  Serial.print(F("Humidity: "));
  Serial.print(fHumidity);
  Serial.print(F("% Temp.: "));
  Serial.print(fTemperature);
  Serial.println(F("°C "));
#endif
}

void handleClientRequests()
{
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
            processRequest(currentLine, client);
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

void processRequest(String currentLine, WiFiClient &client)
{
  if (currentLine.startsWith("GET /setColor?color="))
  {
    handleSetColor(currentLine);
  }
  else if (currentLine.startsWith("GET /builtInLedOn"))
  {
    handleLedOn();
  }
  else if (currentLine.startsWith("GET /builtInLedOff"))
  {
    handleLedOff();
  }
  else if (currentLine.startsWith("GET /getData"))
  {
    sendData(client);
  }
  else if (currentLine.startsWith("GET /"))
  {
    String htmlResponse = generateHtmlResponse();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.print(htmlResponse);
  }
}

void handleSetColor(String currentLine)
{
  String colorValue = currentLine.substring(currentLine.indexOf('=') + 1,
                                            currentLine.indexOf(' ', currentLine.indexOf('=') + 1));
  Serial.print("Received color: ");
  Serial.println(colorValue);
  colorValue.replace("%2C", ",");
  colorValue.replace("%20", " ");
  if (colorValue.startsWith("rgb(") && colorValue.endsWith(")"))
  {
    colorValue.remove(0, 4);
    colorValue.remove(colorValue.length() - 1);
    int red, green, blue;
    int parsedCount = sscanf(colorValue.c_str(), "%d, %d, %d", &red,
                             &green, &blue);
    if (parsedCount == 3)
    {
      redColor = red;
      greenColor = green;
      blueColor = blue;
      myLEDstripSUB.neoPixelFill(redColor, greenColor, blueColor,
                                 true);
      Serial.print("R=");
      Serial.print(redColor);
      Serial.print(" G=");
      Serial.print(greenColor);
      Serial.print(" B=");
      Serial.println(blueColor);
      lampColor = "Custom";
    }
    else
    {
      Serial.println("Failed to parse RGB values.");
    }
  }
  else
  {
    Serial.println("Invalid color value received.");
  }
}

void handleLedOn()
{
  redColor = 255;
  greenColor = 255;
  blueColor = 255;
  myLEDstripSUB.neoPixelFill(redColor, greenColor, blueColor,
                             true);
  Serial.println("LEDs turned ON (White)");
  lampColor = "White";
}
void handleLedOff()
{
  redColor = 0;
  greenColor = 0;
  blueColor = 0;
  myLEDstripSUB.neoPixelFill(redColor, greenColor, blueColor,
                             true);
  Serial.println("LEDs turned OFF");
  lampColor = "Black";
}
void sendData(WiFiClient &client)
{
  String response = "data#";
  response += String(fTemperature, 1) + "#"; // Temperature
  response += String(fHumidity, 1) + "#";    // Humidity
  response += "0#";                          // Placeholder for Light Intensity
  response += "0#";                          // Placeholder for Touch Count
  response += lampColor;                     // Send the current lamp color
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println();
  client.print(response);
}

String generateHtmlResponse()
{
  String htmlResponse = "<!DOCTYPE html>";
  htmlResponse += "<html>";
  htmlResponse += "<head>";
  htmlResponse += getHeadContent();
  htmlResponse += "</head>";
  htmlResponse += "<body>";
  htmlResponse += getBodyContent();
  htmlResponse += "</body>";
  htmlResponse += "</html>";
  return htmlResponse;
}
// Head Contents
String getHeadContent()
{
  String headContent = "<title>IoT-WiFi Edge Device</title>";
  headContent += "<meta name=\"viewport\" content=\"width=devicewidth, initial-scale=1\">";
  headContent += "<link rel=\"icon\" href=\"data:,\">";
  headContent += "<style>" + getCss() + "</style>";
  headContent += "<script src =\"https://cdn.jsdelivr.net/npm/@jaames/iro@5\"></script>";
  headContent += "<script>" + getJavaScript() + "</script>";
  return headContent;
}

// CSS
String getCss()
{
  return "html { font-family: Helvetica; display: inline-block; margin: 0 auto; text-align: center; }"
         "body { text-align: center; font-family: \"Trebuchet MS\", Arial; background-color: #d1d1d6; }"
         "table { border-collapse: collapse; margin-left:auto; margin-right:auto; table-layout: fixed; }"
         "th { padding: 8px; background-color: #032c6e; color: white; }"
         "tr { text-align: center; border: 2px solid #499433; padding: 1px; }"
         "tr:hover { background-color: #bcbcbc; }"
         "td { text-align: center; border: none; padding: 8px; }"
         "th:nth-child(1), td:nth-child(1) { width: 300px; }"
         "th:nth-child(2), td:nth-child(2) { width: 120px; }"
         "th:nth-child(3), td:nth-child(3) { width: 80px; }"
         ".switch { position: relative; display: inline-block; width: 240px; height: 68px; }"
         ".switch input { display: none; }"
         ".slider { position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #858585; border-radius: 34px; }"
         ".slider:before { position: absolute; content: ''; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff;transition : .5s;border - radius : 68px;}"
         "input:checked + .slider { background-color: #1221ec; }"
         "input:checked + .slider:before { transform: translateX(172px); }"
         ".divider { width: 100%; height: 2px; background-color: #000; margin: 20px 0; }"
         "#picker { display: flex; justify-content: center; margin: auto; }";
}

// JavaScript
String getJavaScript()
{
  return "function updateTime() {"
         "var d = new Date();"
         "var t = d.toLocaleTimeString();"
         "document.getElementById('Time').innerHTML = t;"
         "}"
         "function getData() { ajaxLoad('/getData'); }"
         "var ajaxRequest = new XMLHttpRequest();"
         "function ajaxLoad(ajaxURL) {"
         "if(!ajaxRequest){ alert('AJAX is not supported.'); return; }"
         "ajaxRequest.open('GET', ajaxURL, true);"
         "ajaxRequest.onreadystatechange = function() {"
         "if(ajaxRequest.readyState == 4 && ajaxRequest.status == 200) {"
         "var ajaxResult = ajaxRequest.responseText;"
         "var commandArray = ajaxResult.split('#');"
         "if(commandArray[0] == 'data') {"
         "document.getElementById('Temperature').innerHTML = commandArray[1];"
         "document.getElementById('Humidity').innerHTML = commandArray[2];"
         "document.getElementById('Light').innerHTML = commandArray[3];"
         "document.getElementById('TouchCount').innerHTML = commandArray[4];"
         "document.getElementById('LampColor').innerHTML = commandArray[5];"
         "}"
         "}"
         "};"
         "ajaxRequest.send();"
         "}"
         "var myVar1 = setInterval(getData, 2000);"
         "var myVar2 = setInterval(updateTime, 1000);"
         "var colorPicker = new iro.ColorPicker('#picker', {"
         "width: 250,"
         "wheelLightness: false,"
         "color: 'rgb(255, 255, 255)',"
         "borderWidth: 2,"
         "borderColor: '#000',"
         "});"
         "colorPicker.on('input:end', function(color) {"
         "values.innerHTML = color.rgbString;"
         "var xhr = new XMLHttpRequest();"
         "xhr.open('GET', '/setColor?color=' +encodeURIComponent(color.rgbString), true);"
         "xhr.send();"
         "});"
         "function toggleCheckbox(element) {"
         "var xhr = new XMLHttpRequest();"
         "if(element.checked) {"
         "xhr.open('GET', '/builtInLedOn', true);"
         "document.getElementById('values').innerHTML = 'rgb(255, 255, 255)';"
         "} else {"
         "xhr.open('GET', '/builtInLedOff', true);"
         "document.getElementById('values').innerHTML = 'rgb(0, 0, 0)';"
         "}"
         "xhr.send();"
         "}";
}

String getBodyContent()
{
  String bodyContent = "<h1>IoT-WiFi Data Monitoring and Control</h1>";
  bodyContent += "<div class='divider'></div>";
  bodyContent += "<h2>Data Monitoring</h2>";
  bodyContent += "<table>";
  bodyContent += "<tr><th>Data</th><th>Value</th><th>Unit</th></tr>";
  bodyContent += "<tr><td>Temperature</td><td id='Temperature'>0.0</td><td>[&deg;C]</td></tr>";
  bodyContent += "<tr><td>Humidity</td><td id='Humidity'>0.0</td><td>[%]</td></tr>";
  bodyContent += "<tr><td>Light Intensity</td><td id='Light'>0</td><td>[Lux]</td></tr>";
  bodyContent += "<tr><td>Touch Sensor Detection</td><td id='TouchCount'>0</td><td>[CNT]</td></tr>";
  bodyContent += "<tr><td>Lamp Color</td><td id='LampColor'>0</td><td>[COLOR]</td></tr>";
  bodyContent += "</table>";
  bodyContent += "<div class='divider'></div>";
  bodyContent += "<h2>On/Off Control</h2>";
  bodyContent += "<label class='switch'><input type='checkbox' onchange='toggleCheckbox(this)'><span class='slider'></span></label>";
  bodyContent += "<div class='divider'></div>";
  bodyContent += "<h2>Custom Color Control</h2>";
  bodyContent += "<div id='picker' style='margin:auto;width:50%;'></div>";
  bodyContent += "<p id='values'></p>";
  bodyContent += "<p id='Time'>Time</p>";
  return bodyContent;
}

void printWifiStatus()
{
  Serial.println("📡 ===== Wi-Fi 상태 정보 =====");
  Serial.print("📶 네트워크 이름 (SSID): ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("🌐 IP 주소: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("📊 신호 강도 (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.println("================================");
  Serial.println("🌍 웹 브라우저에서 웹 서버에 접속하려면 다음 주소를 입력하세요:");
  Serial.print("➡️ http://");
  Serial.println(ip);
  Serial.println("================================");
}