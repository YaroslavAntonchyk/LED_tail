/*
   -------------------------------------------------------------------------------------
   HX711_ADC
   Arduino library for HX711 24-Bit Analog-to-Digital Converter for Weight Scales
   Olav Kallhovd sept2017
   -------------------------------------------------------------------------------------
*/

/*
   Settling time (number of samples) and data filtering can be adjusted in the config.h file
   For calibration and storing the calibration value in eeprom, see example file "Calibration.ino"

   The update() function checks for new data and starts the next conversion. In order to acheive maximum effective
   sample rate, update() should be called at least as often as the HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS.
   If you have other time consuming code running (i.e. a graphical LCD), consider calling update() from an interrupt routine,
   see example file "Read_1x_load_cell_interrupt_driven.ino".

   This is an example sketch on how to use this library
*/

#include "HX711_ADC.h"

//pins:
const int HX711_dout = 6; //mcu > HX711 dout pin
const int HX711_sck = 7; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

unsigned long t = 0;

void setup() {
  Serial.begin(115200); delay(10);
  //.println();
  Serial.println("Starting...");

  float calibrationValue = 1; // calibration value
  // calibrationValue = 696.0; // uncomment this if you want to set this value in the sketch

  LoadCell.begin();
//  LoadCell.setReverseOutput();
  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
 if (LoadCell.getTareTimeoutFlag()) 
 {
   Serial.println("ERROR, Timeout");
 }
 else 
 {
   LoadCell.setCalFactor(calibrationValue); // set calibration factor (float)
   while (!LoadCell.update());
   Serial.println("Startup is complete");
 }
}

void loop() 
{
  static boolean newDataReady = 0;
  const int serialPrintInterval = 5; //increase value to slow down serial print activity
  static bool detected = false;
  static float slowFilter = 0;
  static float fastFilter = 0;
  const float slowCoef = 0.04;
  const float fastCoef = 0.3;
  static float prevEmpty = 0;
  const float noiseLevel = 450;
  const float inCoef = 1.5;
  const float outCoef = 1;

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) 
  {
    if (millis() > t + serialPrintInterval) 
    {
      float sample = LoadCell.getData();
      fastFilter = fastFilter*(1-fastCoef) + sample*fastCoef;
//      slowFilter = slowFilter*(1-slowCoef) + sample*slowCoef;
      float inThreshold = (abs(slowFilter) + abs(noiseLevel))*inCoef;
      float outThreshold = (abs(slowFilter) + abs(noiseLevel))*outCoef;
      if (fastFilter > inThreshold)
        detected = true;
      if (fastFilter < outThreshold)
      {
        slowFilter = slowFilter*(1-slowCoef) + sample*slowCoef;
//        Serial.println("slow filter");
        detected = false;
      }

      if (detected)
      {
        Serial.println(fastFilter);
      }
      newDataReady = 0;
      t = millis();
    }
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) 
  {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) 
  {
    //Serial.println("Tare complete");
  }

}
