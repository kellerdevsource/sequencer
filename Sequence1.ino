void updateRegistersSequence1() {
  uint16_t value = 15 - sequence1Step;
  SPI.transfer16(1<<value); 
  PORTB |= ssSequence1;
  PORTB &= ~ssSequence1;
  if (repeat) {
    PORTB |= ssSequence2;
    PORTB &= ~ssSequence2;
    delayMicroseconds(5);
    sequence1Gate =  PIND&gateInSeq1;
    if (sequence1Gate) {
      repeatsMax = (map(analogRead(21), 0, 1023, 0, 7));
      repeatGate = PIND&gateInSeq2;
    } else {
      if (sequence1Type != 1) {
        repeatsMax = (map(analogRead(21), 0, 1023, 0, 7));
      }
      repeatGate = 0;
    }
  } else {
    delayMicroseconds(5);
    sequence1Gate =  PIND&gateInSeq1;
  }
}

void sequence1Stepp() {
    sequence1GateTime = 0;
    if (repeat){
      if (repeats<repeatsMax){
        repeats++;
        if (repeatGate){
          sequence1Gate = true;
          PORTD |= gateOutSeq1;
        }
      }  else {
        repeats = 0;
        sequence1NextStep();
      }
    } else {
      sequence1NextStep();
    }
}
void sequence1NextStep() {
      switch (sequence1Type) {
        case 0:
          sequence1NextStepForward();
          updateRegistersSequence1();
        break;
        case 1:
          for (uint8_t i = 0; i < 16; i++) {
            sequence1NextStepForward();
            updateRegistersSequence1();
            if (sequence1Gate) {
              break;
            }
          }
        break;
        case 2:
          if (pingPongSeq1Dir) {
            sequence1NextStepForward();
          } else {
            sequence1NextStepBackword();
          }
          updateRegistersSequence1();
        break;
        case 3:
          sequence1NextStepBackword();
          updateRegistersSequence1();
        break;
        case 4:
          sequence1Step = rand()%(sequence1LastStep-sequence1FirstStep+1) + sequence1FirstStep;
          updateRegistersSequence1();
        break;
      }
      if (sequence1Gate) {
        sequence1UpdateNote();
      } else {
        PORTD &= ~gateOutSeq1;
      }
}
void sequence1NextStepForward() {
  if (sequence1Step < sequence1LastStep) {
    sequence1Step += 1;
  } else {
    if (sequence1Type != 2) {
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
    if (sequence1Type != 2) {
      sequence1Step = sequence1LastStep;
    } else {
      sequence1Step += 1;
      pingPongSeq1Dir = !pingPongSeq1Dir;
    }
  }
}

void sequence1UpdateNote() {
  PORTD |= gateOutSeq1;
  sequence1CV = map(analogRead(20), 0, 1023, 0, 60);//(analogRead(20) >> 4);
  switch (sequenceScale) {
    case 0:
      sequence1Note = minorScale[sequence1CV % 12];
      sequence1CV = sequence1Note + ((sequence1CV / 12) * 12);
      break;
    case 2:
      sequence1Note = SixSemiScale[sequence1CV % 12];
      sequence1CV = sequence1Note + ((sequence1CV / 12) * 12);
      break;
    case 3:
      sequence1Note = majorScale[sequence1CV % 12];
      sequence1CV = sequence1Note + ((sequence1CV / 12) * 12);
      break;
  }
  sequence1CV =  (sequence1CV) << 2;
  OCR0B = sequence1CV;
}
