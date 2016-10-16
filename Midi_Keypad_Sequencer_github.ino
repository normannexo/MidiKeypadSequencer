#include <MIDI.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <Keypad.h>
#include "KeypadSequencer.h"


using namespace midi;

uint8_t notechars[12] = {
  B00111001, // c
  B01011110, // d
  B01011110, // d
  B01111001, // e
  B01111001, // e
  B01110001, // f
  B00111101, // g
  B00111101, // g
  B01110111, // a
  B01110111, // a
  B01111100, // b
  B01111100 //b
};

const byte ROWS = 4;
const byte COLS = 3;
const int btnStop = 13;
int btnStopState = 0;


const int btnPlay = 12;
int btnPlayState = 0;

const int btnShift = 9;
int btnShiftState = 0;

const uint8_t btnFunc = 10;
int btnFuncState = 0;
int btnFuncReading = 0;
int btnFuncLastState = 0;
long lastDebounceTime;
int debounceDelay = 50;
int seqmode = 0;
byte clockmode = CLOCKINT;
TimeDiv timDiv(1);

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};


byte rowPins[ROWS] = {5,4,3,2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8,7,6}; //connect to the column pinouts of the keypad

//play control
boolean play = false;
boolean playpressed = false;
uint8_t activepoint[4] = {0, 1, 3,4}; 

int potBpm = A0;
uint8_t bpm = 0;
int bpmtime = 0;
boolean bpmedit = false;

uint8_t potTimeDiv = A1;

int note = 60;
int playednote = 0;
int currentnote = 0;
uint8_t currentstep = 1;
uint8_t nextsteptick = 0;
unsigned long previousMillis = 0;
unsigned long previousTickMillis = 0;
long interval = 1000;
boolean bnote = false;

byte data1 = 0;
// init 7segment
Adafruit_7segment seg7 = Adafruit_7segment();
uint8_t stepdisplay; // step to display: selected or current note


uint8_t stopchars[4] = {
  B01101101,
  B01111000,
  B01011100,
  B01110011
};
int stepspeed = 6;
uint8_t ticks = 0;


boolean noteflat[12] = {false, true, false, true, false, false, true, false, true, false, true, false};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// define Sequence
int activeStep = 1;
int notes[16] = {60,60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60};
boolean stepactive[16] = {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true};
int noteindex = 0;
boolean stepshift = false;



MIDI_CREATE_DEFAULT_INSTANCE();
void setup() {
   pinMode(btnStop,INPUT);
   pinMode(btnPlay, INPUT);
   pinMode(btnShift, INPUT);
   pinMode(btnFunc, INPUT);
   MIDI.setHandleClock(handleClock);
   MIDI.begin(1);
   muteAll();
   stepspeed = 24/(timDiv.getValue()/4);
   keypad.addEventListener(keypadEvent);

   seg7.begin(0x71);
   
   seg7.writeDisplay();
}



void loop() {
  // put your main code here, to run repeatedly:

   unsigned long currentMillis = millis(); 
   
   keypad.getKey();

   if (clockmode == CLOCKEXT) {
     MIDI.read();
   }
   bpmtime = analogRead(potBpm);
   interval = map(bpmtime, 0, 1023, 50, 1000);
   bpm = map(bpmtime, 0, 1023, 60, 170);


   btnStopState = digitalRead(btnStop);
   if (btnStopState == 1) {
      play = false;
      if (clockmode == CLOCKINT) {
        MIDI.sendRealTime(Stop);
      }
      ticks = 0;
      nextsteptick = 0;
      muteAll();
   }

   btnShiftState = digitalRead(btnShift);
  
   
   btnPlayState = digitalRead(btnPlay);
   if (btnPlayState == 1) {
      if (!play) {
              play = true;
              resetAndPlay();
           }
   }

   btnFuncReading = digitalRead(btnFunc);
   if (btnFuncReading != btnFuncLastState) {
    lastDebounceTime = millis();
   }
   if((millis() - lastDebounceTime) > 20 ) {  // button latch, no debounce needed.
    if (btnFuncState != btnFuncReading) {
      btnFuncState = btnFuncReading;
      if (btnFuncState == 1) {
      seqmode = (seqmode + 1) % 5;
      }
    } 
   }
   btnFuncLastState = btnFuncReading;
   
   
 
  
  if (clockmode == CLOCKINT) {
    if (currentMillis - previousMillis >= (60000/bpm)/24|| playpressed) {
      maketick();
      previousMillis = millis();
    }
  } 


  // Anzeigenblock
  display(seqmode);
  
  
  
  

 

 
 delay(1);

}


void keypadEvent(KeypadEvent key){
    switch (keypad.getState()){
    case PRESSED:
        if (key == '#') {
          switch(seqmode){
          case MODESTEP:
           if (btnShiftState == 0) {
           incOct(activeStep);
            } else {
             for (uint8_t i = 1; i <= 16; i++) {
              incOct(i);
            }
          }
          break;
          case MODETIMDIV:
             timDiv.nextValue();
             break;

          case MODECLOCK: 
            clockmode = (clockmode + 1)%2;
            break;
          
          }
         
         
        }
        if (key == '*') {
          switch (seqmode) {
          case MODESTEP:
          case MODEPLAY:
            if (btnShiftState == 1) {
            for (uint8_t i = 1; i <=16; i++) {
              decOct(i);
            }
            } else {
            decOct(activeStep);
            }
            
            break;
          case MODETIMDIV:
            timDiv.swing = (timDiv.swing +1) %2;
            break;
          
          }
        }
       
        switch (key - '0') {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 0:
          handleNumKeyPressed(key);
          break;
       
        } // end case keypressed
        
      } // end switch getkeystate
    
}

void muteAll() {
  
  for (int i= 0; i< 128; i++) {
    MIDI.sendNoteOff(i, 0, 1);
  }
}

void resetAndPlay() {
  currentnote = 0;
  noteindex = 0;
  playpressed=true;
  
}

void setStepNote(int step, int note) {
  notes[step - 1] = note;
}

void incStepNote(int step) {
  int curroct  = notes[step- 1] - (notes[step - 1]%12);
  int offset = (notes[step-1] + 1) % 12;
  setStepNote(step, curroct + offset);
  
}

void decOct(int step) {
  int curr = notes[step-1];
  curr = curr - 12;
  if (curr < 0) {
    notes[step-1] = curr + 12;
  } else {
    notes[step -1] = curr;
  }
  
}

void incOct(int step) {
  int curr = notes[step-1];
  curr = curr + 12;
  if (curr > 127) {
    notes[step-1] = curr -12;
  } else {
    notes[step -1] = curr;
  }
  
}

void noteStep() {
  currentstep = noteindex + 1;
  MIDI.sendNoteOff(currentnote, 0,1);
  if (stepactive[noteindex]) {
    MIDI.sendNoteOn(notes[noteindex], 127,1);
    currentnote = notes[noteindex];
  }
  
  noteindex = (noteindex+1)%16;

}




void display(uint8_t mode) {
  switch (mode) {
  case MODEBPM:
    seg7.printNumber(bpm, DEC);
    seg7.writeDisplay();
    break;
  case MODETIMDIV:
    timDiv.drawValue(&seg7);
    break;
  case MODEPLAY:
  case MODESTEP:
     if (seqmode==MODEPLAY) {
       stepdisplay = currentstep;
     } else {
       stepdisplay = activeStep;
     }
      seg7.writeDigitNum(1,((stepdisplay-1) % 8) +1 );
     seg7.writeDigitRaw(3, notechars[notes[stepdisplay-1]% 12]);
     // b oder Oktaveninfo?
     if (noteflat[notes[stepdisplay-1] % 12]) {
      seg7.writeDigitRaw(4, B01111100);
     } else {
      seg7.writeDigitNum(4, notes[stepdisplay-1] / 12);
     }
     if (!stepactive[stepdisplay-1]) {
      seg7.writeDigitRaw(3, B01000000);
      seg7.writeDigitRaw(4, B01000000);
    
     }
     
     if (stepdisplay > 8) {
      seg7.writeDigitRaw(0, B01100011);
     } else {
      seg7.writeDigitRaw(0, B01011100);
     }
     if (ticks % 24 < 12) {
       seg7.drawColon(true);
     } else {
       seg7.drawColon(false);
     }
     if (mode == MODEPLAY && !play) {
       seg7.writeDigitRaw(0, stopchars[0]);
       seg7.writeDigitRaw(1, stopchars[1]);
       seg7.writeDigitRaw(3, stopchars[2]);
       seg7.writeDigitRaw(4, stopchars[3]);
       seg7.drawColon(false);
     }
     seg7.writeDisplay();
     break;
  case MODECLOCK:
     if (clockmode == CLOCKINT) {
        seg7.writeDigitRaw(1, B00000110);
        seg7.writeDigitRaw(3, B01010100);
        seg7.writeDigitRaw(4, B01111000);
     } else {
        seg7.writeDigitRaw(1, B00111001);
        seg7.writeDigitRaw(3, B00110000);
        seg7.writeDigitRaw(4, B01011100);
     }
     seg7.writeDisplay();
     break;
  
  }
}

void handleClock() {
   maketick();
  
}

void maketick() {
  if (play) {
    
      if (playpressed) {
        if (clockmode == CLOCKINT) {
          MIDI.sendRealTime(Start);
        }
        playpressed = false;
        ticks = 0;
      }
      
      if (clockmode == CLOCKINT) {
        MIDI.sendRealTime(Clock);
      }

      if (!timDiv.swing) {
        if (ticks == nextsteptick){
          noteStep();
          nextsteptick = (nextsteptick + stepspeed) % 96;
        }
      } else {
       if (ticks == nextsteptick) {
         noteStep();
         if (currentstep%2 == 1) {
            nextsteptick = (nextsteptick + (int)(stepspeed * 1.3)) % 96;
         } else {
            nextsteptick = (nextsteptick + (2*stepspeed - (int)(1.3*stepspeed))) % 96;
         }
       }
      }
      ticks = (ticks+1)%96;
    
    
  } 
}

void handleNumKeyPressed(char key) {
  byte numKey = key - '0';
  switch (numKey) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
    switch (seqmode) {
    default:
      seqmode = MODESTEP;
      if (!stepshift) {
        activeStep = numKey;
      } else {
        activeStep = numKey + 8;
      }
      if (btnShiftState == 1) {
        stepactive[activeStep - 1] = !stepactive[activeStep-1];
      }
    } // end switch seqmode
    break;
  case 9:
    incStepNote(activeStep); 
    break;
  case 0:
    switch(seqmode) {
    case MODESTEP:
      if(stepshift) {
        stepshift = false;
        activeStep = activeStep-8;
      } else {
        stepshift = true;
        activeStep = activeStep + 8;
      }
      break;
     case MODETIMDIV:
      stepspeed = 24/(timDiv.getValue()/4);
      break;
            
    } // end switch seqmode '0'
    break;
    
    
    

  } // end switch numKey
  
  
}

