#include <MIDI.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "KeypadSequencer.h"
#include "src/Key.h"
#include "src/Keypad.h"
#include "initsequences.h"





using namespace midi;

const uint8_t notechars[12] = {
  SEG7_C, // c
  SEG7_D, // d
  SEG7_D, // d
  SEG7_E, // e
  SEG7_E, // e
  SEG7_F, // f
  SEG7_G, // g
  SEG7_G, // g
  SEG7_A, // a
  SEG7_A, // a
  SEG7_B, // b
  SEG7_B //b
};
const boolean noteflat[12] = {false, true, false, true, false, false, true, false, true, false, true, false};


const byte ROWS = 4;
const byte COLS = 3;

byte jammode = 0;


const byte btnStop = 11;
byte btnStopState = 0;
const byte btnPlay = 12;
byte btnPlayState = 0;

const int btnShift = 9;
byte btnShiftState = 0;

const uint8_t btnFunc = 10;
byte btnFuncState = 0;
byte btnFuncReading = 0;
byte btnFuncLastState = 0;
long lastDebounceTime;
byte debounceDelay = 50;
byte seqmode = 0;
byte clockmode = CLOCKINT;
TimeDiv timDiv(2);

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

byte potBpm = A0;
uint8_t bpm = 0;
uint16_t bpmtime = 0;
boolean bpmedit = false;

uint8_t potTimeDiv = A1;

byte note = 60;
byte playednote = 0;
byte currentnote = 0;
uint8_t currentstep = 0;
uint8_t nextsteptick = 0;
unsigned long previousMillis = 0;
unsigned long previousTickMillis = 0;
long interval = 1000;
boolean bnote = false;


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



Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


int noteindex = 0;

// tests for using pattern class
Pattern pattern(patternbytes[0][0],patternbytes[0][1]);

// pattern select mode definitions
byte activepattern = 1;
byte activestore = 0;

// edit mode definitions MODE_STEP
byte noteedit = 0;
byte stepshift = 0;
byte activeStep = 0;




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
      seqmode = (seqmode + 1) % 7;
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
          case MODEPATSELECT:
            activestore = activestore?0:1;
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
      break;
      case RELEASED:
        
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
          handleNumKeyReleased(key);
          break;
        } // end switch released;
      break;
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
  pattern.notes[step ] = note;
}

void incStepNote(int step) {
  int curroct  = pattern.notes[step] - (pattern.notes[step]%12);
  int offset = (pattern.notes[step] + 1) % 12;
  setStepNote(step, curroct + offset);
  
}

void decOct(int step) {
  int curr = pattern.notes[step];
  curr = curr - 12;
  if (curr < 0) {
    pattern.notes[step] = curr + 12;
  } else {
    pattern.notes[step] = curr;
  }
  
}

void incOct(int step) {
  int curr = pattern.notes[step];
  curr = curr + 12;
  if (curr > 127) {
    pattern.notes[step] = curr -12;
  } else {
    pattern.notes[step] = curr;
  }
  
}

void noteStep() {
  currentstep = noteindex;
  MIDI.sendNoteOff(currentnote, 0,1);
  switch (jammode) {
  case 0:
    if (pattern.actives[noteindex]) {
      MIDI.sendNoteOn(pattern.notes[noteindex], 127,1);
      currentnote = pattern.notes[noteindex];
    }
    break;
  case 1:
    if (random(10) > 1) {
      MIDI.sendNoteOn(pattern.notes[noteindex], 127,1);
      currentnote = pattern.notes[noteindex];
    }
    break;
  case 2:
    MIDI.sendNoteOn(currentnote, 127,1);
    break;
  case 3:
   {
    byte rndnote = pattern.notes[random(16)];
    MIDI.sendNoteOn(rndnote, 127,1);
    currentnote = rndnote;
    break;
   }
  case 4:
   {
    byte rndnote = random(10, 80);
    MIDI.sendNoteOn(rndnote, 127,1);
    currentnote = rndnote;
    break;
   }
  
  
  } // end switch jammode
  
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
     seg7.writeDigitNum(1,((stepdisplay) % 8) +1 );
     seg7.writeDigitRaw(3, notechars[pattern.notes[stepdisplay]% 12]);
     // b oder Oktaveninfo?
     if (noteflat[pattern.notes[stepdisplay] % 12]) {
      seg7.writeDigitRaw(4, B01111100);
     } else {
      seg7.writeDigitNum(4, pattern.notes[stepdisplay] / 12);
     }
     if (!pattern.actives[stepdisplay]) {
      seg7.writeDigitRaw(3, B01000000);
      seg7.writeDigitRaw(4, B01000000);
    
     }
     
     if (stepdisplay > 7) {
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
        seg7.writeDigitRaw(0, B00000000);
        seg7.writeDigitRaw(1, SEG7_I);
        seg7.writeDigitRaw(3, SEG7_N);
        seg7.writeDigitRaw(4, SEG7_T);
     } else {
        seg7.writeDigitRaw(0, SEG7_M);
        seg7.writeDigitRaw(1, SEG7_I);
        seg7.writeDigitRaw(3, SEG7_D);
        seg7.writeDigitRaw(4, SEG7_I);
     }
     seg7.writeDisplay();
     break;
  case MODEJAM:
     seg7.writeDigitRaw(0,SEG7_J);
     seg7.writeDigitRaw(1,SEG7_A);
     seg7.writeDigitRaw(3,SEG7_M);
     if (jammode > 0) {
       seg7.writeDigitNum(4, jammode, false);
       
     } else {
      seg7.writeDigitRaw(4, B00000000);
     }
     
     seg7.writeDisplay();
     break;
  case MODEPATSELECT:
    seg7.printNumber(activepattern, DEC);
    if (activestore) {
      seg7.writeDigitRaw(0, SEG7_S);
      seg7.writeDigitRaw(1, SEG7_A);
      seg7.writeDigitRaw(3, SEG7_V);
    } else {
      seg7.writeDigitRaw(0, SEG7_P);
      seg7.writeDigitRaw(1, SEG7_A);
      seg7.writeDigitRaw(3, SEG7_T);
    }
    seg7.writeDisplay();
    break;
  } // end switch mode
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
         if (currentstep%2 == 0) {
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
    case MODEPATSELECT:
      if (activestore) {
        if (numKey < 4) {
          copyPatternToSlot(patternbytes[numKey-1][0], patternbytes[numKey-1][1], pattern.notes, pattern.actives);
        }
        activestore = 0;
        activepattern = numKey;

      } else {
        patternSelect(numKey);
        activepattern = numKey;
      }
      break;
    case MODEJAM:
      jam(numKey, true);
      break;
    default:
      seqmode = MODESTEP;
      if (activeStep != (numKey -1) + 8 * stepshift) {
          activeStep = (numKey -1) + 8*stepshift;
        } else {
        if (btnShiftState == 1) {
          pattern.actives[activeStep] = !pattern.actives[activeStep];
        } else {
          incStepNote(activeStep);
        }
      }
      
      break;
    } // end switch seqmode
    break;
  case 9:
    pattern.actives[activeStep] = !pattern.actives[activeStep]; 
    break;
  case 0:
    switch(seqmode) {
    case MODESTEP:
      if(stepshift) {
        stepshift = 0;
        activeStep = activeStep-8;
      } else {
        stepshift = 1;
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

void handleNumKeyReleased(char key) {
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
    case MODEJAM:
      jam(numKey, false);
      break;
    } // end switch seqmode '0'
    break;
    
    
    

  } // end switch numKey
  
  
}

void jam(byte num, boolean bjam) {
  switch(num) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
    jammode = bjam?num:0;
    break;
  }
  
  
}

void patternSelect(byte numKey) {
  switch(numKey) {
  case 1:
  case 2:
  case 3:
    pattern.setPattern(patternbytes[numKey-1][0], patternbytes[numKey-1][1]);
    break;
  }
}


