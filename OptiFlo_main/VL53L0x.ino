/* The functions for controlling the VL53L0x sensor */

void vlSetup() {
  if (!lox.begin()) {
    while(1);
  }
  lox.startRangeContinuous();
}


void vlMeasure() {
  if(lox.isRangeComplete()) {
    distance = lox.readRange();
    distance /= 10;
    distance -= 2;
  }
}