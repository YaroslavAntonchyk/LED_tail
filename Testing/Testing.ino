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
const int ledR = 9;
const int ledG = 11;
const int ledB = 10;

constexpr float slowCoef = 0.02;
constexpr float fastCoef = 0.3;
constexpr float inCoef = 1.5;
constexpr float outCoef = 1;


String inputString = "";         // a String to hold incoming data
volatile bool stringComplete = false;  // whether the string is complete

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

void setup() 
{
  Serial.begin(115200); delay(10);
  Serial.println("Starting...");

  LoadCell.begin();
//  LoadCell.setReverseOutput();
  LoadCell.start(2000, true);

  if (LoadCell.getTareTimeoutFlag()) 
  {
    Serial.println("ERROR, Timeout");
  }
  else 
  {
    LoadCell.setCalFactor(1); // set calibration factor (float)
    while (!LoadCell.update());
    Serial.println("Startup is complete");
  }

  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
}

void calibrateLoadCells()
{
  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) 
  {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) 
  {
//    Serial.println("Tare complete");
  }
}

class Sensor
{
public:
  Sensor(const float _inCoef, const float _outCoef, const float _slowCoef, const float _fastCoef): 
    m_inCoef(_inCoef),
    m_outCoef(_outCoef),
    m_slowCoef(_slowCoef),
    m_fastCoef(_fastCoef),
    m_slowFilter(0.0),
    m_fastFilter(0.0),
    m_noiseLevel(0.0),
    m_detected(false)
  {
    m_noiseLevel = 700.0;
  }

  bool detect(float _sample)
  {
    m_fastFilter = m_fastFilter*(1-m_fastCoef) + _sample*m_fastCoef;
    float inThreshold = (abs(m_slowFilter) + abs(m_noiseLevel*m_inCoef));
    float outThreshold = (abs(m_slowFilter) + abs(m_noiseLevel*m_outCoef));
    if (m_fastFilter > inThreshold)
      m_detected = true;
    if (m_fastFilter < outThreshold + m_noiseLevel)
    {
      m_slowFilter = m_slowFilter*(1-m_slowCoef) + _sample*m_slowCoef;
      m_detected = false;
    }
    return m_detected;
  }
  
  bool detectH(float _sample)
  {
    const int hysteresisStep = 1000;
    m_fastFilter = m_fastFilter*(1-m_fastCoef) + _sample*m_fastCoef;
    float inThreshold = (abs(m_slowFilter + hysteresisStep*m_inCoef) + abs(m_noiseLevel));
    float outThreshold = (abs(m_slowFilter + hysteresisStep*m_outCoef) + abs(m_noiseLevel));
    if (m_fastFilter > inThreshold)
      m_detected = true;
    if (m_fastFilter < outThreshold)
    {
      m_slowFilter = m_slowFilter*(1-m_slowCoef) + _sample*m_slowCoef;
      m_detected = false;
    }
    return m_detected;
  }

  const float m_inCoef;
  const float m_outCoef;
  const float m_slowCoef;
  const float m_fastCoef;
  float m_slowFilter;
  float m_fastFilter;
  float m_noiseLevel;
  bool m_detected;
};

void setLedColorRGB(int r, int g, int b)
{
  r = constrain(r, 0, 255);
  g = constrain(g, 0, 255);
  b = constrain(b, 0, 255);
  analogWrite(ledR, r);
  analogWrite(ledG, g);
  analogWrite(ledB, b);
}

void loop() 
{
  static boolean newDataReady = 0;
  const int serialPrintInterval = 5; //increase value to slow down serial print activity
  static unsigned long t = 0;
//  static Sensor s(inCoef, outCoef, slowCoef, fastCoef);
  static Sensor s(7, 10, slowCoef, fastCoef);
  static int ledR = 0;
  if (stringComplete)
  {
    ledR = extractValue(inputString);
    stringComplete = false;
  }
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  if (newDataReady) 
  {
      float sample = abs(LoadCell.getData());

      if (s.detectH(sample))
      {
        setLedColorRGB(ledR, 128, 128);
//        Serial.println("Detected");
      }
      else 
      {
        setLedColorRGB(0, 0, 0);
      }
      
//      Serial.print(s.m_fastFilter);
//      Serial.print(',');
//      Serial.println(s.m_slowFilter);
      newDataReady = false;
      t = millis();
  }

//  calibrateLoadCells();
}


/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() 
{
  while (Serial.available()) 
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') 
    {
      stringComplete = true;
    }
  }
}

int extractValue(String str)
{
  int num = 0;
  char symbol = str.charAt(0);
  for (int i = 1; isDigit(symbol); i++)
  {
    num = num*10 + (int)(symbol-48); //convert ASCII to digit
    symbol = str.charAt(i);
  }
  return num;
}
