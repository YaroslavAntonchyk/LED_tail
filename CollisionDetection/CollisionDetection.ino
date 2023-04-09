#include <SoftwareSerial.h>
#include "AsyncStream.h"

const int ledPin =  13;  // Built-in LED

//Receiver Output Enable. RO is enabled when RE is low; RO is high impedance when RE is high
const int RecEnPin = 2;
// Driver Output Enable. The driver outputs, Y and Z, are enabled by bringing DE high.
// They are high impedance when DE is low. If the driver outputs are enabled, the parts function as line drivers.
// While they are high impedance, they function as line receivers if RE is low
const int DevEnPin = 3;


// const char DEVICE_ID = '1';

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
  byte state;
  byte color;
  byte crc;
};
SoftwareSerial swSerial(A1, A0); // RX, TX
AsyncStream<20> serial(&Serial, sizeof(Message));

void setup() 
{
  swSerial.begin(115200);
  Serial.begin(115200);
  swSerial.setTimeout(5);
  Serial.setTimeout(5);

  pinMode(ledPin, OUTPUT);
  pinMode(RecEnPin, OUTPUT);
  pinMode(DevEnPin, OUTPUT);

  digitalWrite(ledPin, LOW); 
  digitalWrite(RecEnPin, LOW);
  digitalWrite(DevEnPin, LOW);

}

void loop() 
{
  byte id = 1;
  digitalWrite(DevEnPin, HIGH);
  byte buff[] = {1, 2, 3, 4};
  Serial.write(buff, sizeof(buff) / sizeof(byte));
  Serial.flush();

  swSerial.print(Serial.available());
  swSerial.print('-');
  // while (Serial.available())
  // {
  //   swSerial.print(static_cast<int>(Serial.read()));
  // }
  if (serial.available())
  {
    Message rxMsg(serial.buf);
    swSerial.print(static_cast<int>(rxMsg.id));
    swSerial.print(static_cast<int>(rxMsg.state));
    swSerial.print(static_cast<int>(rxMsg.color));
    swSerial.println(static_cast<int>(rxMsg.crc));
  }
}
