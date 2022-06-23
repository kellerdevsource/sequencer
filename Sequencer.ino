#define trigOutPin           B00001000 //A3 (17)
#define trigInPin            B00001000  //A4 (18)
#define gateOutSeq1          B00000001 //D8
#define clkRegSequences      B00001000 //D3
#define dataRegSequences     B00000100 //D2
#define storageRegSequence1  B00010000 //D4
#define gateInSeq1           B01000000 //D6
bool triggered;
uint16_t triggerStepMax = 250; // The Tempo ~0-1024
uint16_t triggerStep;
uint16_t controlStepMax = 3;
uint16_t controlStep;

volatile bool ctrl;
volatile bool ctrlFast;


uint8_t mainTempoStep;
//C,C#,D,D#,E,F,F#,G,G#,A,A#,B;
//0, 1,2, 3,4,5, 6,7, 8,9,10,11;
uint8_t minorScale[12] = {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 10, 10};
uint8_t majorScale[12] = {0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9, 11};
uint8_t sequenceScale = 0;

bool sequence1Forward = true;
bool sequence1GateTimer;
bool sequence1SkipOffSteps = false;
bool sequence1Gate;
uint16_t sequence1GateTime;
uint16_t sequence1GateTimeMax = 127; // Read from pot 0-255 : how long the gate is open withing a step
uint8_t sequence1TempoStepMax = 8;  // Read from pot 32-1: sequence 1 tempo, relative to the main tempo
uint8_t sequence1Step;
uint8_t sequence1LastStep = 3; // Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1FirstStep = 0;// Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1CV;
uint8_t sequence1Note;



void setup() {
  analogReference(EXTERNAL);
  cli();

  TCCR1B = TCCR1B & B11111000 | B00000001;

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
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, INPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, INPUT);
  pinMode(15, INPUT);
  pinMode(17, OUTPUT);
  pinMode(18, INPUT);


  updateRegistersControls();
  updateRegistersSequence1(0);
  sequence1Step = sequence1LastStep;

  Serial.begin(9600);
}

ISR(TIMER2_COMPA_vect) {
  if (triggerStep < triggerStepMax) {
    if (triggerStep == triggerStepMax / 2) {
      PORTC &= ~trigOutPin;
    }
    triggerStep++;
  } else {
    PORTC |= trigOutPin;
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
  if (PINC & trigInPin) {
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
      PORTB &= ~gateOutSeq1;
    }
  }
 
}
void updateRegistersControls() {
  triggerStepMax = map(analogRead(14), 0, 1023, 250, 80); //60?-187.5 BPM
  sequence1TempoStepMax = (1 << (map(analogRead(14), 0, 1023, 5, 0)));
}
void updateRegistersSequence1(uint8_t value) {
  for (uint8_t i = 0; i < sequence1LastStep + 1; i++) {
    PORTD &= ~clkRegSequences;
    if ((value >> (sequence1LastStep - i)) & 1) {
      PORTD |= dataRegSequences;
    } else {
      PORTD &= ~dataRegSequences;
    }
    PORTD |= clkRegSequences;
  }
  PORTD |= storageRegSequence1;
  PORTD &= ~storageRegSequence1;
  PORTD &= ~clkRegSequences;
  PORTD &= ~dataRegSequences;
  sequence1Gate = (PIND & gateInSeq1);
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
void sequence1NextStepForward() {
  if (sequence1Step < sequence1LastStep) {
    sequence1Step += 1;
  } else {
    sequence1Step = sequence1FirstStep;
  }
  updateRegistersSequence1(1 << sequence1Step);
}
void sequence1NextStepBackword() {
  if (sequence1Step > sequence1FirstStep) {
    sequence1Step -= 1;
  } else {
    sequence1Step = sequence1LastStep;
  }
  updateRegistersSequence1(1 << sequence1Step);
}
void sequence1UpdateNote() {
  PORTB |= gateOutSeq1;
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
  sequence1CV =  sequence1CV << 2;
  analogWrite(9, sequence1CV);
}
void sequence1Stepp() {
  sequence1GateTime = 0;
  if (sequence1SkipOffSteps) {
    for (uint8_t i = 0; i <= (sequence1LastStep - sequence1FirstStep); i++) {
      if (sequence1Forward) {
        sequence1NextStepForward();
      } else {
        sequence1NextStepBackword();
      }
      if (sequence1Gate) {
        sequence1UpdateNote();
        break;
      }
    }
  } else {
    if (sequence1Forward) {
      sequence1NextStepForward();
    } else {
      sequence1NextStepBackword();
    }
    if (sequence1Gate) {
      sequence1UpdateNote();
    } else {
      PORTB &= ~gateOutSeq1;
    }
  }
}
