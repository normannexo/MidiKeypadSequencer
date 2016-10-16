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
    byte mIndex = 0;
   
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
    Step(byte note, boolean active);
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
    Pattern(byte*, boolean*);

   
    
};

Step* buildStepArrayFromRaw(byte* notes, boolean* actives);


  


#endif
