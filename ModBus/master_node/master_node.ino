#include <SoftwareSerial.h>

const int ledPin =  13;  // Built-in LED
const int EnTxPin =  2;  // HIGH:Transmitter, LOW:Receiver

const int devicesInNet = 1;

SoftwareSerial swSerial(A1, A0); // RX, TX

struct Message
{ //add default constructor
  Message(byte _id, char _state, char _color, byte _crc = 255):
    id(_id),
    state(_state),
    color(_color),
    crc(_crc)
  {

  }
  byte id;
  char state;
  char color;
  byte crc;
};

void setup() 
{ 
  swSerial.begin(115200);
  Serial.begin(115200);
  swSerial.setTimeout(5);
  Serial.setTimeout(5);

  pinMode(ledPin, OUTPUT);
  pinMode(EnTxPin, OUTPUT);

  digitalWrite(ledPin, LOW); 
  digitalWrite(EnTxPin, HIGH); 
} 
 
void loop() 
{ 
  static unsigned long t1 = 0;
  static unsigned long t2 = 0;
  {
    for(byte id = 1; id <= devicesInNet; ++id)
    {
      digitalWrite(EnTxPin, HIGH); //RS485 as transmitter
      Message txMsg(id, 'n', 'a');
      txMsg.crc = crc8_bytes((byte*)&txMsg, sizeof(txMsg) - 1);
      Serial.write((byte*)&txMsg, sizeof(txMsg));
      Serial.flush();

      digitalWrite(EnTxPin, LOW); //RS485 as receiver

      Message rxMsg(0, '0', '0', 0);
      size_t msgLength = Serial.readBytes((byte*)&rxMsg, sizeof(rxMsg));
      byte crc = crc8_bytes((byte*)&rxMsg, sizeof(rxMsg));
      swSerial.print(static_cast<int>(rxMsg.id));
      swSerial.print(rxMsg.state);
      swSerial.print(rxMsg.color);
      swSerial.print(crc == 0 ? 'K' : 'F');
      if ((msgLength != 0) && (crc == 0) && (rxMsg.id == txMsg.id))
      {
        if(rxMsg.state == 'y')
        {
          txMsg.color = 'g';
        }
        else if (rxMsg.state == 'n')
        {
          txMsg.color = 'd';
        }

        digitalWrite(EnTxPin, HIGH); //RS485 as transmitter

        txMsg.crc = crc8_bytes((byte*)&txMsg, sizeof(txMsg) - 1);
        Serial.write((byte*)&txMsg, sizeof(txMsg));
        Serial.flush();

        digitalWrite(EnTxPin, LOW); //RS485 as receiver

        Serial.readBytes((byte*)&rxMsg, sizeof(rxMsg));
        swSerial.print(static_cast<int>(rxMsg.id));
        swSerial.print(rxMsg.state);
        swSerial.print(rxMsg.color);
        swSerial.print(crc == 0 ? 'K' : 'F');
        swSerial.println(micros() - t1);
        t1 = micros(); 
      }
    }
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
