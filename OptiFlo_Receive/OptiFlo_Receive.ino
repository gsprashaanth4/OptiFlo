/* This is the Program to be run on the arduino connected
   to the computer when the sensor operates in wireless mode,
   to receive the incoming data, it sends it out back to the 
   Processing sketch */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN

const byte address[6] = "00001";

/* initiating the receiving values */
float posx = 0.0, posy = 0.0;

/* Radio packet defined as same as the transmitter */
struct RadioPacket // Any packet up to 32 bytes can be sent.
{
    float dx;
    float dy;
};

RadioPacket r1;

void setup() {
  Serial.begin(115200); // this should match with Processing
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW); // modifiable on both receiver and transmitter
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    /* reading the incoming data */
    radio.read(&r1, sizeof(r1));

    /* Printing the received data serially, for the Processing sketch to read */
    Serial.print(r1.dx);
    Serial.print(",");
    Serial.println(r1.dy);
  }
}