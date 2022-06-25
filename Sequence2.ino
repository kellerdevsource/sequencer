
void updateRegistersSequence2() {
  uint16_t value = 15 - sequence2Step;
  SPI.transfer16(1<<value); 
  PORTB |= ssSequence2;
  PORTB &= ~ssSequence2;
  delayMicroseconds(5);
  sequence2Gate =  PIND&gateInSeq2;
}

void sequence2Stepp() {
    sequence2GateTime = 0;
    sequence2NextStep();
    if (sequence2Gate) {
      sequence2UpdateNote();
    } else {
      PORTD &= ~gateOutSeq2;
    }
}
void sequence2NextStep() {
      switch (sequence2Type) {
        case 0:
          switch (sequence2Dir) {
            case 0:
            sequence2NextStepForward();
            break;
            case 1:
            sequence2NextStepBackword();
            break;
            case 2:
            if (pingPongSeq2Dir) {
              sequence2NextStepForward();
            } else {
              sequence2NextStepBackword();
            }
            break;
          }
          updateRegistersSequence2();
        break;
        case 1:
          switch (sequence2Dir) {
            case 0:
            for (uint8_t i = 0; i < 16; i++) {
              sequence2NextStepForward();
              updateRegistersSequence2();
              if (sequence2Gate) {
                break;
              }
            }
            break;
            case 1:
            for (uint8_t i = 0; i < 16; i++) {
              sequence2NextStepBackword();
              updateRegistersSequence2();
              if (sequence2Gate) {
                break;
              }
            }
            break;
            case 2:
            for (uint8_t i = 0; i < 32; i++) {
              if (pingPongSeq2Dir) {
                sequence2NextStepForward();
              } else  {
                sequence2NextStepBackword();
              }
              updateRegistersSequence2();
              if (sequence2Gate && sequence2Step != sequence2LastOnStep) {
                sequence2LastOnStep = sequence2Step;
                break;
              }
            }
            break;
          }
        break;
        case 2:
          sequence2Step = rand()%(sequence2LastStep-sequence2FirstStep+1) + sequence2FirstStep;
          updateRegistersSequence2();
        break;
      }
}
void sequence2NextStepForward() {
  if (sequence2Step < sequence2LastStep) {
    sequence2Step += 1;
  } else {
    if (sequence2Dir != 2) {
      sequence2Step = sequence2FirstStep;
    } else {
      sequence2Step -= 1;
      pingPongSeq2Dir = !pingPongSeq2Dir;
    }
  }
}
void sequence2NextStepBackword() {
  if (sequence2Step > sequence2FirstStep) {
    sequence2Step -= 1;
  } else {
    if (sequence2Dir != 2) {
      sequence2Step = sequence2LastStep;
    } else {
      sequence2Step += 1;
      pingPongSeq2Dir = !pingPongSeq2Dir;
    }
  }
}

void sequence2UpdateNote() {
  PORTD |= gateOutSeq2;
  sequence2CV = map(analogRead(21), 0, 1023, 0, 60);//(analogRead(20) >> 4);

  switch (sequenceScale) {
    case 0:
      sequence2Note = minorScale[sequence2CV % 12];
      sequence2CV = sequence2Note + ((sequence2CV / 12) * 12);
      break;
    case 2:
      sequence2Note = SixSemiScale[sequence2CV % 12];
      sequence2CV = sequence2Note + ((sequence2CV / 12) * 12);
      break;
    case 3:
      sequence2Note = majorScale[sequence2CV % 12];
      sequence2CV = sequence2Note + ((sequence2CV / 12) * 12);
      break;
  }
  sequence2CV =  (sequence2CV) << 2;
  OCR0A = sequence2CV;
}
