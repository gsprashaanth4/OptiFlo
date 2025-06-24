void RadioSetup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
  delay(500);
}


void RadioWrite() {
    /* Writnig an instance r1 of RadioPacket */
  radio.write(&r1, sizeof(r1));
}
