// LSM6DSOX 센서 활용 웹서버로 Neopixel RGB LED 제어하기

#include <SPI.h>
#include <WiFiNINA.h>
#include <NeoPixelConnect.h>
#include <Arduino_LSM6DSOX.h>

#define SECRET_SSID ""
#define SECRET_OPTIONAL_PASS ""
const char SSID[] = SECRET_SSID;
const char PASS[] = SECRET_OPTIONAL_PASS;

// NeoPixel 설정
#define NUM_OF_SUB_PIXEL 5                // NeoPixel LED 개수
const uint8_t SUB_PIXEL_CONTROL_PIN = 20; // NeoPixel 데이터 핀
NeoPixelConnect myLEDstripSUB(SUB_PIXEL_CONTROL_PIN, NUM_OF_SUB_PIXEL, pio1, 1);

// 변수 선언
void printWifiStatus();
unsigned char cmdMode, redColor, greenColor, blueColor, valueSetBrightness = 150;
unsigned char getRedColor = 0, getGreenColor = 0, getBlueColor = 0;
int temperature_deg = 0;                               // 온도 데이터 저장
unsigned long setTimerIntervalForReadTempValue = 2000; // 온도 읽기 간격 (2초)
unsigned long readTempValuePreviousMillis = 0;
int status = WL_IDLE_STATUS; // Wi-Fi 연결 상태
WiFiServer server(80);       // 웹 서버 포트 설정

void setup()
{
    // 온보드 LED 설정
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    Serial.begin(115200);

    // LSM6DSOX 센서 초기화
    if (!IMU.begin())
    {
        Serial.println("Failed to initialize IMU!");
        while (1)
            ;
    }

    // Wi-Fi 모듈 확인
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");
        while (true)
            ;
    }

    // Wi-Fi 펌웨어 버전 확인
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    {
        Serial.println("Please upgrade the firmware");
    }

    // Wi-Fi 연결
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to Network named: ");
        Serial.println(SSID);
        status = WiFi.begin(SSID, PASS);
        delay(10000);
    }

    // 웹 서버 시작
    server.begin();
    printWifiStatus();

    // NeoPixel 초기 상태 설정
    redColor = 100;
    greenColor = 100;
    blueColor = 100;

    // NeoPixel LED 초기 애니메이션
    myLEDstripSUB.neoPixelFill(255, 0, 0, true);
    delay(2000); // 빨간색
    myLEDstripSUB.neoPixelFill(0, 255, 0, true);
    delay(2000); // 초록색
    myLEDstripSUB.neoPixelFill(0, 0, 255, true);
    delay(2000);                                                                   // 파란색
    myLEDstripSUB.neoPixelFill(redColor / 2, greenColor / 2, blueColor / 2, true); // 초기 밝기 설정
}

// 메인 루프
void loop()
{
    // 온도 센서 데이터 읽기
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

    // 클라이언트 연결 확인
    WiFiClient client = server.available();
    if (client)
    {
        Serial.println("new client");
        String currentLine = "";
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                Serial.write(c);

                // HTTP 요청 처리
                if (c == '\n')
                {
                    if (currentLine.length() == 0)
                    {
                        // HTTP 헤더 전송
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connection: close");
                        client.println();

                        // HTML 페이지 전송
                        client.println("<!DOCTYPE HTML>");
                        client.println("<html>");
                        client.println("<head>");
                        client.print("<title>IoT-WiFi & Web Server LED Control Project</title>");
                        client.println("<style>");
                        client.print(".container {margin: 0 auto; text-align: center; margin-top: 25px;}");
                        client.print("button {color: white; width: 100px; height: 100px;");
                        client.print("border-radius: 50%; margin: 20px; border: none; font-size: 20px; outline: none; transition: all 0.2s;}");
                        client.print(".red {background-color: rgb(196, 39, 39);}");
                        client.print(".green {background-color: rgb(39, 121, 39);}");
                        client.print(".blue {background-color: rgb(5, 87, 180);}");
                        client.print(".white {background-color: rgb(200, 200, 200);}");
                        client.print(".off {background-color: grey;}");
                        client.print("button:hover {cursor: pointer; opacity: 0.7;}");
                        client.print("</style>");
                        client.println("</head>");
                        client.println("<body>");
                        client.println("<h1>IoT-WiFi & Web Server LED Control Project</h1>");
                        client.println("<hr />");
                        client.println("<br />");
                        client.print("Onboard Temp[&deg;C]: ");
                        client.print(temperature_deg);
                        client.println("<br /><br />");
                        client.print("<div class='container'>");
                        client.print("<button class='red' onclick='location.href=\"/RH\"'>RED ON</button>");
                        client.print("<button class='off' onclick='location.href=\"/RL\"'>RED OFF</button><br>");
                        client.print("<button class='green' onclick='location.href=\"/GH\"'>GREEN ON</button>");
                        client.print("<button class='off' onclick='location.href=\"/GL\"'>GREEN OFF</button><br>");
                        client.print("<button class='blue' onclick='location.href=\"/BH\"'>BLUE ON</button>");
                        client.print("<button class='off' onclick='location.href=\"/BL\"'>BLUE OFF</button><br>");
                        client.print("<button class='white' onclick='location.href=\"/WH\"'>WHITE ON</button>");
                        client.print("<button class='off' onclick='location.href=\"/WL\"'>WHITE OFF</button>");
                        client.print("</div>");
                        client.println("</body>");
                        client.println("</html>");
                        client.println();
                        break;
                    }
                    else
                    {
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {
                    currentLine += c;
                }

                // LED 제어 요청 처리
                if (currentLine.endsWith("GET /RH"))
                {
                    digitalWrite(LEDR, HIGH);
                    digitalWrite(LEDG, LOW);
                    digitalWrite(LEDB, LOW);
                    myLEDstripSUB.neoPixelFill(255, 0, 0, true); // 빨간색 켜기
                }
                else if (currentLine.endsWith("GET /GH"))
                {
                    digitalWrite(LEDR, LOW);
                    digitalWrite(LEDG, HIGH);
                    digitalWrite(LEDB, LOW);
                    myLEDstripSUB.neoPixelFill(0, 255, 0, true); // 초록색 켜기
                }
                else if (currentLine.endsWith("GET /BH"))
                {
                    digitalWrite(LEDR, LOW);
                    digitalWrite(LEDG, LOW);
                    digitalWrite(LEDB, HIGH);
                    myLEDstripSUB.neoPixelFill(0, 0, 255, true); // 파란색 켜기
                }
                else if (currentLine.endsWith("GET /WH"))
                {
                    digitalWrite(LEDR, HIGH);
                    digitalWrite(LEDG, HIGH);
                    digitalWrite(LEDB, HIGH);
                    myLEDstripSUB.neoPixelFill(255, 255, 255, true); // 흰색 켜기
                }
                else if (currentLine.endsWith("GET /RL") || currentLine.endsWith("GET /GL") || currentLine.endsWith("GET /BL") || currentLine.endsWith("GET /WL"))
                {
                    digitalWrite(LEDR, LOW);
                    digitalWrite(LEDG, LOW);
                    digitalWrite(LEDB, LOW);
                    myLEDstripSUB.neoPixelFill(0, 0, 0, true); // 모든 LED 끄기
                }
            }
        }
        client.stop();
        Serial.println("client disconnected");
    }
}

// Wi-Fi 상태 정보를 출력하는 함수
// Wi-Fi 상태 정보를 보기 좋게 출력하는 함수
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
