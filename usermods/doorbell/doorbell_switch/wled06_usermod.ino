/*
 * Door bell switch that sends an MQTT message when button is pressed
 * IO pin needs pullup resistor between it and ground.
 * 
 * TODO: Could probably write funciton to handle press and release.
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

void sendDoorBellRing() {
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

void sendDoorBellRelease() {
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
   
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == 1) {
                sendDoorBellRing();
            } else {
                sendDoorBellRelease();
            }
        }        
    } 
    lastButtonState = reading;
}
