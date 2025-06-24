/* At various points of the program, the three modes of the package will be mentioned,
   namely, Wired Mode (optical Flow data), Wireless Mode (optical Flow data), 
   Visualizer Mode (Wired transmission of image data to python visualizer) */

/* High-Speed GPIO_port reading function to get data from the camera */
/* there are various other GPIO ports, like port 7 for example that could be used based on need*/
inline byte readPixel() {
  uint32_t gpioState = GPIO6_PSR;
  byte pixD = 0;
  pixD |= ((gpioState >> 3) & 0x01) << 0;   // Pin 0
  pixD |= ((gpioState >> 18) & 0x01) << 1;  // Pin 14
  pixD |= ((gpioState >> 19) & 0x01) << 2;  // Pin 15
  pixD |= ((gpioState >> 22) & 0x01) << 3;  // Pin 17
  pixD |= ((gpioState >> 23) & 0x01) << 4;  // Pin 16
  pixD |= ((gpioState >> 24) & 0x01) << 5;  // Pin 22
  pixD |= ((gpioState >> 25) & 0x01) << 6;  // Pin 23
  pixD |= ((gpioState >> 26) & 0x01) << 7;  // Pin 20
  return pixD;
}


/* The function to write the SCCB settings of the camera */
void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(OV7670_ADDR >> 1);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}


void CameraSetupBasic() {

  /* Pinmode Settings */
  pinMode(PCLK_PIN, INPUT);
  pinMode(VSYNC_PIN, INPUT);
  pinMode(HREF_PIN, INPUT);
  pinMode(D0_PIN, INPUT);
  pinMode(D1_PIN, INPUT);
  pinMode(D2_PIN, INPUT);
  pinMode(D3_PIN, INPUT);
  pinMode(D4_PIN, INPUT);
  pinMode(D5_PIN, INPUT);
  pinMode(D6_PIN, INPUT);
  pinMode(D7_PIN, INPUT);
  pinMode(RESET, OUTPUT);

  /* Enabling the camera reset */
  digitalWrite(RESET, LOW);
  delay(500);

  /* The external clock supplied to the OV7670 */
  /* should support upto 48MHz, based on the wiring */
  analogWriteFrequency(XCLK_PIN, 13000000);
  analogWrite(XCLK_PIN, 128);  // 50% duty cycle
  delay(100);

  /* Releasing reset on the camera */
  digitalWrite(RESET, HIGH);
  delay(100);
}


/* Writing the SCCB settings */
void CameraSetupSCCB() {
  writeReg(0x12, 0x80);  // Reset to default values also for output format
  delay(100);

  writeReg(0x6B, 0x83);  // Prescaler
  writeReg(0x11, 0x00);  // Internal Clock

  writeReg(0x3B, 0x0A);
  writeReg(0x3A, 0x04);
  writeReg(0x12, 0x04);  // Output format: rgb
  writeReg(0x8C, 0x00);  // Disable RGB444
  writeReg(0x40, 0xD0);  // Set RGB565

  writeReg(0x17, 0x16);  // HSTART
  writeReg(0x18, 0x04);  // HSTOP
  writeReg(0x32, 0x24);  // HREF
  writeReg(0x19, 0x02);  // VSTART
  writeReg(0x1A, 0x7A);  // VSTOP
  writeReg(0x03, 0x0A);  // VREF

  writeReg(0x15, 0x02);  // VSYNC NEGATED, HIGH is VALID

  writeReg(0x0C, 0x04);  // Enable Scaling
  writeReg(0x3E, 0x1A);  // Divide by 4
  writeReg(0x1E, 0x27);
  writeReg(0x72, 0x22);  // Downsample by 4
  writeReg(0x73, 0xF2);  // Divide by 4

  writeReg(0x4F, 0x80);
  writeReg(0x50, 0x80);
  writeReg(0x51, 0x00);
  writeReg(0x52, 0x22);
  writeReg(0x53, 0x5E);
  writeReg(0x54, 0x80);
  writeReg(0x56, 0x40);
  writeReg(0x58, 0x9E);
  writeReg(0x59, 0x88);
  writeReg(0x5A, 0x88);
  writeReg(0x5B, 0x44);
  writeReg(0x5C, 0x67);
  writeReg(0x5D, 0x49);
  writeReg(0x5E, 0x0E);
  writeReg(0x69, 0x00);
  writeReg(0x6A, 0x40);

  writeReg(0x6C, 0x0A);
  writeReg(0x6D, 0x55);
  writeReg(0x6E, 0x11);
  writeReg(0x6F, 0x9F);
  writeReg(0xB0, 0x84);
  writeReg(0x55, 0x00);  // Brightness
  writeReg(0x56, 0x40);  // Contrast
  writeReg(0xFF, 0xFF);  // End marker
}


/* Setting up the interrupt function for the camera */
void CameraSetupInterrupt() {
  attachInterrupt(digitalPinToInterrupt(VSYNC_PIN), doThis, RISING);
  delay(100);
}


void CaptureCalculate() {

  if (!frameAvailable || frameCapture) return;
  frameCapture = true;

  /* variable initiation */
  int dots = 0;
  uint8_t img[CameraWidth * CameraHeight]; // array to store image data from camera
  int lines = 0; // state variable used during capture


  /* Making sure the while loop only sustains until the CameraHeight resolution is covered */
  while (digitalReadFast(VSYNC_PIN) && (lines < CameraHeight)) {

    while (digitalReadFast(VSYNC_PIN) && ! digitalReadFast(HREF_PIN)); // Wait for a line to start
    if (! digitalReadFast(VSYNC_PIN)) {
      break;  // Line didn't start, but frame ended
    }
    

    while (digitalReadFast(HREF_PIN)) {

      /* The chosen output format is RGB565, as it offered 
      the highest observed speed and carried the highest detail
      compared to other formats
      
      This format sends out 2 bytes per pixel, which is read
      and stored by one uint16_t space in the img array
      over two PCLK cycles */

      while (!digitalReadFast(PCLK_PIN));
      uint16_t pixel = (readPixel()) << 8;
      while (digitalReadFast(PCLK_PIN));

      while (!digitalReadFast(PCLK_PIN));
      pixel |= readPixel();
      uint8_t r5 = (pixel >> 11) & 0x1F;
      uint8_t g6 = (pixel >> 5) & 0x3F;
      uint8_t b5 = pixel & 0x1F;

      while (digitalReadFast(PCLK_PIN));

      uint8_t r = (r5 * 527 + 23) >> 6;  // ~255/31
      uint8_t g = (g6 * 259 + 33) >> 6;  // ~255/63
      uint8_t b = (b5 * 527 + 23) >> 6;  // ~255/31

      /* Integer grayscale conversion (no float) */
      img[lines * CameraWidth + dots] = (uint8_t)((r * 77 + g * 150 + b * 29) >> 8);
      dots++;
    }
    /* incrementing state variable */
    lines++;
    dots = 0;
  }


  /* pre-cropping the image into windoWidth*windoHeight from CameraWidth*CameraHeight resolution*/
  int k = 0;
  for( int y = startCol; y < startCol+windoHeight; y++) {
    for( int x = startRow; x < startRow+windoWidth; x++) {
      imgF[k++] = img[y * CameraWidth + x];
    }
  }


  /* waiting for atleast two frames before working out displacement */
  if (!firstFrame) {
    
    /* Formula to work out real world displacement */
    /* from obtained pixel displacement */
    realFactorX = (2 * distance * tan(0.10567)) / windoWidth;
    realFactorY = (2 * distance * tan(0.10567)) / windoHeight;

    /* calling of the optical-flow function */
    estimateMotionSSD(dx, dy);

    /* a count variable is added to run 10
    iterations before starting displacement computation
    to smooth out any jitter during the start */

    if (cnt < 10) cnt++;

    if (cnt == 10) {
      r1.dx += dx * realFactorX;
      r1.dy += dy * realFactorY;
    }
  }


  /* copying current image to previous image for next SSD iteration */
  memcpy(imgFprev, imgF, windoWidth * windoHeight);

  /* Output based on Mode Selected */
  if (MODE == "WIRELESS_DATA" || MODE == "WIRELESS") {
    RadioWrite();
  }
  else if (MODE == "WIRED_DATA") {
    Serial.print(r1.dx);
    Serial.print(",");
    Serial.print(r1.dy);
    Serial.print("   ");
    Serial.print(distance);
  }
  else if (MODE == "WIRED_VISUAL") {
    Serial.print(r1.dx);
    Serial.print(",");
    Serial.println(r1.dy);
  }
  else if (MODE == "WIRED_IMG") {
    Serial.write(imgF, windoWidth*windoHeight);
  }


  /* second time-stamp, after image created */
  unsigned long time1 = micros();

  unsigned long time = time1 - time0;
  float fps = 1000000.0f / (float)(time);  // FPS calculated

  if(MODE == "WIRELESS_DATA" || MODE == "WIRED_DATA") {
    Serial.print("   Effective FPS: ");
    Serial.println(fps, 10);
  }

  /* state variables reset */
  firstFrame = false;
  frameAvailable = false;
  frameCapture = false;
}


/* interrupt function, controls variable that specifies image readiness */
inline void doThis() {
  frameAvailable = true;
}


/* function to call to perform SSD - optical flow */
void estimateMotionSSD(int &bestDx, int &bestDy) {

  const int searchRadius = 10;  // how far to look for motion in each direction, d(Pixel) belongs to [0,searchRadius]
  long minSSD = LONG_MAX; // initiating the minSSD value to the maximum possible

  /* Looping through and selecting the offsets, which is used to select the candidate block from the previous image */
  for (int dy = -searchRadius; dy <= searchRadius; dy++) {
    for (int dx = -searchRadius; dx <= searchRadius; dx++) {
      long ssd = 0; // reset the value for every new offset block

      /* Loop over all pixels of image, Excluding the search-radius and preparing the pixels of central block and 
         preparing the pixels of the Candidate block from the previous image for SSD */
      for (int y = searchRadius; y < windoHeight - searchRadius; y++) {
        for (int x = searchRadius; x < windoWidth - searchRadius; x++) {

          /* initiating variables to access the current pixel of the current candidate 
             block by inducing the offset relative to the current block's pixel */
          int newX = x + dx;
          int newY = y + dy;

          /* safety checks, making sure no 'out of bounds' */
          if (newX < 0 || newX >= windoWidth || newY < 0 || newY >= windoHeight) continue;

          /* performing SSD */
          int curr = imgF[y * windoWidth + x]; // The current pixel of the central Block
          int prev = imgFprev[newY * windoWidth + newX]; // The current pixel of the current Candidate Block with
          int diff = (curr - prev); // difference between them
          ssd += diff * diff; // square and add them, until all pixels of the both blocks are done, 
                              // after which the process is repeated with the next candidate block.
        }
      }
      
      /* determining displacement based on likeness from the SSD */
      if (ssd < minSSD) {
        minSSD = ssd;
        bestDx = dx;
        bestDy = dy;
      }
    }
  }
}
