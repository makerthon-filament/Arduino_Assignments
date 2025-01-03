#include <SPI.h>
#include <WiFiNINA.h>

// Wi-Fi 네트워크 정보
char ssid[] = "";            // Wi-Fi 네트워크 이름
char pass[] = "";            // Wi-Fi 비밀번호
int status = WL_IDLE_STATUS; // Wi-Fi 상태
WiFiServer server(80);       // HTTP 서버 포트 80

void setup()
{
    // 내장 LED 핀 설정
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);

    // 시리얼 통신 초기화
    Serial.begin(9600);

    // Wi-Fi 모듈 확인
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Wi-Fi 모듈을 찾을 수 없습니다!");
        while (true)
            ;
    }

    // Wi-Fi 펌웨어 버전 확인
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    {
        Serial.println("Wi-Fi 펌웨어를 업데이트하세요!");
    }

    // Wi-Fi 네트워크에 연결
    while (status != WL_CONNECTED)
    {
        Serial.print("네트워크 연결 시도 중: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass); // Wi-Fi 연결
        delay(10000);
    }

    // 서버 시작
    server.begin();
    printWifiStatus(); // 연결 상태 출력
}

void loop()
{
    // 클라이언트 대기
    WiFiClient client = server.available();

    if (client)
    { // 새로운 클라이언트 접속
        Serial.println("새 클라이언트가 연결되었습니다.");
        String currentLine = "";

        while (client.connected())
        { // 클라이언트와 연결 유지
            if (client.available())
            { // 클라이언트에서 데이터 수신
                char c = client.read();
                Serial.write(c); // 수신 데이터를 시리얼 모니터에 출력

                if (c == '\n')
                { // 요청의 끝인지 확인
                    if (currentLine.length() == 0)
                    {
                        // HTTP 헤더 전송
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        // HTML 및 CSS 전송
                        client.print("<style>");
                        client.print(".container {margin: 0 auto; text-align: center; margin-top: 50px;}");
                        client.print("button {padding: 15px 30px; margin: 5px; font-size: 18px; cursor: pointer;}");
                        client.print(".red {background: red; color: white;}");
                        client.print(".green {background: green; color: white;}");
                        client.print(".blue {background: blue; color: white;}");
                        client.print(".off {background: gray; color: white;}");
                        client.print("</style>");
                        client.print("<div class='container'>");
                        client.print("<button class='red' onmousedown='location.href=\"/RH\"'>Red ON</button>");
                        client.print("<button class='off' onmousedown='location.href=\"/RL\"'>Red OFF</button>");
                        client.print("<br>");
                        client.print("<button class='green' onmousedown='location.href=\"/GH\"'>Green ON</button>");
                        client.print("<button class='off' onmousedown='location.href=\"/GL\"'>Green OFF</button>");
                        client.print("<br>");
                        client.print("<button class='blue' onmousedown='location.href=\"/BH\"'>Blue ON</button>");
                        client.print("<button class='off' onmousedown='location.href=\"/BL\"'>Blue OFF</button>");
                        client.print("</div>");

                        client.println(); // HTTP 응답 종료
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

                // 요청 처리: URL 끝점 확인
                if (currentLine.endsWith("GET /RH"))
                    digitalWrite(LEDR, HIGH); // Red ON
                if (currentLine.endsWith("GET /RL"))
                    digitalWrite(LEDR, LOW); // Red OFF
                if (currentLine.endsWith("GET /GH"))
                    digitalWrite(LEDG, HIGH); // Green ON
                if (currentLine.endsWith("GET /GL"))
                    digitalWrite(LEDG, LOW); // Green OFF
                if (currentLine.endsWith("GET /BH"))
                    digitalWrite(LEDB, HIGH); // Blue ON
                if (currentLine.endsWith("GET /BL"))
                    digitalWrite(LEDB, LOW); // Blue OFF
            }
        }

        // 클라이언트 연결 종료
        client.stop();
        Serial.println("클라이언트 연결 종료");
    }
}

void printWifiStatus()
{
    // 네트워크 이름 출력
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // IP 주소 출력
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // 신호 강도 출력
    long rssi = WiFi.RSSI();
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");

    // 브라우저 접속 정보 출력
    Serial.print("브라우저에서 접속하세요: http://");
    Serial.println(ip);
}
