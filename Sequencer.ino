#include <SPI.h>
#define trigOutPin           B00001000 //A3 (17)
#define trigInPin            B00010000 //A4 (18)
#define ctrlRegsCLK           B00000010 //A1 (15)
#define ctrlRegsData          B00000100 //A2 (16)
#define gateOutSeq1          B00000100 //D2
#define ssSequence1          B00000010 //D9
#define gateInSeq1           B00010000 //D4

bool triggered;
uint16_t triggerStepMax = 250; // The Tempo ~0-1024
uint16_t triggerStep;
uint16_t controlStepMax = 3;
uint16_t controlStep;
uint8_t ctrlRegsOp;

volatile bool ctrl;
volatile bool ctrlFast;


uint8_t mainTempoStep;

uint8_t minorScale[12] = {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 10, 10};
uint8_t majorScale[12] = {0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9, 11};
uint8_t sequenceScale = 0;

uint8_t sequence1Type = 0; //0-normal;1-skipOffSteps;2-random
uint8_t sequence1Dir = 0; //0-forward;1-backword;2-pingpong
bool pingPongSeq1Dir;
bool pingPongSeq1Flip;
bool sequence1GateTimer;
bool sequence1Gate;
uint16_t sequence1GateTime;
uint16_t sequence1GateTimeMax = 127; // Read from pot 0-255 : how long the gate is open withing a step
uint8_t sequence1TempoStepMax = 8;  // Read from pot 32-1: sequence 1 tempo, relative to the main tempo
uint8_t sequence1Step;
uint8_t sequence1LastStep = 15; // Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1FirstStep = 0;// Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1LastOnStep;
uint8_t sequence1CV;
uint8_t sequence1Note;



void setup() {
  analogReference(EXTERNAL);
  
  cli();
  
  TIMSK0 = TIMSK0 & B11111110; // Disable millis()
  
  TCCR0B = TCCR0B & B11111000 | B00000001; //set Timer0 prescaler to 1
  TCCR0A = TCCR0A & B11111100 | B00000001; //set pwm mode on Timer0 to phase correct
  TCCR0A |= B10100000; // enable pwm


  TCCR1B = TCCR1B & B11111000; //disable Timer1

  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR2A = 249;// = (16*10^6) / (2000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 32 prescaler
  TCCR2B |= (1 << CS20) | (1 << CS21);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  sei();

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, INPUT);

  digitalWrite(10, 0);

  SPI.begin();
  
  Serial.begin(9600);

  updateRegistersControls();
  sequence1Step = 15;
}

ISR(TIMER2_COMPA_vect) {
  if (triggerStep < triggerStepMax) {
    if (triggerStep == triggerStepMax / 2) {
      PORTC |= trigOutPin;
    }
    triggerStep++;
  } else {
    PORTC &= ~trigOutPin;
    triggerStep = 0;
  }
  if (controlStep < controlStepMax) {
    controlStep++;
  } else {
    ctrl = true;
    controlStep = 0;
  }
  ctrlFast = true;
}

void loop() {
  if (ctrlFast) {
    ctrlFast = false;
    controlFast();
  }
  if (ctrl) {
    ctrl = false;
    control();
  }

}
void controlFast() {
//PINC & trigInPin
  if (!(PINC&trigInPin)) {
    if (!triggered) {
      triggered = true;
      triggerStepp();
    }
  } else {
    if (triggered) {
      triggered = false;
    }
  }
}
void control() {
  updateRegistersControls();
  sequence1GateTimer = sequence1GateTime < map(sequence1GateTimeMax, 0, 250, 0, (triggerStepMax * sequence1TempoStepMax) / (controlStepMax+1));
  if (sequence1GateTimer) {
    sequence1GateTime++;
  } else {
    if (sequence1Gate) {
      sequence1Gate = 0;
      PORTD &= ~gateOutSeq1;
    }
  }
 
}
void updateRegistersControls() {
  triggerStepMax = 200;//map(analogRead(14), 0, 1023, 250, 80); //60?-187.5 BPM
  sequence1TempoStepMax = 2;//(1 << (map(analogRead(14), 0, 1023, 5, 0)));
  switch (ctrlRegsOp) {
    case 0:
    PORTC|=ctrlRegsData;
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    PORTC&=~ctrlRegsData;
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    //Serial.println(analogRead(14));
    break;
    case 3:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    Serial.println(analogRead(14));
    break;
  }
  if (ctrlRegsOp<14) {
    ctrlRegsOp++;
  } else {
    ctrlRegsOp = 0;
  }
}
void updateRegistersSequence1() {
  uint16_t value = 15 - sequence1Step;
  SPI.transfer16(1<<value); 
  PORTB |= ssSequence1;
  PORTB &= ~ssSequence1;
  delayMicroseconds(5);
  sequence1Gate =  PIND&gateInSeq1;
}

void triggerStepp() {
  if (!(mainTempoStep % sequence1TempoStepMax)) {
    sequence1Stepp();
  }
  if (mainTempoStep < 31) {
    mainTempoStep++;
  } else {
    mainTempoStep = 0;
  }
}
void sequence1UpdateNote() {
  PORTD |= gateOutSeq1;
  sequence1CV = map(analogRead(20), 0, 1007, 0, 60);//(analogRead(20) >> 4);

  

  switch (sequenceScale) {
    case 0:
      sequence1Note = minorScale[sequence1CV % 12];
      sequence1CV = sequence1Note + ((sequence1CV / 12) * 12);
      break;
    case 2:
      sequence1Note = majorScale[sequence1CV % 12];
      sequence1CV = sequence1Note + ((sequence1CV / 12) * 12);
      break;
  }
  sequence1CV =  (sequence1CV) << 2;
  OCR0B = sequence1CV;
}
void sequence1NextStepForward() {
  if (sequence1Step < sequence1LastStep) {
    sequence1Step += 1;
  } else {
    if (sequence1Dir != 2) {
      sequence1Step = sequence1FirstStep;
    } else {
      sequence1Step -= 1;
      pingPongSeq1Dir = !pingPongSeq1Dir;
    }
  }
}
void sequence1NextStepBackword() {
  if (sequence1Step > sequence1FirstStep) {
    sequence1Step -= 1;
  } else {
    if (sequence1Dir != 2) {
      sequence1Step = sequence1LastStep;
    } else {
      sequence1Step += 1;
      pingPongSeq1Dir = !pingPongSeq1Dir;
    }
  }
}
void sequence1NextStep() {
      switch (sequence1Type) {
        case 0:
          switch (sequence1Dir) {
            case 0:
            sequence1NextStepForward();
            break;
            case 1:
            sequence1NextStepBackword();
            break;
            case 2:
            if (pingPongSeq1Dir) {
              sequence1NextStepForward();
            } else {
              sequence1NextStepBackword();
            }
            break;
          }
          updateRegistersSequence1();
        break;
        case 1:
          switch (sequence1Dir) {
            case 0:
            for (uint8_t i = 0; i < 16; i++) {
              sequence1NextStepForward();
              updateRegistersSequence1();
              if (sequence1Gate) {
                break;
              }
            }
            break;
            case 1:
            for (uint8_t i = 0; i < 16; i++) {
              sequence1NextStepBackword();
              updateRegistersSequence1();
              if (sequence1Gate) {
                break;
              }
            }
            break;
            case 2:
            for (uint8_t i = 0; i < 32; i++) {
              if (pingPongSeq1Dir) {
                sequence1NextStepForward();
              } else  {
                sequence1NextStepBackword();
              }
              updateRegistersSequence1();
              if (sequence1Gate && sequence1Step != sequence1LastOnStep) {
                sequence1LastOnStep = sequence1Step;
                break;
              }
            }
            break;
          }
        break;
        case 2:
          sequence1Step = rand()%(sequence1LastStep-sequence1FirstStep+1) + sequence1FirstStep;
          updateRegistersSequence1();
        break;
      }
}
void sequence1Stepp() {
    sequence1GateTime = 0;
    sequence1NextStep();
    if (sequence1Gate) {
      sequence1UpdateNote();
    } else {
      PORTD &= ~gateOutSeq1;
    }
}
