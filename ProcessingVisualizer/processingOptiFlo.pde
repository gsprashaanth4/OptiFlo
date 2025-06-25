import processing.serial.*;

Serial myPort;
String val;
float valX = 0.0;
float valY = 0.0;

float x = 0.0, y = 0.0;

void setup() {
  size(800, 600); // 2D canvas size
  smooth(); 
  fill(255, 100, 100); // Content color
  noStroke();
  
  String portName = "COM11";  // Change as needed
  myPort = new Serial(this, portName, 115200); // Baud rate to match with the one in the OptiFlo sketches
  delay(1000);
  background(0);
}

void draw() {

  // Read serial data
  if (myPort.available() > 0) {
    val = myPort.readStringUntil('\n');

    if (val != null) {
      try {
        // convert Serial data to floats
        val = trim(val);
        String[] valS = split(val, ',');
        if (valS.length >= 2) {
          // Storing the float values
          valX = Float.parseFloat(valS[0]);
          valY = Float.parseFloat(valS[1]);
        }
      } catch (Exception e) {
        println("Parsing error: " + e);
      }
    }
  }
  
  // Map valX and valY to screen position
  x = valY;
  y = valX;

  // Debug output
  println("X: " + x + ", Y: " + y);
  
  // Draw a circle at that position
  ellipse(width/2+x, height/2+y, 5, 5);
  
  // Reset the background when the sensor is reset
  if(x==0.0 && y==0.0) {
    background(0);
  }
  
}
