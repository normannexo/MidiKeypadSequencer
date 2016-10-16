#include "KeypadSequencer.h"



byte TimeDiv::nextValue() {
  mIndex = (mIndex + 1) % 4;
  return mValues[mIndex];
}

byte TimeDiv::getValue() {
  return mValues[mIndex];
}


void TimeDiv::drawValue(Adafruit_7segment* seg) {
  
  seg->printNumber(mValues[mIndex], DEC);
  seg->writeDigitNum(1, 1, true);
  if (swing == 1) {
    seg->writeDigitRaw(0, B01101101);
  }
  seg->writeDisplay(); 
 
}

TimeDiv::TimeDiv(uint8_t initIndex) {
  mIndex = initIndex;
}


void displayStep(Adafruit_7segment* seg, int stepdisplay) {
    
}


// Step class
Step::Step(byte note, boolean active) {
  this->note = note;
  this->active = active;
}

Step::Step() {
  this->note = 0;
  this->active = true;
}

//Pattern class
Pattern::Pattern(byte * notes, boolean* actives) {
  for (int i = 0; i < 16; i++) {
    this->notes[i] = notes[i];
    this->actives[i] = actives[i];
  }
  
}



// util functions

Step* buildStepArrayFromRaw(byte* notes, boolean* actives) {
  Step tmpStep[16];
  for (int i = 0; i < 16; i++) {
    tmpStep[i].note = notes[i];
    tmpStep[i].active = actives[i];
  }
  return tmpStep;
}





