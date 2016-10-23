#ifndef KeypadSequencer_h
#define KeypadSequencer_h

#include <Arduino.h>
#include "Adafruit_LEDBackpack.h"



#define MODESTEP 0
#define MODEPLAY 1
#define MODEBPM 2
#define MODETIMDIV 3
#define MODECLOCK 4
#define MODEJAM 5
#define MODEPATSELECT 6

#define CLOCKINT 0
#define CLOCKEXT 1

//SEG7 Letters
#define SEG7_A B01110111
#define SEG7_B B01111100
#define SEG7_C B00111001
#define SEG7_D B01011110
#define SEG7_E B01111001
#define SEG7_F B01110001
#define SEG7_G B00111101
#define SEG7_I B00000110
#define SEG7_J B00001110
#define SEG7_M B00010101
#define SEG7_N B01010100
#define SEG7_O B00111111
#define SEG7_P B01110011
#define SEG7_S B01101101
#define SEG7_T B01111000
#define SEG7_U B00111110
#define SEG7_V B00111110

//***************************

class Button {

  public:
    Button(byte pin);
    Button(byte pin, byte debounce);
    byte getValue();

  private:
    byte mPin;
    byte mDebounce;
    byte mPrevious;
    byte mCurrent;
    byte mStatus;
    byte mValue;
  
};

class TimeDiv {
  private:
    uint8_t mValues[4] = {4, 8, 16, 32};
    byte mIndex = 3;
   
  public:
    TimeDiv(byte initIndex);
    byte getValue();
    byte nextValue();
    void drawValue(Adafruit_7segment *seg);
    byte swing = 0;
    
     
};

class Step {
  public:
    byte note = 0;
    boolean active = 0;
    Step(byte note, byte active);
    Step();
    
    
};

class Pattern {
  public:
    byte patlength = 16;
    byte currentstep = 0;
    byte currentnote;
    Step steps[16];
    byte notes[16];
    byte actives[16];
    byte* getNotes();
    Pattern(byte*, byte*);
    void setPattern(byte*, byte*);

   
    
};

Step* buildStepArrayFromRaw(byte* notes, byte* actives);

void copyPatternToSlot(byte * target_notes, byte * target_actives, byte * notes, byte * actives);


  


#endif
