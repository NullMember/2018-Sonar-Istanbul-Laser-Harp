#include <Arduino.h>
#include <SonarMIDIUSB.h>

#define DEBUG_ALL
#define DEBUG_INFO

const uint8_t laserCount = 8;                                                   //How many laser attached to Laser Arp?
const uint8_t scaleCount = 2;                                                   //How many scale was defined
const uint8_t scalePin = 3;                                                     //Scale change button pin
const uint8_t ldrPins[laserCount] = {A0, A1, A2, A3, A6, A7, A8, A9};           //LDR pins
const uint8_t scale[scaleCount][laserCount] = {{0, 2, 4, 5, 7, 9, 11, 12},      //Major Scale
                                               {0, 2, 3, 5, 7, 8, 10, 12}};     //Minor Scale

volatile uint8_t _scale = 0;                                                    //Current scale (used in interrupt)
volatile bool scaleChanged = 0;                                                 //Is scale changed? (used in interrupt)
uint8_t baseNote = 60;                                                          //Base note
uint8_t notes[laserCount];                                                      //Notes scaled with baseNote + scale[] array
uint8_t velocity[laserCount];                                                   //Velocity was defined for touch sensitivity. Not used yet
uint8_t noteOn[laserCount];                                                     //Is note on?
uint16_t ldrNow[laserCount];                                                    //Current LDR values. Used in self calibration
uint16_t ldrAvr[laserCount];
uint32_t lpf;
uint8_t lpfDiv = 6;
uint8_t lpfCount = 64;
uint8_t botThreshold = 50;
uint8_t topThreshold = 200;

SonarMIDIUSB midi;                                                              //MIDIUSB object

void changeScale();                                                             //Change scale subroutine. Used for interrupt
void lpfInit();                                                                 //Low pass filter initialization for analog inputs
void lpfCalc(uint8_t);                                                          //Low pass filter calculation for analog inputs

void setup(){
    Serial.begin(115200);                                                       //Write LOW to relay pin
    pinMode(scalePin, INPUT_PULLUP);                                            //Set scale button pin to INPUT with PULLUP resistor
    attachInterrupt(digitalPinToInterrupt(scalePin), changeScale, FALLING);     //Trigger changeScale subroutine at scalePin FALLING Edge 
    for(uint8_t i = 0; i < laserCount; i++){                                            
        pinMode(ldrPins[i], INPUT);                                             //Set LDR pins to INPUT
        notes[i] = baseNote + scale[_scale][i];                                 //Set notes with (baseNote + scale[current scale]) calculation
        noteOn[i] = 0;                                                          //Set noteOn array to 0. This array is necessery because we don't want to send noteOn-Off multiple times
        velocity[i] = 100;                                                      //Set velocity array to 100
    }
    lpfInit();                                                                  //Initialize low pass filter
}

void loop(){
    if(scaleChanged){                                                           //if scaleChanged variable set (in interrupt)
        scaleChanged = 0;                                                       //set scaleChanged to 0
        if(_scale == scaleCount) _scale = 0;                                    //if _scale variable reached to maximum scale count set _scale to 0
                                                                                //this is necessery because flexibility
        for(uint8_t i = 0; i < laserCount; i++){
            midi.sendNoteOff(notes[i]);                                         //send noteOff message first. If we do not triggered note will stay at noteOn forever
            noteOn[i] = 0;                                                      //set NoteOn to 0
            notes[i] = scale[_scale][i] + baseNote;                             //change notes with new scale
        }
    }
    for(uint8_t i = 0; i < laserCount; i++){
        ldrNow[i] = analogRead(ldrPins[i]);                                     //read current LDR value
        if(ldrNow[i] > (ldrAvr[i] + topThreshold)){                             //if current LDR value greater than average ldr value + threshold
            lpfCalc(i);                                                         //filter out high frequency
        }
        #ifdef DEBUG_ALL
        Serial.print("LDR No: ");
        Serial.print(i);
        Serial.print(" Okunan deger: ");
        Serial.println(ldrNow[i]);
        #endif
        if((ldrNow[i] < (ldrAvr[i] - botThreshold)) && (noteOn[i] == 0)){       //if average(threshold) LDR value is greater than current LDR value AND we not sent noteOn before
            midi.sendNoteOn(notes[i], velocity[i]);                             //send noteOn message with velocity
            noteOn[i] = 1;                                                      //set noteOn to 1
            #ifdef DEBUG_INFO
            Serial.print("Nota: ");
            Serial.print(notes[i]);
            Serial.println(" icin NoteOn gonderildi.");
            #endif
        }
        if((ldrNow[i] > ldrAvr[i]) && (noteOn[i] == 1)){                        //if average value is less than current LDR value AND we not sent noteOff before
            midi.sendNoteOff(notes[i]);                                         //send noteOff message
            noteOn[i] = 0;                                                      //set noteOn to 0
            lpfCalc(i);                                                         //recalculate average ldr value
            #ifdef DEBUG_INFO
            Serial.print("Nota: ");
            Serial.print(notes[i]);
            Serial.println(" icin NoteOff gonderildi.");
            #endif
        }
    }
}

void changeScale(){
    _scale++;
    scaleChanged = 1;
}

void lpfInit(){
    #ifdef DEBUG_INFO
    Serial.print("LDR Average: ");
    #endif
    for(uint8_t i = 0; i < laserCount; i++){
        lpfCalc(i);
        #ifdef DEBUG_INFO
        Serial.print(ldrAvr[i]);
        Serial.print(", ");
        #endif
    }
    #ifdef DEBUG_INFO
    Serial.println();
    #endif
}

void lpfCalc(uint8_t LDR){
    lpf = 0;
    for(uint8_t i = 0; i < lpfCount; i++){
        lpf += analogRead(ldrPins[LDR]);
    }
    lpf = lpf >> lpfDiv;
    ldrAvr[LDR] = lpf;
    #ifdef DEBUG_INFO
    Serial.print("New LDR ");
    Serial.print(LDR);
    Serial.print(" Average: ");
    Serial.println(ldrAvr[LDR]);
    #endif
}