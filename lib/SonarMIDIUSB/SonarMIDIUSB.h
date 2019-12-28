#ifndef SONARMIDIUSB_H
#define SONARMIDIUSB_H

#include <Arduino.h>
#include <MIDIUSB.h>

class SonarMIDIUSB{
    private:
        midiEventPacket_t midiEvent;
    public:
        void sendNoteOn(byte note, byte velocity = 100, byte channel = 0);
        void sendNoteOff(byte note, byte velocity = 0, byte channel = 0);
        void sendAfterTouch(byte note, byte pressure = 100, byte channel = 0);
        void sendControlChange(byte control, byte value, byte channel = 0);
        void sendPitchBend(int value, byte channel = 0);
};

inline void SonarMIDIUSB::sendNoteOn(byte note, byte velocity, byte channel) {
  midiEvent = {uint8_t(0x09), uint8_t(0x90 | channel), uint8_t(note), uint8_t(velocity)};
  MidiUSB.sendMIDI(midiEvent);
}

inline void SonarMIDIUSB::sendNoteOff(byte note, byte velocity, byte channel) {
  midiEvent = {uint8_t(0x08), uint8_t(0x80 | channel), uint8_t(note), uint8_t(velocity)};
  MidiUSB.sendMIDI(midiEvent);
}

inline void SonarMIDIUSB::sendAfterTouch(byte note, byte pressure, byte channel) {
  midiEvent = {uint8_t(0x0A), uint8_t(0xA0 | channel), uint8_t(note), uint8_t(pressure)};
  MidiUSB.sendMIDI(midiEvent);
}

inline void SonarMIDIUSB::sendControlChange(byte control, byte value, byte channel) {
  midiEvent = {uint8_t(0x0B), uint8_t(0xB0 | channel), uint8_t(control), uint8_t(value)};
  MidiUSB.sendMIDI(midiEvent);
}

inline void SonarMIDIUSB::sendPitchBend(int value, byte channel) {
  midiEvent = {uint8_t(0x0E), uint8_t(0xE0 | channel), uint8_t(value & 0x7F), uint8_t(value >> 7)};
  MidiUSB.sendMIDI(midiEvent);
}

#endif