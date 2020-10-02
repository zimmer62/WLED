#include "wled.h"
/*
 * Door bell switch that sends an MQTT message when button is pressed
 * IO pin needs pullup resistor between it and ground.
 * 
 * TODO: Could probably write funciton to handle press and release.
 * This v1 usermod file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * If you just need 8 bytes, use 2551-2559 (you do not need to increase EEPSIZE)
 * 
 * Consider the v2 usermod API if you need a more advanced feature set!
 */

const int pinA = 12; // 12 = D6 on D1_MINI
int buttonState = 0;
int lastButtonState = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 5;

void userSetup()
{
    pinMode(pinA, INPUT);
}

void userConnected()
{
}

void sendDoorBellRing()
{
    DEBUG_PRINTLN("Checking MQTT before sending doorbell");
    if (mqtt != nullptr)
    {
        DEBUG_PRINTLN("****Sending Doorbell NOW!****");
        char subuf[38];
        strcpy(subuf, mqttDeviceTopic);
        strcat(subuf, "/doorbell");
        mqtt->publish(subuf, 0, true, "ring");
    }
}

void sendDoorBellRelease()
{
    DEBUG_PRINTLN("Checking MQTT before sending doorbell release");
    if (mqtt != nullptr)
    {
        DEBUG_PRINTLN("****Sending Doorbell Release NOW!****");
        char subuf[38];
        strcpy(subuf, mqttDeviceTopic);
        strcat(subuf, "/doorbell");
        mqtt->publish(subuf, 0, true, "release");
    }
}

void userLoop()
{
    int reading = digitalRead(pinA);

    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        if (reading != buttonState)
        {
            buttonState = reading;
            if (buttonState == 1)
            {
                sendDoorBellRing();
            }
            else
            {
                sendDoorBellRelease();
            }
        }
    }
    lastButtonState = reading;
}
