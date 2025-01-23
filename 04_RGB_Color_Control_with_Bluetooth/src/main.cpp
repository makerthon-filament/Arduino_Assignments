#include <ArduinoBLE.h>
#include <NeoPixelConnect.h>

// ==================== 상수 및 객체 정의 ====================
#define DEBUG_MODE_ENABLE 1  // 디버그 모드 활성화
#define NUM_OF_PIXEL 12      // 네오픽셀 LED 수
#define PIXEL_CONTROL_PIN 15 // 네오픽셀 데이터 핀

// BLE 서비스 및 특성 설정
BLEService myService("71c46861-691a-4b1e-9ddb-d722fa9ad632");
BLECharacteristic myCharacteristic("71c46861-691a-4b1e-9ddb-d722fa9ad632",
                                   BLERead | BLEWrite | BLEWriteWithoutResponse | BLENotify, 16);

// NeoPixel 객체 생성
NeoPixelConnect myLEDstrip(PIXEL_CONTROL_PIN, NUM_OF_PIXEL, pio1, 0);

// ==================== 전역 변수 선언 ====================
String strMsgPacket;
unsigned char cmdMode, redColor, greenColor, blueColor;
bool flgModeColorControl, flgModeEffectA, flgModeEffectB;
unsigned long pixelsInterval = 250;      // LED 효과 간격 (ms)
unsigned long rainbowPreviousMillis = 0; // 레인보우 효과 이전 시간
int rainbowCycles = 0;                   // 레인보우 사이클

// ==================== 함수 프로토타입 ====================
void bleConnectHandler(BLEDevice central);                                            // BLE 연결 이벤트 핸들러
void bleDisconnectHandler(BLEDevice central);                                         // BLE 연결 해제 이벤트 핸들러
void writtenEventCharacteristic(BLEDevice central, BLECharacteristic characteristic); // BLE 데이터 수신 핸들러

void myPacketParseHandler(String rawPacketData);                            // 데이터 파싱 핸들러
void appLedStripControlHandler(void);                                       // LED 컨트롤 핸들러
void myRainbowEffect(void);                                                 // 레인보우 효과 함수
uint32_t Wheel(byte WheelPos, uint8_t *red, uint8_t *green, uint8_t *blue); // 색상 휠 변환 함수

// ==================== setup() ====================
void setup()
{
#if (DEBUG_MODE_ENABLE)
  Serial.begin(9600); // 디버깅 시리얼 모니터 시작
#endif

  // BLE 초기화
  if (!BLE.begin())
  {
#if (DEBUG_MODE_ENABLE)
    Serial.println("starting BLE failed!");
#endif
    while (1)
      ; // BLE 시작 실패 시 무한 루프
  }

#if (DEBUG_MODE_ENABLE)
  Serial.println("starting BLE success!");
#endif

  // BLE 설정
  BLE.setLocalName("myNanoRP2040");
  BLE.setDeviceName("myNanoRP2040");
  BLE.setAdvertisedService(myService);
  myService.addCharacteristic(myCharacteristic);
  BLE.addService(myService);

  // BLE 이벤트 핸들러 설정
  BLE.setEventHandler(BLEConnected, bleConnectHandler);
  BLE.setEventHandler(BLEDisconnected, bleDisconnectHandler);
  myCharacteristic.setEventHandler(BLEWritten, writtenEventCharacteristic);

  // BLE 브로드캐스팅 시작
  BLE.advertise();

#if (DEBUG_MODE_ENABLE)
  Serial.println("Bluetooth device active, waiting for Central Device Connections...");
#endif

  // LED 초기값 설정
  redColor = 100;
  greenColor = 100;
  blueColor = 100;
  myLEDstrip.neoPixelFill(redColor / 2, greenColor / 2, blueColor / 2, true);
}

// ==================== loop() ====================
void loop()
{
  appLedStripControlHandler(); // LED 스트립 제어
  BLE.poll();                  // BLE 이벤트 수신 대기
}

// ==================== BLE 연결 이벤트 핸들러 ====================
void bleConnectHandler(BLEDevice central)
{
#if (DEBUG_MODE_ENABLE)
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
#endif
  redColor = 0;
  greenColor = 0;
  blueColor = 200;
  myLEDstrip.neoPixelFill(redColor, greenColor, blueColor, true);
}

// ==================== BLE 연결 해제 이벤트 핸들러 ====================
void bleDisconnectHandler(BLEDevice central)
{
#if (DEBUG_MODE_ENABLE)
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
#endif
  redColor = 0;
  greenColor = 0;
  blueColor = 0;
  myLEDstrip.neoPixelFill(redColor, greenColor, blueColor, true);
}

// ==================== BLE 데이터 수신 핸들러 ====================
void writtenEventCharacteristic(BLEDevice central, BLECharacteristic characteristic)
{
  uint8_t value[16];
  int length = myCharacteristic.valueLength();
  myCharacteristic.readValue(value, length);

  strMsgPacket = "";
  for (unsigned int i = 0; i < static_cast<unsigned int>(length); i++)
  {
    strMsgPacket += (char)value[i];
  }

#if (DEBUG_MODE_ENABLE)
  Serial.print("Length: ");
  Serial.print(length);
  Serial.print(" Data: ");
  Serial.println(strMsgPacket);
#endif

  myPacketParseHandler(strMsgPacket); // 데이터 파싱 핸들러 호출
}

// ==================== 데이터 파싱 핸들러 ====================
void myPacketParseHandler(String rawPacketData)
{
  cmdMode = rawPacketData.substring(0, 2).toInt();
  redColor = rawPacketData.substring(2, 5).toInt();
  greenColor = rawPacketData.substring(5, 8).toInt();
  blueColor = rawPacketData.substring(8, 11).toInt();

#if (DEBUG_MODE_ENABLE)
  Serial.print("Mode Value: ");
  Serial.println(cmdMode);
  Serial.print("Red Value: ");
  Serial.println(redColor);
  Serial.print("Green Value: ");
  Serial.println(greenColor);
  Serial.print("Blue Value: ");
  Serial.println(blueColor);
#endif

  // 모드에 따른 플래그 설정
  flgModeColorControl = (cmdMode == 2);
  flgModeEffectA = (cmdMode == 4);
  flgModeEffectB = (cmdMode == 6);
}

// ==================== LED 스트립 제어 함수 ====================
void appLedStripControlHandler(void)
{
  if (flgModeColorControl)
  {
    myLEDstrip.neoPixelFill(redColor, greenColor, blueColor, true);
    flgModeColorControl = false;
  }
  else if (flgModeEffectA)
  {
    if ((unsigned long)(millis() - rainbowPreviousMillis) >= pixelsInterval)
    {
      rainbowPreviousMillis = millis();
      myRainbowEffect();
    }
  }
  else if (flgModeEffectB)
  {
    redColor = 150;
    greenColor = 150;
    blueColor = 150;
    myLEDstrip.neoPixelFill(redColor, greenColor, blueColor, true);
  }
}

// ==================== 레인보우 이펙트 ====================
void myRainbowEffect()
{
  uint8_t colors[NUM_OF_PIXEL][3];

  for (uint16_t i = 0; i < NUM_OF_PIXEL; i++)
  {
    Wheel((i + rainbowCycles) & 255, &colors[i][0], &colors[i][1], &colors[i][2]);
  }

  myLEDstrip.neoPixelFill(colors[0][0], colors[0][1], colors[0][2], true);
  rainbowCycles = (rainbowCycles + 1) % 256;
}

// ==================== 색상 휠 변환 함수 ====================
uint32_t Wheel(byte WheelPos, uint8_t *red, uint8_t *green, uint8_t *blue)
{
  // 범위 초과 체크 제거
  WheelPos = 255 - WheelPos;

  if (WheelPos < 85)
  {
    *red = 255 - WheelPos * 3;
    *green = 0;
    *blue = WheelPos * 3;
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    *red = 0;
    *green = WheelPos * 3;
    *blue = 255 - WheelPos * 3;
  }
  else
  {
    WheelPos -= 170;
    *red = WheelPos * 3;
    *green = 255 - WheelPos * 3;
    *blue = 0;
  }
  return 1;
}
