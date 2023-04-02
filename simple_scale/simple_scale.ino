#include "HX711_ADC.h"
#include "AsyncStream.h"

//pins:
const int HX711_dout = 7; //mcu > HX711 dout pin
const int HX711_sck = 6; //mcu > HX711 sck pin
const int EnTxPin = 2;
const int buttonPin = 5; // the number of the pushbutton pin
const int ledPin = 13;
const int ledR = 10;
const int ledG = 11;
const int ledB = 9;

const int MAX_BRIGHTNESS = 128;
const byte DEVICE_ID = '1'; 

struct Message
{
  Message(byte _id, char _state, char _color, byte _crc = 255):
    id(_id),
    state(_state),
    color(_color),
    crc(_crc)
  {

  }
  Message(char* buf):
    id(buf[0]),
    state(buf[1]),
    color(buf[2]),
    crc(buf[3])
  {
    
  }
  byte id;
  char state;
  char color;
  byte crc;
};

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
AsyncStream<20> serial(&Serial, sizeof(Message));

void setup() 
{
  Serial.begin(115200);
  Serial.setTimeout(5);
  Serial.println("Starting...");

  pinMode(EnTxPin, OUTPUT );
  pinMode(ledPin, OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(EnTxPin, LOW);
  digitalWrite(ledPin, LOW);
  
  LoadCell.begin();
  LoadCell.setReverseOutput();
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

void loop() 
{
  static unsigned long t = 0;
  static float sample = 0;
  static bool state = false; 
  // t = micros();
  if (LoadCell.update())
  {
      float raw = LoadCell.getData();
      sample = sample*0.8f + raw*0.2f;
      state = sample > 10000;
      // Serial.print(raw);
      // Serial.print(',');
      // Serial.println(sample);
  }
  if (serial.available()) 
  {
    Message rxMsg(serial.buf);
    // size_t msgLength = Serial.readBytes((byte*)&rxMsg, sizeof(rxMsg));

    byte crc = crc8_bytes((byte*)&rxMsg, sizeof(rxMsg));
    if ((crc == 0) && (rxMsg.id == DEVICE_ID))
    {
      digitalWrite (EnTxPin, HIGH);
      Message txMsg(DEVICE_ID, (state ? 'y' : 'n'), rxMsg.color);
      txMsg.crc = crc8_bytes((byte*)&txMsg, sizeof(txMsg) - 1);
      Serial.write((byte*)&txMsg, sizeof(txMsg));
      Serial.flush();
      digitalWrite (EnTxPin, LOW);

      if (rxMsg.color == 'g')
      {
        digitalWrite(ledPin, HIGH);
        setLedColorRGB(0, MAX_BRIGHTNESS, 0); //green
      }
      else if(rxMsg.color == 'd')
      {
        digitalWrite(ledPin, LOW);
        setLedColorRGB(MAX_BRIGHTNESS, 0, 0); //red 
      }
  }
  }
  // Serial.println(micros() - t);
}

byte crc8_bytes(byte *buffer, byte size) 
{
  byte crc = 0;
  for (byte i = 0; i < size; i++) 
  {
    byte data = buffer[i];
    for (int j = 8; j > 0; j--) 
    {
      crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
      data >>= 1;
    }
  }
  return crc;
}

void setLedColorRGB(int r, int g, int b)
{
  r = constrain(r, 0, MAX_BRIGHTNESS);
  g = constrain(g, 0, MAX_BRIGHTNESS);
  b = constrain(b, 0, MAX_BRIGHTNESS);
  analogWrite(ledR, r);
  analogWrite(ledG, g);
  analogWrite(ledB, b);
}
