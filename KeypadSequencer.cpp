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


