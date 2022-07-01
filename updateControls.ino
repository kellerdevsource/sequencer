uint8_t ctrlRegsOp;
bool funcButton;
bool funcSeq1;
bool funcSeq2;

void updateRegistersControls() {
  uint16_t potTempReading;
  bool sequence2PlayRead;
  switch (ctrlRegsOp) {
    case 0:
    PORTC|=ctrlRegsData;
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    PORTC&=~ctrlRegsData;
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    triggerStepMax = map(analogRead(14), 0, 1023, 700, 40); //30?-375 BPM
    break;
    case 1:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    if (digitalRead(14)){
      if (!funcButton) {
        mainTempoStep = 1;
        sequence1Step = sequence1FirstStep;
        sequence2Step = sequence2FirstStep;
        repeats = 0;
        funcButton = true;
      }
    } else {
      funcButton = false;
    }
    break;
    case 2:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    if (digitalRead(14)) {
      if (!funcSeq1) {
        funcSeq1 = true;
        sequence1Stepp();
      }
    } else {
      funcSeq1 = false;
    }
    break;
    case 3:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence1Play = digitalRead(14);
    break;
    case 4:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence1TempoStepMax = 1 << (map(analogRead(14), 0, 1023, 5, 0));
    break;
    case 5:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence1Type = map(analogRead(14), 0, 1023, 0, 4);
    break;
    case 6:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    potTempReading = analogRead(14)>>6;
    if (sequence1FirstStep<potTempReading) {
      sequence1LastStep = potTempReading;
    } else {
      sequence1LastStep = sequence1FirstStep;
    }
    break;
    case 7:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    potTempReading = analogRead(14);
    potTempReading = potTempReading>>6;
    if (sequence1LastStep>potTempReading) {
      sequence1FirstStep = potTempReading;
    } else {
      sequence1FirstStep = sequence1LastStep;
    }
    break;
    case 8:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    if (digitalRead(14)) {
      if (!funcSeq2) {
        funcSeq2 = true;
        sequence2Stepp();
      }
    } else {
      funcSeq2 = false;
    }
    break;
    case 9:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence2PlayRead = digitalRead(14);
    if (!sequence2PlayRead){
        if (!sequence2LastStep) {
         repeat = true;
        }
        sequence2Play = false;
    } else {
      if (repeat){
        repeat = false;
        sequence1GateTimeMax = 127;
      }
      sequence2Play = true;
    }
    break;
    case 10:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence2TempoStepMax = 1 << (map(analogRead(14), 0, 1023, 5, 0));//Sequence 1 trigger divider
    break;
    case 11:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence2Type = map(analogRead(14), 0, 1023, 0, 4);
    break;
    case 12:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    potTempReading = analogRead(14)>>6;
    if (sequence2FirstStep<potTempReading) {
      sequence2LastStep = potTempReading;
    } else {
      sequence2LastStep = sequence2FirstStep;
    }
    break;
    case 13:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    potTempReading = analogRead(14);
    if (repeat){
      sequence1GateTimeMax = potTempReading>>2;
    } else {
      potTempReading = potTempReading>>6;
      if (sequence2LastStep>potTempReading) {
        sequence2FirstStep = potTempReading;
      } else {
        sequence2FirstStep = sequence2LastStep;
      }
    }
    break;
    case 14:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequenceScale = analogRead(14)>>8;
    break;
  }
  if (ctrlRegsOp<14) {
    ctrlRegsOp++;
  } else {
    ctrlRegsOp = 0;
  }
}
