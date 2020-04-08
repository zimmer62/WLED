/*
 * Chime side of the doorbell 
 * Opto isolation between solenoid and circuit is recomended.
 * set pixel 1-3 to anything other than off to initiate chime
 * set pixel 1-3 to off to reset that chime
 * chimes will fire for triggerLength in milliseconds. Could risk burning out solenoid if wrong setting.
 * Due to WLED replay support, that should be disabled due to a shared pin 12
 */

struct chime
{
    int led;
    int pin;
    unsigned long trigger;
    unsigned long triggerLength;
    unsigned long coolDown;
};

struct beat
{
    bool note1;
    bool note2;
    bool note3;
};

class Song
{
private:
    beat *beats;
    int numberOfBeats;
    int tempo;
    unsigned long lastNoteTime;
    int currentNote = -1;
    chime *chimes;

public:
    Song(chime chimes_[], beat beats_[], int numberOfBeats_, int tempo_)
    {
        chimes = chimes_;
        beats = beats_;
        numberOfBeats = numberOfBeats_;
        tempo = tempo_;
    };

    void Play()
    {
        currentNote = 0;
    };

    void CheckNote()
    {
        if (currentNote >= 0)
        {
            if ((millis() - lastNoteTime) > tempo)
            {
                //play current note:
                DEBUG_PRINT("Playing Note: ");
                DEBUG_PRINTLN(currentNote);

                beat b = beats[currentNote];                
                if (b.note1) { setRealtimePixel(chimes[0].led,25,0,0,0); } else { setRealtimePixel(chimes[0].led,0,0,0,0);}
                if (b.note2) { setRealtimePixel(chimes[1].led,0,25,0,0); } else { setRealtimePixel(chimes[1].led,0,0,0,0);}
                if (b.note3) { setRealtimePixel(chimes[2].led,0,0,25,0); } else { setRealtimePixel(chimes[2].led,0,0,0,0);}
                strip.setBrightness(100);
                strip.show();
                
                

                lastNoteTime = millis();
                currentNote++;
            }
        }
        if (currentNote >= numberOfBeats) { currentNote = -1;}
    }
};

beat beats[] = {
    {0, 1, 0},
    {0, 0, 1},
    {0, 1, 0},
    {1, 0, 0},
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    {0, 1, 0},    
    };

beat beats2[] = {
    {0,1,0},
    {1,0,0},
    {0,0,1},
    {0,1,0},
    {0,0,0},
    {0,1,0},
    {0,0,1},
    {0,1,0},
    {1,0,0},
};

chime chimes[3];

Song song1 = Song(chimes, beats2, sizeof(beats2) / sizeof(beats2[0]), 830);

//     0,
//     1000,
//     0
// }




char doorbellSwitchTopic[38] = "wled/doorbell-switch";

void setupPinMode()
{
    for (int i = 0; i < sizeof chimes / sizeof chimes[0]; i++)
    {
        pinMode(chimes[i].pin, OUTPUT);
        digitalWrite(chimes[i].pin, LOW);
    }
}

void userHandleMQTTPayload(char *topic, char *payload)
{
    DEBUG_PRINT("Topic: ");
    DEBUG_PRINTLN(topic);
    DEBUG_PRINT("Payload: ");
    DEBUG_PRINTLN(payload);
    if (strstr(topic, "/doorbell"))
    {
        if (strstr(payload, "ring"))
        {
            DEBUG_PRINTLN("******* This is what we were looking for RING!!!!!!");
            song1.Play();
            //DO SOMETHING!!!
        }
    }
}

//mqttDeviceTopic //this device
//mqttGroupTopic // /all

void userMQTTSubscribe()
{
    char subuf[38];
    strcpy(subuf, doorbellSwitchTopic); //wled/doorbell-switch/doorbell
    strcat(subuf, "/doorbell");
    mqtt->subscribe(subuf, 0);
    DEBUG_PRINT("Subscribed to: ");
    DEBUG_PRINTLN(subuf);

    strcpy(subuf, mqttDeviceTopic); //wled/doorbell/doorbell
    strcat(subuf, "/doorbell");
    mqtt->subscribe(subuf, 0);
    DEBUG_PRINT("Subscribed to: ");
    DEBUG_PRINTLN(subuf);
}

void userSetup()
{
    chimes[0] = {0, 13, 1, 15, 250}; //chime 3 Pin 13 = D1_MINI D7 //low note
    chimes[1] = {2, 14, 1, 15, 250}; //chime 1 Pin 14 = D1_MINI D5 //middle note
    chimes[2] = {1, 12, 1, 15, 250}; //chime 2 Pin 12 = D1_MINI D6 //high note ** NOTE THIS IS SHARED WITH RLYPIN, DISABLE THIS IN NpbWrapper.h
    setupPinMode();
}

void userConnected()
{
}

bool isOn(int pixel)
{
    uint32_t color = strip.getPixelColor(pixel);
    return ((color >> 16) + (color >> 8) + color);
}

void checkReset(bool isOn, unsigned long &trigger)
{
    if (!isOn && trigger == 0)
    {
        trigger = 1;
    }
}

void checkTrigger(bool isOn, unsigned long &trigger)
{
    if (isOn && trigger == 1)
    {
        trigger = 2;
    }
}

void checkHigh(int pin, unsigned long &trigger)
{
    if (trigger == 2)
    {
        DEBUG_PRINT("Pin ");
        DEBUG_PRINT(pin);
        DEBUG_PRINT(" ");
        DEBUG_PRINTLN("High");
        digitalWrite(pin, HIGH);
        trigger = millis();
    }
}

void checkLow(int pin, unsigned long &trigger, unsigned long triggerLength)
{
    if ((trigger > 2) && ((millis() - trigger) > triggerLength))
    {
        DEBUG_PRINT("Pin ");
        DEBUG_PRINT(pin);
        DEBUG_PRINT(" ");
        DEBUG_PRINTLN("Low");
        digitalWrite(pin, LOW);
        trigger = 0;
    }
}

void doChime(int led, int pin, unsigned long &trigger, unsigned long triggerLength)
{
    bool b = isOn(led);
    checkReset(b, trigger);
    checkTrigger(b, trigger);
    checkHigh(pin, trigger);
    checkLow(pin, trigger, triggerLength);
}

void userLoop()
{
    for (int i = 0; i < sizeof chimes / sizeof chimes[0]; i++)
    {
        doChime(chimes[i].led, chimes[i].pin, chimes[i].trigger, chimes[i].triggerLength);
    }
    song1.CheckNote();

    yield();
}