
bool triggered;
uint16_t triggerStepMax = 250; // The Tempo ~0-1024
uint16_t triggerStep;
uint16_t controlStepMax = 3;
uint16_t controlStep;

volatile bool ctrl;
volatile bool ctrlFast;


uint8_t mainTempoStep;

bool sequence1Forward = true;
bool sequence1GateTimer;
bool sequence1SkipOffSteps = true;
bool sequence1Gate;
uint16_t sequence1GateTime;
uint16_t sequence1GateTimeMax =2; // Read from pot 0-255 : how long the gate is open withing a step
uint8_t sequence1TempoStepMax = 2;  // Read from pot 63-0: sequence 1 tempo, relative to the main tempo
uint8_t sequence1Step;
uint8_t sequence1LastStep = 15; // Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1FirstStep = 0;// Read from pot 0-15 : how much of the sequence is played
uint8_t sequence1CV;




void setup() {
  analogReference(EXTERNAL);
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
  pinMode(6, INPUT);
  pinMode(12, INPUT);
  pinMode(14, INPUT);
  pinMode(15, INPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);

  
  updateRegistersControls();
  updateRegistersSequence1(0);
  sequence1Step = sequence1LastStep;
  
  Serial.begin(9600);
}

ISR(TIMER2_COMPA_vect) {
  if (triggerStep < triggerStepMax) {
    if (triggerStep>triggerStepMax/2) {
      digitalWrite(11,0);
    }
    triggerStep++;
  } else {
    digitalWrite(11,1);
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
  if (digitalRead(12)) {
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
  sequence1GateTimer = sequence1GateTime < map(sequence1GateTimeMax, 0, 255, 0, (triggerStepMax*(1 + sequence1TempoStepMax))/controlStepMax);
  //Serial.println(sequence1GateState);
  if (sequence1GateTimer) {
    sequence1GateTime++;
  } else {
    digitalWrite(8, 0);
  }
}
void updateRegistersControls() {
  triggerStepMax = map(analogRead(14), 0, 1023, 250, 80); //60-187.5 BPM
  //sequence1TempoStepMax = (1<<(map(analogRead(15), 0, 1023, 6, 0)));
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

void triggerStepp() {

  if (!(mainTempoStep%8)) {
    //digitalWrite(13, 1);
  } else {
    //digitalWrite(13, 0);
  }
  if (!(mainTempoStep%sequence1TempoStepMax)) {
    sequence1Stepp();
  }
  if (mainTempoStep < 63) {
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
void sequence1UpdateNote(){
  sequence1CV = analogRead(20) >> 5;
  digitalWrite(8, 1);
  analogWrite(9, sequence1CV<<3); 
}
void sequence1Stepp() {
  //Serial.println(mainTempoStep);
  if (sequence1SkipOffSteps) {
    for (uint8_t i = 0; i <= (sequence1LastStep-sequence1FirstStep); i++) {
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
     }
  }
  sequence1GateTime = 0;
}
