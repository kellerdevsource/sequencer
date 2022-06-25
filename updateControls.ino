uint8_t ctrlRegsOp;
bool funcButton;
bool funcSeq1;
bool funcSeq2;

void updateRegistersControls() {
  uint8_t potTempReading;
  switch (ctrlRegsOp) {
    case 0:
    PORTC|=ctrlRegsData;
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    PORTC&=~ctrlRegsData;
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    triggerStepMax = map(analogRead(14), 0, 1023, 250, 80); //60?-187.5 BPM
    break;
    case 1:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    if (digitalRead(14)){
      if (!funcButton) {
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
        if (!funcButton) {
          if (sequence1Type<2){
            sequence1Type ++;
          } else {
            sequence1Type = 0;
          }
        } else {
          if (sequence1Dir<2){
            sequence1Dir ++;
          } else {
            sequence1Dir = 0;
          }
        }
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
    sequence1TempoStepMax = 1 << (map(analogRead(14), 0, 1023, 5, 0));//Sequence 1 trigger divider
    break;
    case 5:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence1GateTimeMax = analogRead(14)>>2;
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
    potTempReading = analogRead(14)>>6;
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
        if (!funcButton) {
          if (sequence2Type<2){
            sequence2Type ++;
          } else {
            sequence2Type = 0;
          }
        } else {
          if (sequence2Dir<2){
            sequence2Dir ++;
          } else {
            sequence2Dir = 0;
          }
        }
      }
    } else {
      funcSeq2 = false;
    }
    break;
    case 9:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence2Play = digitalRead(14);
    break;
    case 10:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence2TempoStepMax = 1 << (map(analogRead(14), 0, 1023, 5, 0));//Sequence 1 trigger divider
    break;
    case 11:
    PORTC|=ctrlRegsCLK;
    PORTC&=~ctrlRegsCLK;
    sequence2GateTimeMax = analogRead(14)>>2;
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
    potTempReading = analogRead(14)>>6;
    if (sequence2LastStep>potTempReading) {
      sequence2FirstStep = potTempReading;
    } else {
      sequence2FirstStep = sequence2LastStep;
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
