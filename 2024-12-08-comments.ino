#define REMOTEXY_MODE__ESP32CORE_BLE

#include <BLEDevice.h>
#include <RemoteXY.h>

#define REMOTEXY_BLUETOOTH_NAME "SoccerBot"
#define REMOTEXY_ACCESS_PASSWORD "test"

#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = { 
  255, 2, 0, 0, 0, 29, 0, 19, 0, 0, 0, 0, 186, 1, 106, 200, 1, 1, 2, 0,
  4, 21, 69, 18, 63, 32, 31, 180, 4, 67, 69, 19, 64, 32, 31, 166
};

struct {
  int8_t slider_left;
  int8_t slider_right;
  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)

// Einrichtung der Pins
#define SENSOR_BLACK A0
#define MOTOR_LEFT_FW 3
#define MOTOR_LEFT_BW 2
#define MOTOR_RIGHT_FW 19
#define MOTOR_RIGHT_BW 18

void setup() {
  RemoteXY_Init();
  Serial.begin(9600);

  pinMode(MOTOR_LEFT_FW, OUTPUT);
  pinMode(MOTOR_LEFT_BW, OUTPUT);
  pinMode(MOTOR_RIGHT_FW, OUTPUT);
  pinMode(MOTOR_RIGHT_BW, OUTPUT);
  pinMode(SENSOR_BLACK, INPUT);

  Serial.println("Bereit!");
}

int mapSliderToSpeed(int sliderValue) {
  // Zuordnung von Slider-Wert -> Motor-Gewschindigkeit
  if (sliderValue > 10) {
    return map(sliderValue, 10, 100, 160, 255);
  } else if (sliderValue < -10) {
    return map(sliderValue, -10, -100, -160, -255);
  } else {
    return 0;
  }
}

void controlMotor(int motor_fw, int motor_bw, int speed) {
  // Steuerung der Motoren
  if (speed > 0) {
    analogWrite(motor_fw, speed);
    analogWrite(motor_bw, 0);
  } else if (speed < 0) {
    analogWrite(motor_fw, 0);
    analogWrite(motor_bw, -speed);
  } else {
    analogWrite(motor_fw, 0);
    analogWrite(motor_bw, 0);
  }
}

void forcefulBackup() {
  // Wird bei Überschreiten der Barriere ausgeführt
  Serial.println("Barriere erkannt! Kehre um....");

  analogWrite(MOTOR_LEFT_FW, 0);
  analogWrite(MOTOR_LEFT_BW, 255);
  analogWrite(MOTOR_RIGHT_FW, 0);
  analogWrite(MOTOR_RIGHT_BW, 255);

  delay(500);

  Serial.println("Umkehr...");
  
  analogWrite(MOTOR_LEFT_FW, 255);
  analogWrite(MOTOR_LEFT_BW, 0);
  analogWrite(MOTOR_RIGHT_FW, 0);
  analogWrite(MOTOR_RIGHT_BW, 255);

  delay(500);

  analogWrite(MOTOR_LEFT_FW, 0);
  analogWrite(MOTOR_LEFT_BW, 0);
  analogWrite(MOTOR_RIGHT_FW, 0);
  analogWrite(MOTOR_RIGHT_BW, 0);   

  Serial.println("Umkehr abgeschlossen.");
}

void loop() { 
  RemoteXY_Handler();

  bool isBlack = analogRead(SENSOR_BLACK) > 4000;
  Serial.print("Sensor-Wert: ");
  Serial.println(analogRead(SENSOR_BLACK));
  Serial.print("isBlack: ");
  Serial.println(isBlack);

  if (isBlack) {
    forcefulBackup();
  } else {
    // Slider-Werte aus der App auslesen und tatsächlichen Werte berechnen
    int leftSpeed = mapSliderToSpeed(RemoteXY.slider_left);
    int rightSpeed = mapSliderToSpeed(RemoteXY.slider_right);

    Serial.print("Left speed: ");
    Serial.print(leftSpeed);
    Serial.print(" | Right speed: ");
    Serial.println(rightSpeed);

    // "normale" Steuerung
    controlMotor(MOTOR_LEFT_FW, MOTOR_LEFT_BW, leftSpeed);
    controlMotor(MOTOR_RIGHT_FW, MOTOR_RIGHT_BW, rightSpeed);
  }

  delay(200);
}
