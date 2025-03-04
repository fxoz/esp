// NOTE: Incorrect formatting since the following code was copied from a PDF file!
// Credits: Felix Orosz, David Stargala

#define REMOTEXY_MODE__ESP32CORE_BLE
#include <ESP32Servo.h>
#include <BLEDevice.h>
#include <RemoteXY.h>
#define REMOTEXY_BLUETOOTH_NAME "ESP-08-D"
#define REMOTEXY_ACCESS_PASSWORD "Roboter"
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = // 90 bytes
    {255, 5, 0, 0, 0, 83, 0, 19, 0, 0, 0, 0, 186, 1, 106, 200, 1, 1, 5, 0,
     4, 12, 69, 22, 77, 48, 16, 178, 1, 41, 26, 24, 24, 0, 178, 31, 240, 159, 154, 128,
     0, 4, 34, 97, 77, 22, 176, 16, 178, 2, 1, 1, 39, 9, 1, 178, 26, 16, 31, 83,
     101, 110, 115, 111, 114, 0, 79, 70, 70, 0, 2, 66, 1, 39, 9, 1, 178, 26, 16, 31,
     98, 111, 111, 115, 116, 0, 97, 105, 109, 0};
struct
{
    int8_t slider_left;    // -100 bis 100
    uint8_t button_shoot;  // 1 beim Drücken, sonst 0
    int8_t slider_right;   // -100 bis 100
    uint8_t switch_sensor; // 1 aktiviert, 0 deaktiviert
    uint8_t switch_speed;  // 1 aktiviert, 0 deaktiviert
    uint8_t connect_flag;  // 1 bei Verbindung, sonst 0
} RemoteXY;
#pragma pack(pop)
// Einrichtung der Pins
#define SENSOR_BLACK_FW_PIN A0
#define SENSOR_BLACK_BW_PIN A1
#define MOTOR_LEFT_FW_PIN 3
#define MOTOR_LEFT_BW_PIN 20
#define MOTOR_RIGHT_FW_PIN 5
#define MOTOR_RIGHT_BW_PIN 4
#define SERVO_PIN 10
#include "PWM.h"
Servo servo;
bool shootButtonPrevState = false;
void setup()
{
    RemoteXY_Init();
    PWM_init();
    servo.setPeriodHertz(50);
    servo.attach(SERVO_PIN);
    Serial.begin(9600);
    pinMode(MOTOR_LEFT_FW_PIN, OUTPUT);
    pinMode(MOTOR_LEFT_BW_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_FW_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_BW_PIN, OUTPUT);
    pinMode(SENSOR_BLACK_FW_PIN, INPUT);
    pinMode(SENSOR_BLACK_BW_PIN, INPUT);
    // Stoppe die Motoren
    analogWrite(MOTOR_LEFT_FW_PIN, 0);
    analogWrite(MOTOR_LEFT_BW_PIN, 0);
    analogWrite(MOTOR_RIGHT_FW_PIN, 0);
    analogWrite(MOTOR_RIGHT_BW_PIN, 0);
    Serial.println("Bereit!");
}
int mapSliderToSpeed(int sliderValue, int speedMode)
{
    // Zuordnung von Slider-Wert -> Motor-Gewschindigkeit
    if (speedMode == 1)
    {
        if (sliderValue > 5)
        {
            return map(sliderValue, 5, 100, 170, 255);
        }
        else if (sliderValue < -5)
        {
            return map(sliderValue, -5, -100, -170, -255);
        }
    }
    else
    {
        if (sliderValue > 5)
        {
            return 170;
        }
        else if (sliderValue < -5)
        {
            return -170;
        }
    }
    return 0;
}
void stopAllMotors()
{
    // Alle Motoren werden gestoppt.
    PWM_analogWrite(MOTOR_LEFT_FW_PIN, 0);
    PWM_analogWrite(MOTOR_LEFT_BW_PIN, 0);
    PWM_analogWrite(MOTOR_RIGHT_FW_PIN, 0);
    PWM_analogWrite(MOTOR_RIGHT_BW_PIN, 0);
}
void RcontrolMotor(int motorForwardsPin, int motorBackwardsPin, int speedSlider, int directionSlider)
{
    // Steuerung des rechten Motors, da sie sich beim Abbiegen unterschiedlich drehen.
    int speedSliderFW, speedSliderBW, directionSliderFW, directionSliderBW;
    speedSlider = constrain(speedSlider, -255, 255);
    directionSlider = constrain(directionSlider, -255, 255);
    if (speedSlider > 0)
    {
        speedSliderFW = speedSlider;
        speedSliderBW = 0;
    }
    else
    {
        speedSliderFW = 0;
        speedSliderBW = abs(speedSlider);
    }
    if (directionSlider > 0)
    {
        directionSliderFW = directionSlider;
        directionSliderBW = 0;
    }
    else
    {
        directionSliderFW = 0;
        directionSliderBW = abs(directionSlider);
    }
    if (speedSlider == 0 && directionSlider == 0)
    {
        PWM_analogWrite(motorForwardsPin, 0);
        PWM_analogWrite(motorBackwardsPin, 0);
        return;
    }
    if (speedSlider != 0)
    {
        if (directionSlider > 0)
        {
            PWM_analogWrite(motorForwardsPin, (100.0 - abs(RemoteXY.slider_right)) / 100.0 *
                                                  speedSliderFW);
            PWM_analogWrite(motorBackwardsPin, (100.0 - abs(RemoteXY.slider_right)) / 100.0 *
                                                   speedSliderBW); // new
            return;
        }
        PWM_analogWrite(motorForwardsPin, speedSliderFW);
        PWM_analogWrite(motorBackwardsPin, speedSliderBW);
        return;
    }
    else if (directionSlider != 0)
    {
        PWM_analogWrite(motorForwardsPin, directionSliderBW);
        PWM_analogWrite(motorBackwardsPin, directionSliderFW);
    }
}
void LcontrolMotor(int motorForwardsPin, int motorBackwardsPin, int speedSlider, int directionSlider)
{
    // Steuerung des linken Motors, da sie sich beim Abbiegen unterschiedlich drehen.
    int speedSliderFW, speedSliderBW, directionSliderFW, directionSliderBW;
    speedSlider = constrain(speedSlider, -255, 255);
    directionSlider = constrain(directionSlider, -255, 255);
    if (speedSlider > 0)
    {
        speedSliderFW = speedSlider;
        speedSliderBW = 0;
    }
    else
    {
        speedSliderFW = 0;
        speedSliderBW = abs(speedSlider);
    }
    if (directionSlider > 0)
    {
        directionSliderFW = directionSlider;
        directionSliderBW = 0;
    }
    else
    {
        directionSliderFW = 0;
        directionSliderBW = abs(directionSlider);
    }
    if (speedSlider == 0 && directionSlider == 0)
    {
        PWM_analogWrite(motorForwardsPin, 0);
        PWM_analogWrite(motorBackwardsPin, 0);
        return;
    }
    if (speedSlider != 0)
    {
        if (directionSlider > 0)
        {
            PWM_analogWrite(motorForwardsPin, speedSliderFW);
            PWM_analogWrite(motorBackwardsPin, speedSliderBW);
            return;
        }
        PWM_analogWrite(motorForwardsPin, (100.0 - abs(RemoteXY.slider_right)) / 100.0 *
                                              speedSliderFW);
        PWM_analogWrite(motorBackwardsPin, (100.0 - abs(RemoteXY.slider_right)) / 100.0 *
                                               speedSliderBW);
        return;
    }
    else if (directionSlider != 0)
    {
        PWM_analogWrite(motorForwardsPin, directionSliderFW);
        PWM_analogWrite(motorBackwardsPin, directionSliderBW);
    }
}
void forcefulBackup(int isBW)
{
    // Wird bei Überschreiten der Barriere ausgeführt, je nach Sensor (FW = vorne, BW =
hinten) muss das RoboCar in unterschiedliche Richtungen ausweichen.
 Serial.println("Barriere erkannt! Kehre um....");
int isFW = isBW ? 0 : 1;

PWM_analogWrite(MOTOR_LEFT_FW_PIN, 255 * isBW);
PWM_analogWrite(MOTOR_LEFT_BW_PIN, 255 * isFW);
PWM_analogWrite(MOTOR_RIGHT_FW_PIN, 255 * isBW);
PWM_analogWrite(MOTOR_RIGHT_BW_PIN, 255 * isFW);
delay(500);
stopAllMotors();
Serial.println("Umkehr abgeschlossen.");
}
void shootBall()
{
    // Automatisierter Prozess vom Wegschießen des Balles in die jetzige Richtung.
    int servoMinPos = 0;
    int servoStartPos = 20;
    int servoMaxPos = 90;
    stopAllMotors();
    servo.write(servoStartPos);
    PWM_analogWrite(MOTOR_LEFT_FW_PIN, 0);
    PWM_analogWrite(MOTOR_LEFT_BW_PIN, 255);
    PWM_analogWrite(MOTOR_RIGHT_FW_PIN, 0);
    PWM_analogWrite(MOTOR_RIGHT_BW_PIN, 255);
    delay(500);
    PWM_analogWrite(MOTOR_LEFT_FW_PIN, 255);
    PWM_analogWrite(MOTOR_LEFT_BW_PIN, 0);
    PWM_analogWrite(MOTOR_RIGHT_FW_PIN, 255);
    PWM_analogWrite(MOTOR_RIGHT_BW_PIN, 0);
    delay(300);
    servo.write(servoMaxPos);
    delay(200);
    stopAllMotors();
    delay(200);
    servo.write(servoMinPos);
    delay(200);
    servo.write(servoStartPos);
}
void loop()
{
    RemoteXY_Handler();
    bool isBlackFW = analogRead(SENSOR_BLACK_FW_PIN) > 3000;
    bool isBlackBW = analogRead(SENSOR_BLACK_BW_PIN) > 3000;
    if (RemoteXY.button_shoot == 1 && !shootButtonPrevState)
    {
        shootBall();
    }
    shootButtonPrevState = RemoteXY.button_shoot;
    int enableBlackSensor = RemoteXY.switch_sensor;
    int enableBoostMode = RemoteXY.switch_speed;
    if (isBlackFW && enableBlackSensor == 1 && (RemoteXY.slider_left != 0 || RemoteXY.slider_right != 0))
    {
        forcefulBackup(0);
    }
    if (isBlackBW && enableBlackSensor == 1 && (RemoteXY.slider_left != 0 || RemoteXY.slider_right != 0))
    {
        forcefulBackup(1);
    }
    int speedSlider = mapSliderToSpeed(RemoteXY.slider_left, enableBoostMode);
    int directionSlider = mapSliderToSpeed(RemoteXY.slider_right, enableBoostMode);
    LcontrolMotor(MOTOR_LEFT_FW_PIN, MOTOR_LEFT_BW_PIN, speedSlider,
                  directionSlider);
    RcontrolMotor(MOTOR_RIGHT_FW_PIN, MOTOR_RIGHT_BW_PIN, speedSlider,
                  directionSlider);
    delay(1);
}
