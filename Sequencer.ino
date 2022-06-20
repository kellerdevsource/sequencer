
uint16_t controlStepMax = 10;
uint16_t controlStep;
volatile bool ctrl;
volatile bool ctrlFast;
uint16_t steppStepMax = 125; // The Tempo ~0-1024
uint16_t steppStep;

uint8_t nextPot;
uint16_t sequence1GateTime;
uint16_t sequence1GateTimeMax =255; // Read from pot 0-255 : how long the gate is open withing a step
uint8_t tempo1Step;
volatile bool sequencer1Trigger;
volatile bool sequencer1Triggered;
uint8_t tempo1StepMax = 3;  // Read from pot 15-0: sequence 1 tempo, relative to the main tempo
volatile bool tempo1StepOut;
uint8_t sequence1Step;
uint8_t sequence1LastStep = 15; // Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1FirstStep = 0;// Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1CV;
bool sequence1Gate;
bool sequence1GateState;
uint8_t mainTempoStep;
bool sequence1SkipOffSteps = true;




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
  updateRegistersSequence1(0);
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
  ctrlFast = true;
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
  Serial.println(sequence1Step);
  sequence1GateState = sequence1GateTime < map(sequence1GateTimeMax, 0, 255, 0, (steppStepMax*(1 + tempo1StepMax))/controlStepMax);
  if (sequence1GateState) {
    sequence1GateTime++;
  } else {
    updateRegistersSequence1(0);
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
  sequence1Gate = digitalRead(6);
}
void stepp() {
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
  updateRegistersSequence1(1 << sequence1Step);
}
void sequence1Stepp() {
  if (sequence1SkipOffSteps) {
    for (uint8_t i = 0; i <= (sequence1LastStep-sequence1FirstStep); i++) {
      sequence1NextStep();
      if (sequence1Gate) {
        break;
      }
    }
  } else {
    sequence1NextStep();
  }
  sequence1CV = analogRead(14) >> 4;
  analogWrite(9, sequence1CV<<2);
  sequence1GateTime = 0;
}
