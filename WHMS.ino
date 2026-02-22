#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include "MPU6050.h"
#include <math.h>

//////////////////// PIN DEFINITIONS ////////////////////

#define BUZZER 25
#define CALIB_BTN 32
#define SOS_BTN 33
#define CANCEL_BTN 18
#define BATTERY_ADC 34

#define GSM_RX 16
#define GSM_TX 17

#define GPS_RX 26
#define GPS_TX 27

#define PHONE_NUMBER "+918971753636"

//////////////////// OLED SETTINGS ////////////////////

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//////////////////// OBJECTS ////////////////////

TinyGPSPlus gps;
MPU6050 mpu;

HardwareSerial gsm(2);
HardwareSerial gpsSerial(1);

//////////////////// VARIABLES ////////////////////

unsigned long currentMillis;

bool calibrationMode = false;
bool falseTriggerMode = false;
bool sosMode = false;

bool lastSOSState = HIGH;

unsigned long calibPressTime = 0;
unsigned long buzzerStart = 0;
unsigned long falseStart = 0;

int16_t ax, ay, az;
float offsetX = 0, offsetY = 0, offsetZ = 0;

//////////////////// BUZZER CONTROL ////////////////////

void startBuzzer() {
  buzzerStart = millis();
  digitalWrite(BUZZER, HIGH);
}

void updateBuzzer(unsigned long duration) {
  if (digitalRead(BUZZER) == HIGH) {
    if (millis() - buzzerStart >= duration) {
      digitalWrite(BUZZER, LOW);
    }
  }
}

//////////////////// BATTERY FUNCTION ////////////////////

float getBatteryVoltage() {
  int raw = analogRead(BATTERY_ADC);
  float voltage = (raw / 4095.0) * 3.3;
  voltage = voltage * 2.0;  
  return voltage;
}

int batteryPercent(float voltage) {
  if (voltage >= 4.2) return 100;
  if (voltage <= 3.0) return 0;
  return (voltage - 3.0) * 100 / (4.2 - 3.0);
}

//////////////////// GSM FUNCTIONS ////////////////////

void sendSMS(String message) {
  gsm.println("AT+CMGF=1");
  delay(500);
  gsm.print("AT+CMGS=\"");
  gsm.print(PHONE_NUMBER);
  gsm.println("\"");
  delay(500);
  gsm.print(message);
  delay(200);
  gsm.write(26);
  delay(3000);
}

void makeCall() {
  gsm.print("ATD");
  gsm.print(PHONE_NUMBER);
  gsm.println(";");
  delay(10000);
  gsm.println("ATH");
}

//////////////////// CALIBRATION ////////////////////

void calibrateMPU() {
  long sumX = 0, sumY = 0, sumZ = 0;

  for (int i = 0; i < 200; i++) {
    mpu.getAcceleration(&ax, &ay, &az);
    sumX += ax;
    sumY += ay;
    sumZ += az;
    delay(5);
  }

  offsetX = sumX / 200.0;
  offsetY = sumY / 200.0;
  offsetZ = sumZ / 200.0;
}

//////////////////// SETUP ////////////////////

void setup() {

  Serial.begin(115200);

  Wire.begin(21, 22);   // ESP32 I2C pins

  pinMode(BUZZER, OUTPUT);
  pinMode(CALIB_BTN, INPUT_PULLUP);
  pinMode(SOS_BTN, INPUT_PULLUP);
  pinMode(CANCEL_BTN, INPUT_PULLUP);

  analogReadResolution(12);

  gsm.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  mpu.initialize();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED allocation failed");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

//////////////////// LOOP ////////////////////

void loop() {

  currentMillis = millis();

  ////////////////// GPS UPDATE //////////////////
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  ////////////////// BATTERY //////////////////
  float battVolt = getBatteryVoltage();
  int battPercent = batteryPercent(battVolt);

  ////////////////// OLED DISPLAY //////////////////
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print("Battery: ");
  display.print(battPercent);
  display.println("%");

  display.print("Sat: ");
  display.println(gps.satellites.value());

  if (gps.location.isValid())
    display.println("GPS: FIXED");
  else
    display.println("GPS: Searching...");

  if (calibrationMode)
    display.println("Calibrating...");
  else if (falseTriggerMode)
    display.println("False Trigger!");
  else if (sosMode)
    display.println("SOS Sent");
  else
    display.println("System Active");

  display.display();

  ////////////////// CALIBRATION //////////////////

  if (digitalRead(CALIB_BTN) == LOW) {
    if (calibPressTime == 0)
      calibPressTime = currentMillis;

    if (currentMillis - calibPressTime >= 3000 && !calibrationMode) {
      calibrationMode = true;
      startBuzzer();
      calibrateMPU();
    }
  } else {
    calibPressTime = 0;
  }

  if (calibrationMode) {
    updateBuzzer(3000);
    if (millis() - buzzerStart >= 3000)
      calibrationMode = false;
  }

  ////////////////// FALL DETECTION //////////////////

  mpu.getAcceleration(&ax, &ay, &az);

  float magnitude = sqrt(
    sq((ax - offsetX) / 16384.0) +
    sq((ay - offsetY) / 16384.0) +
    sq((az - offsetZ) / 16384.0)
  );

  if (magnitude > 2.5 && !falseTriggerMode) {
    falseTriggerMode = true;
    falseStart = millis();
    startBuzzer();
  }

  ////////////////// FALSE TRIGGER //////////////////

  if (falseTriggerMode) {

    updateBuzzer(10000);

    if (digitalRead(CANCEL_BTN) == LOW) {
      falseTriggerMode = false;
      digitalWrite(BUZZER, LOW);
    }

    if (millis() - falseStart >= 10000) {

      falseTriggerMode = false;
      sosMode = true;

      String location = "Location not fixed";

      if (gps.location.isValid()) {
        location = "Lat:";
        location += String(gps.location.lat(), 6);
        location += " Lon:";
        location += String(gps.location.lng(), 6);
      }

      sendSMS("Fall Detected!\n" + location);
      makeCall();
    }
  }

  ////////////////// SOS BUTTON (UPDATED) //////////////////

  bool currentSOSState = digitalRead(SOS_BTN);

  if (lastSOSState == HIGH && currentSOSState == LOW) {

    sosMode = true;

    startBuzzer();   // 3 sec beep

    String location = "Location not fixed";

    if (gps.location.isValid()) {
      location = "Lat:";
      location += String(gps.location.lat(), 6);
      location += " Lon:";
      location += String(gps.location.lng(), 6);
    }

    sendSMS("SOS Pressed!\n" + location);
    makeCall();
  }

  if (sosMode) {
    updateBuzzer(3000);

    if (millis() - buzzerStart > 5000)
      sosMode = false;
  }

  lastSOSState = currentSOSState;

  delay(5);
}