#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "spo2_algorithm.h"
#include "MAX30105.h"

/*
  OLED:
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = D21
  -SCL = D22
  MAX30102
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = D21 (or SDA)
  -SCL = AD22 (or SCL)
  -INT = Not connected
  Vibration motor:
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -INT = D18
*/

// Khởi tạo màn mình OLED
#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Khai báo chân kết nối cảm biến rung
#define VM_PIN 18

// Thiết lập thông số cảm biến SpO2
byte ledBrightness = 60;  //Options: 0=Off to 255=50mA
byte sampleAverage = 4;   //Options: 1, 2, 4, 8, 16, 32
byte ledMode = 2;         //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
byte sampleRate = 100;    //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
int pulseWidth = 411;     //Options: 69, 118, 215, 411
int adcRange = 4096;      //Options: 2048, 4096, 8192, 16384
long unblockedValue;      // Giá trị TB của IR khi powerup
MAX30105 particleSensor;

// Biến lưu trữ dữ liệu
#define IR_THRESHOLD 200000
uint32_t irBuffer[100];      //mảng giá trị LED hồng ngoại đo được
uint32_t redBuffer[100];     //mảng giá trị LED đỏ đo được
int32_t bufferLength = 100;  // độ dài mảng lưu trữ
int32_t dataReadLength = 25;
int32_t spo2;           // Giá trị SpO2
int8_t validSPO2;       // kiểm tra spO2 có hợp lệ hay không
int32_t heartRate;      // Giá trị heart rate
int8_t validHeartRate;  //kiểm tra heart rate có hợp lệ hay không
long cntHeartRate = 0;
long totalHeartRate = 0;

void oledPrint(char *str) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(str);
  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(VM_PIN, OUTPUT);

  // Khởi tạo OLED
  delay(1000);
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  // Kiểm tra kết nối MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))  //Use default I2C port, 400kHz speed
  {
    display.setCursor(0, 0);
    display.println("MAX30102 was not found. Please check wiring/power.");
    display.display();
    while (1)
      ;
  }
  // Cấu hình MAX30102
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  // Power up
  oledPrint("Powering up !!!");
  // Đọc 100 mẫu dữ liệu đầu tiên (bufferLengh = 100)
  for (byte i = 0; i < bufferLength; i++) {
    // Kiem tra sensor
    while (particleSensor.available() == false)
      particleSensor.check();

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }
}

void loop() {
  if (particleSensor.getIR() >= IR_THRESHOLD) {
    // Xóa n giá trị đầu tiên (dataReadLengh = n)
    for (byte i = dataReadLength; i < bufferLength; i++) {
      redBuffer[i - dataReadLength] = redBuffer[i];
      irBuffer[i - dataReadLength] = irBuffer[i];
    }

    // Đọc n giá trị tiếp theo vào cuối mảng (dataReadLength = n)
    for (byte i = bufferLength - dataReadLength; i < bufferLength; i++) {
      // Kiem tra sensor
      while (particleSensor.available() == false)
        particleSensor.check();

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();  

      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);
    }
    // Tính toán heart rate và spO2
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);


    if (validSPO2 && validHeartRate) {
      cntHeartRate++;
      totalHeartRate += heartRate;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Heart rate: ");
      display.print(heartRate);
      display.println(" BPM");
      display.println();
      display.print("SpO2: ");
      display.print(spo2);
      display.println("%");

    // Kiem tra spO2 va heart rate cao hay thap
      bool isNormalHeartRate, isNormalSpO2;
      float avgHeartRate = (float)totalHeartRate / cntHeartRate;
      if (avgHeartRate < 80) {
        if (heartRate < 45 || heartRate > 105) {
          display.println();
          if (heartRate < 45) {
            display.println("Low heart rate");
          } else {
            display.println("High heart rate");
          }
          isNormalHeartRate = false;
        } else
          isNormalHeartRate = true;
      } else {
        if (heartRate < 50 || heartRate > 110) {
          display.println();
          if (heartRate < 50) {
            display.println("Low heart rate");
          } else {
            display.println("High heart rate");
          }
          isNormalHeartRate = false;
        } else {
          isNormalHeartRate = true;
        }
      }

      isNormalSpO2 = spo2 >= 94;
      if (!isNormalSpO2) {
        display.println();
        display.println("Low SPO2");
      }

      if (!isNormalHeartRate || !isNormalSpO2) {
        digitalWrite(VM_PIN, HIGH);
      } else {
        digitalWrite(VM_PIN, LOW);
      }

      display.display();
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Heart rate: ");
      display.println("0 BPM");
      display.println();
      display.print("SpO2: ");
      display.println("---%");
      display.display();
      digitalWrite(VM_PIN, LOW);
    }
  } else {
    cntHeartRate = 0;
    totalHeartRate = 0;
    digitalWrite(VM_PIN, LOW);
    oledPrint("Put your finger!");
  }
}
