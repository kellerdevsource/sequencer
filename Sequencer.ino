
uint32_t currentTime;
uint32_t newTime;
uint32_t controlTime = 5;
uint16_t maxControlSteps = 20; // Read from pot 32-0  : the tempo
uint16_t controlStep;
uint16_t sequence1GateTime;
uint16_t sequence1GateTimeMax = 150; // Read from pot 0-255 : how long the gate is open withing a step
uint8_t sequence1Divider = 0; // Read from pot 0-16 : multiplier of the speed of the sequence, compared to the main tempo
uint8_t sequence1DividerStep;
uint8_t sequence1Step;
uint8_t sequence1LastStep = 15; // Read from pot 0-15 : how much of the sequence is played

bool sequence1StepStates[] = {1,1,0,1,0,1,1,0,0,1,0,1,1,0,1,0};

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  //Serial.println("loop");
  newTime = millis();
    if (newTime > (currentTime + controlTime)) {
    currentTime = newTime;
      control();
  }
}

void control() {
  digitalWrite(13, (sequence1StepStates[sequence1Step] && (sequence1GateTime < map(sequence1GateTimeMax, 0, 255, 0, maxControlSteps*(1 + sequence1Divider)))));
  sequence1GateTime++;
  if (controlStep < maxControlSteps) {
    controlStep++;
  } else {
    stepp();
    controlStep = 0;
  }
}

void stepp() {
  if (sequence1DividerStep < sequence1Divider) {
    sequence1DividerStep += 1;
  } else {
    sequence1DividerStep = 0;
    if (sequence1Step < sequence1LastStep) {
      sequence1Step += 1;
      sequence1GateTime = 0;
      //digitalWrite(13, sequence1StepStates[sequence1Step]);
    } else {
      sequence1Step = 0;
    }
  }
}
