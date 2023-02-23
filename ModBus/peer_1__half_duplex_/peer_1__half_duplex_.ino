#include <SoftwareSerial.h>

const int ledPin =  13;  // Built-in LED
const int EnTxPin =  2;  // HIGH:Transmitter, LOW:Receiver

const char DEVICE_ID = '1';

SoftwareSerial mySerial(A1, A0); // RX, TX

struct Message
{
  Message(char _id, char _state, char _color, byte _crc = 255):
    id(_id),
    state(_state),
    color(_color),
    crc(_crc)
  {

  }
  char id;
  char state;
  char color;
  byte crc;
};

void setup() 
{ 
  Serial.begin(115200);
  mySerial.begin(115200);
  Serial.setTimeout(5);
  mySerial.setTimeout(5);

  pinMode(ledPin, OUTPUT);
  pinMode(EnTxPin, OUTPUT);

  digitalWrite(ledPin, LOW); 
  digitalWrite(EnTxPin, HIGH); 
} 
 
void loop() 
{ 
  static unsigned long t = 0;

  digitalWrite(EnTxPin, HIGH); //RS485 as transmitter
  Message txMsg('1', 'n', 'a');
  txMsg.crc = crc8_bytes((byte*)&txMsg, sizeof(txMsg) - 1);
  mySerial.write((byte*)&txMsg, sizeof(txMsg));
  mySerial.flush();

  digitalWrite(EnTxPin, LOW); //RS485 as receiver

  Message rxMsg('0', '0', '0', '0');
  size_t msgLength = mySerial.readBytes((byte*)&rxMsg, sizeof(rxMsg));
  byte crc = crc8_bytes((byte*)&rxMsg, sizeof(rxMsg));
  // for (size_t i = 0; i < msgLength; ++i)
  //   Serial.print(inputString[i]);
  if ((msgLength != 0) && (crc == 0))// && (rxMsg.id == DEVICE_ID))
  {
    Serial.print(rxMsg.id);
    Serial.print(rxMsg.state);
    Serial.print(rxMsg.color);
    Serial.println(micros() - t);
    t = micros();    
  }
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
