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
#include "FastLED.h"

//pins:
const int HX711_dout = 6; //mcu > HX711 dout pin
const int HX711_sck = 7; //mcu > HX711 sck pin
const int LED_PIN = 2;
const int LED_NUM = 60;

constexpr float slowCoef = 0.02;
constexpr float fastCoef = 0.3;
constexpr float inCoef = 1.5;
constexpr float outCoef = 1;

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

CRGB leds[LED_NUM];



void setup() {
  Serial.begin(115200); delay(10);
  Serial.println("Starting...");

  FastLED.addLeds<WS2852, LED_PIN, GRB>(leds, LED_NUM);
  FastLED.setBrightness(100);

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
    Serial.println("Tare complete");
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

void setLedColor(int r, int g, int b)
{
  for(int k = 0; k < LED_NUM; k++)
    leds[k] = CRGB(r, g, b); 
  FastLED.show();
}

void loop() 
{
  static boolean newDataReady = 0;
  const int serialPrintInterval = 5; //increase value to slow down serial print activity
  const int tareInterval = 200;
  static int tareTimer = 0;
  static unsigned long t = 0;
//  static Sensor s(inCoef, outCoef, slowCoef, fastCoef);
  static Sensor s(7, 10, slowCoef, fastCoef);

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  if (newDataReady) 
  {
    if (millis() > t + serialPrintInterval) 
    {
      float sample = abs(LoadCell.getData());

      if (s.detectH(sample))
      {
        setLedColor(0, 128, 128);
      }
      else 
      {
        setLedColor(0, 0, 0);
        if (tareInterval < tareTimer)
        {
//          LoadCell.tareNoDelay();
          tareTimer = 0;
          Serial.println("Taring");
        }
        else
        {
          Serial.println("Measuring");
        }
        
      }
      
      Serial.print(s.m_fastFilter);
      Serial.print(',');
      Serial.println(s.m_slowFilter);
      newDataReady = 0;
      tareTimer += serialPrintInterval;
      t = millis();
    }
  }

  calibrateLoadCells();
}
