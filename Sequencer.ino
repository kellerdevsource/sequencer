
uint32_t newTime;
uint32_t tempoTime = 500000;
uint32_t controlTime = 2000;
uint32_t triggerTime;
bool triggered;

uint16_t controlStepMax = 3;
uint16_t controlStep;
volatile bool ctrl;
volatile bool ctrlFast;
uint16_t steppStepMax = 125; // The Tempo ~0-1024
uint16_t steppStep;

bool sequence1Forward = false;
bool sequence2Forward = false;

uint16_t sequence1GateTime;
uint16_t sequence1GateTimeMax =128; // Read from pot 0-255 : how long the gate is open withing a step
uint8_t tempo1Step;
volatile bool sequencer1Trigger;
volatile bool sequencer1Triggered;
uint8_t tempo1TimeMultiplier = 7;  // Read from pot 15-0: sequence 1 tempo, relative to the main tempo
volatile bool tempo1StepOut;
uint8_t sequence1Step;
uint8_t sequence1LastStep = 15; // Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1FirstStep = 0;// Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1CV;
bool sequence1SkipOffSteps = true;
bool sequence1Gate;
bool sequence1GateState;

uint16_t sequence2GateTime;
uint16_t sequence2GateTimeMax =128; // Read from pot 0-255 : how long the gate is open withing a step
uint8_t tempo2Step;
volatile bool sequencer2Trigger;
volatile bool sequencer2Triggered;
uint8_t tempo2TimeMultiplier = 3;  // Read from pot 15-0: sequence 1 tempo, relative to the main tempo
volatile bool tempo2StepOut;
uint8_t sequence2Step;
uint8_t sequence2LastStep = 15; // Read from pot 0-15 : how much of the sequence is played
uint8_t sequence2FirstStep = 0;// Read from pot 0-15 : how much of the sequence is played
uint8_t sequence2CV;
bool sequence2SkipOffSteps = true;
bool sequence2Gate;
bool sequence2GateState;

uint8_t mainTempoStep;




void setup() {
  cli();

  TCCR1B = TCCR1B & B11111000 | B00000001;

  sei();
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(20, INPUT);
  pinMode(13, OUTPUT);
  updateRegistersSequence1(0);
  Serial.begin(9600);
}


void loop() {
  newTime = micros();
  if (digitalRead(12) && !triggered) {
    if (triggerTime) {
      stepp();
      tempoTime = newTime - triggerTime;
    }
    triggerTime = newTime
    triggered = true;
  } else {
    tiggered = false;
  }

  uint32_t tempo1Scaler = tempoTime*((tempo1TimeMultiplier+1)/8);
  if (newTime> (triggerTime + tempo1Scaler)){
    sequence1Stepp();
  }
  uint32_t tempo1Scaler = tempoTime*((tempo2TimeMultiplier+1)/8);
  if (newTime> (triggerTime + tempo2Scaler)){
    sequence2Stepp();
  }
  
  if (newTime> (currentTime + controlTime)){
    currentTime = newTime;
    control();
  }
  
}

void control() {
  //Serial.println(sequence1Step);
  sequence1GateState = sequence1GateTime < map(sequence1GateTimeMax, 0, 255, 0, (steppStepMax*(1 + tempo1StepMax))/controlStepMax);
  sequence2GateState = sequence2GateTime < map(sequence2GateTimeMax, 0, 255, 0, (steppStepMax*(1 + tempo2StepMax))/controlStepMax);
  if (sequence1GateState) {
    sequence1GateTime++;
  } else {
    updateRegistersSequence1(0);
  }
  if (sequence2GateState) {
    sequence2GateTime++;
  } else {
    updateRegistersSequence2(0);
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

void updateRegistersSequence2(uint8_t value) {
  for (uint8_t i = 0; i < sequence1LastStep+1; i++) {
    digitalWrite(3, 0);
    digitalWrite(2, (value >> (sequence1LastStep - i)) & 1);
    digitalWrite(3, 1);
  }
  digitalWrite(5, 1);
  digitalWrite(5, 0);
  digitalWrite(3, 0);
  digitalWrite(2, 0);
  sequence2Gate = digitalRead(7);
}

void stepp() {
  if (mainTempoStep < 7) {
    digitalWrite(13, 0);
    mainTempoStep++;
  } else {
    digitalWrite(13, 1);
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
void sequence1Stepp() {
  if (sequence1SkipOffSteps) {
    for (uint8_t i = 0; i <= (sequence1LastStep-sequence1FirstStep); i++) {
      if (sequence1Forward) {
        sequence1NextStepForward();
      } else {
        sequence1NextStepBackword();
      }
      if (sequence1Gate) {
        break;
      }
    }
  } else {
      if (sequence1Forward) {
        sequence1NextStepForward();
      } else {
        sequence1NextStepBackword();
      }
  }
  sequence1CV = analogRead(20) >> 4;
  analogWrite(9, sequence1CV<<2);
  sequence1GateTime = 0;
}
void sequence2NextStepForward() {
  if (sequence2Step < sequence2LastStep) {
    sequence2Step += 1;
  } else {
    sequence2Step = sequence2FirstStep;
  }
  updateRegistersSequence2(1 << sequence2Step);
}
void sequence2NextStepBackword() {
  if (sequence2Step > sequence2FirstStep) {
    sequence2Step -= 1;
  } else {
    sequence2Step = sequence2LastStep;
  }
  updateRegistersSequence2(1 << sequence2Step);
}
void sequence2Stepp() {
  if (sequence2SkipOffSteps) {
    for (uint8_t i = 0; i <= (sequence2LastStep-sequence2FirstStep); i++) {
      if (sequence2Forward) {
        sequence2NextStepForward();
      } else {
        sequence2NextStepBackword();
      }
      if (sequence2Gate) {
        break;
      }
    }
  } else {
      if (sequence2Forward) {
        sequence2NextStepForward();
      } else {
        sequence2NextStepBackword();
      }
  }
  sequence2CV = analogRead(21) >> 4;
  analogWrite(10, 128);
  sequence2GateTime = 0;
}
