
uint16_t controlStepMax = 20;
uint16_t controlStep;
volatile bool ctrl;
volatile bool ctrlFast;
uint16_t steppStepMax = 125; // The Tempo ~0-1024
uint16_t steppStep;

uint8_t nextPot;
uint16_t sequence1GateTime;
uint16_t sequence1GateTimeMax =128; // Read from pot 0-255 : how long the gate is open withing a step
uint8_t tempo1Step;
volatile bool sequencer1Trigger;
volatile bool sequencer1Triggered;
uint8_t tempo1StepMax = 7;  // Read from pot 15-0: sequence 1 tempo, relative to the main tempo
volatile bool tempo1StepOut;
uint8_t sequence1Step;
uint8_t sequence1LastStep = 127; // Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1FirstStep = 0;// Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1CVOut;
uint8_t sequence1CV[128];
bool sequence1GateOut;
bool sequence1Gate[128];
bool sequence1GateState;
uint8_t mainTempoStep;
bool sequence1SkipOffSteps = true;

uint8_t sequence2Step;
uint8_t sequence2LastStep = 127; // Read from pot 0-15 : how much of the sequence is played
uint8_t sequence2FirstStep = 0;// Read from pot 0-15 : how much of the sequence is played
bool sequence2Gate[128];
uint8_t sequence2CV[128];


void setup() {
  cli();

  TCCR1B = TCCR1B & B11111000 | B00000001;

  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 249;// = (16*10^6) / (2000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 32 prescaler
  TCCR2B |= (1 << CS20)|(1 << CS21);   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  sei();
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(6, INPUT);
  pinMode(14, INPUT);
  pinMode(13, OUTPUT);
  //sequence1Step = sequence1LastStep;
  buildDataBaseSequence1();
  buildDataBaseSequence2();
  Serial.begin(9600);
}

ISR(TIMER2_COMPA_vect) {
  if (steppStep < steppStepMax) {
    steppStep++;
  } else {
    stepp();
    steppStep = 0;
  }
  if (controlStep < controlStepMax) {
    controlStep++;
  } else {
    ctrl = true;
    controlStep = 0;
  }
  controlFast();
}

void loop() {
  //Serial.println("loop");
  if (ctrl) {
    control();
    ctrl = false;
  }
  if (ctrlFast) {
    controlFast();
    ctrlFast = false;
  }
}
void controlFast() {
  sequencer1Trigger = tempo1StepOut;
  if (sequencer1Trigger) {
    if (!sequencer1Triggered) {
      sequencer1Triggered = true;
      sequence1Stepp();
    }
  } else {
    sequencer1Triggered = false;
  }
}
void control() {
  buildDataBaseSequence1();
  updateRegistersSequence1(1<<sequence1Step);
  buildDataBaseSequence2();
  updateRegistersSequence2(1<<sequence2Step);
  sequence1GateState = sequence1GateTime < map(sequence1GateTimeMax, 0, 255, 0, (steppStepMax*(1 + tempo1StepMax))/controlStepMax);
  if (sequence1GateState) {
    sequence1GateTime++;
  } else {
    updateRegistersSequence1(0);
  }
}
void buildDataBaseSequence1() {
  updateRegistersSequence1(1<<sequence1FirstStep);
  sequence1Gate[sequence1FirstStep] = digitalRead(6);
  sequence1CV[sequence1FirstStep] = analogRead(14)>>4;
  for (uint8_t i = sequence1FirstStep+1; i <= sequence1LastStep; i++) {
    digitalWrite(3, 1);
    digitalWrite(4, 1);
    digitalWrite(3, 0);
    digitalWrite(4, 0);
    sequence1CV[i] = analogRead(14)>>4;
    sequence1Gate[i] = digitalRead(6);
  }
}
void buildDataBaseSequence2() {
  updateRegistersSequence2(1<<sequence2FirstStep);
  sequence2Gate[sequence2FirstStep] = digitalRead(6);
  sequence2CV[sequence2FirstStep] = analogRead(14)>>4;
  for (uint8_t i = sequence2FirstStep+1; i <= sequence2LastStep; i++) {
    digitalWrite(3, 1);
    digitalWrite(5, 1);
    digitalWrite(3, 0);
    digitalWrite(5, 0);
    sequence2CV[i] = analogRead(14)>>4;
    sequence2Gate[i] = digitalRead(6);
  }
}
void updateRegistersSequence1(uint8_t value) {
  for (uint8_t i = 0; i < sequence1LastStep+1; i++) {
    digitalWrite(3, 0);
    digitalWrite(2, (value >> (sequence1LastStep - i)) & 1);
    digitalWrite(3, 1);
  }
  digitalWrite(4, 1);
  digitalWrite(4, 0);
  digitalWrite(3, 0);
  digitalWrite(2, 0);
}
void updateRegistersSequence2(uint8_t value) {
  for (uint8_t i = 0; i < sequence2LastStep+1; i++) {
    digitalWrite(3, 0);
    digitalWrite(2, (value >> (sequence2LastStep - i)) & 1);
    digitalWrite(3, 1);
  }
  digitalWrite(5, 1);
  digitalWrite(5, 0);
  digitalWrite(3, 0);
  digitalWrite(2, 0);
}

void stepp() {
  /*
  Serial.print(sequence1Step);
  Serial.print("   ");
  Serial.println(sequence1CV[sequence1Step]);
  */
  if (mainTempoStep < 7) {
    digitalWrite(13, 0);
    mainTempoStep++;
  } else {
    digitalWrite(13, 1);
    mainTempoStep = 0;
  }
  if (tempo1Step < tempo1StepMax) {
    tempo1StepOut = false;
    tempo1Step++;
  } else {
    tempo1StepOut = true;
    tempo1Step = 0;
  }
}
void sequence1NextStep() {
  if (sequence1Step < sequence1LastStep) {
    sequence1Step += 1;
  } else {
    sequence1Step = sequence1FirstStep;
  }
}
void sequence1Stepp() {
  if (sequence1SkipOffSteps) {
    for (int i = 0; i <= (sequence1LastStep-sequence1FirstStep); i++) {
      sequence1NextStep();
      if (sequence1Gate[sequence1Step]) {
        break;
      }
    }
  } else {
    sequence1NextStep();
  }
  sequence1CVOut = sequence1CV[sequence1Step];
  sequence1GateOut = sequence1Gate[sequence1Step];
  analogWrite(9, sequence1CVOut<<2);
  sequence1GateTime = 0;
}
