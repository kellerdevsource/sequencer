#include <SPI.h>
#define trigOutPin           B00001000 //A3 (17)
#define trigInPin            B00010000 //A4 (18)
#define ctrlRegsCLK          B00000010 //A1 (15)
#define ctrlRegsData         B00000100 //A2 (16)

#define gateOutSeq1          B00000100 //D2
#define ssSequence1          B00000010 //D9
#define gateInSeq1           B00010000 //D4

#define gateOutSeq2          B00001000 //D3
#define ssSequence2          B00000100 //D10
#define gateInSeq2           B10000000 //D7

bool triggered;
bool repeat;
uint16_t triggerStepMax = 250; // The Tempo ~0-1024
uint16_t triggerStep;
uint16_t controlStepMax = 3;
uint16_t controlStep;

volatile bool ctrl;
volatile bool ctrlFast;


uint8_t mainTempoStep = 1;

uint8_t minorScale[12] = {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 10, 10};
uint8_t SixSemiScale[12] = {0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10};
uint8_t majorScale[12] = {0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9, 11};
uint8_t sequenceScale = 0; //0 - minor; 1 - 12 seitone; 2 - 6 semitone; 3 - major


uint8_t sequence1Type = 0; //0-forward;1-backwords;2-pingpong;3-skip;4-random
bool pingPongSeq1Dir;
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
bool sequence1Play;
uint8_t repeats;
uint8_t repeatsMax;

uint8_t sequence2Type = 0; //0-forward;1-backwords;2-pingpong;3-skip;4-random
bool pingPongSeq2Dir;
bool sequence2GateTimer;
bool sequence2Gate;
uint16_t sequence2GateTime;
uint16_t sequence2GateTimeMax = 127; // Read from pot 0-255 : how long the gate is open withing a step
uint8_t sequence2TempoStepMax = 8;  // Read from pot 32-1: sequence 1 tempo, relative to the main tempo
uint8_t sequence2Step;
uint8_t sequence2LastStep = 15; // Read from pot 0-15 : how much of the sequence is played
uint8_t sequence2FirstStep = 0;// Read from pot 0-15 : how much of the sequence is played
uint8_t sequence2LastOnStep;
uint8_t sequence2CV;
uint8_t sequence2Note;
bool sequence2Play;

void setup() {
  analogReference(EXTERNAL);
  
  cli();
  
  TIMSK0 = TIMSK0 & B11111110; // Disable millis()
  
  TCCR0B = TCCR0B & B11110000 | B00000001; //set Timer0 prescaler to 1
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
  sequence2GateTimer = sequence2GateTime < map(sequence2GateTimeMax, 0, 250, 0, (triggerStepMax * sequence2TempoStepMax) / (controlStepMax+1));
  if (sequence2GateTimer) {
    sequence2GateTime++;
  } else {
    if (sequence2Gate) {
      sequence2Gate = 0;
      PORTD &= ~gateOutSeq2;
    }
  }
}


void triggerStepp() {
  if (sequence1Play && (!(mainTempoStep % sequence1TempoStepMax))) {
    sequence1Stepp();
  }
  if (sequence2Play && (!(mainTempoStep % sequence2TempoStepMax))) {
    sequence2Stepp();
  }
  if (mainTempoStep < 31) {
    mainTempoStep++;
  } else {
    mainTempoStep = 0;
  }
}
