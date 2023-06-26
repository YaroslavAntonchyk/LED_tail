#include <SoftwareSerial.h>
#include "AsyncStream.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "HardwareSerial_private.h"
#include "ButtonHandler.h"

////////////////////////////////////
const byte thisDeviceId = 1;
////////////////////////////////////

const byte dstDeviceId = 0;
const int ledPin = 13;  // Built-in LED
const int buttonPin = 5;

//Receiver Output Enable. RO is enabled when RE is low; RO is high impedance when RE is high
const int RecEnPin = 3;
// Driver Output Enable. The driver outputs, Y and Z, are enabled by bringing DE high.
// They are high impedance when DE is low. If the driver outputs are enabled, the parts function as line drivers.
// While they are high impedance, they function as line receivers if RE is low
const int DevEnPin = 2;

constexpr unsigned char SOT = '{';
constexpr unsigned char EOT = '}';

const unsigned int timeOut = 500; //microseconds

enum Status { success, fail };

struct Message
{
  Message() = default;

  Message(byte _srcId, byte _dstId, byte _state):
    sot('{'),
    srcId(_srcId),
    dstId(_dstId),
    state(_state),
    eot('}')
  {

  }

  Message(char* buf):
    sot(buf[0]),
    srcId(buf[1]),
    dstId(buf[2]),
    state(buf[3]),
    eot(buf[4])
  {
    
  }

  void print(Stream* _port)
  {
    _port->print(static_cast<char>(sot));
    _port->print(srcId);
    _port->print(dstId);
    _port->print(static_cast<char>(state));
    _port->println(static_cast<char>(eot));
  }

  bool operator==(Message const& other)
  {
    bool isEqual = true;
    isEqual &= (sot == other.sot);
    isEqual &= (srcId == other.srcId);
    isEqual &= (dstId == other.dstId);
    isEqual &= (state == other.state);
    isEqual &= (eot == other.eot);
    return isEqual;
  }

  byte sot;
  byte srcId;
  byte dstId;
  byte state;
  byte eot;
};

SoftwareSerial swSerial(A1, A0); // RX, TX

class RS485 : public HardwareSerial
{
public:
  RS485() : 
    HardwareSerial (&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0),
    isBusFree(true)
  { 

  }

  void _rx_complete_irq(void)
  {
    if (bit_is_clear(*_ucsra, UPE0)) 
    {
      // No Parity error, read byte and store it in the buffer if there is
      // room
      unsigned char c = *_udr;
      if (SOT == c)
      {
        isBusFree = false;
      }
      else if (EOT == c) 
      {
        isBusFree = true;
      }

      rx_buffer_index_t i = (unsigned int)(_rx_buffer_head + 1) % SERIAL_RX_BUFFER_SIZE;

      // if we should be storing the received character into the location
      // just before the tail (meaning that the head would advance to the
      // current location of the tail), we're about to overflow the buffer
      // and so we don't write the character or advance the head.
      if (i != _rx_buffer_tail) 
      {
        _rx_buffer[_rx_buffer_head] = c;
        _rx_buffer_head = i;
      }
    } 
    else 
    {
      // Parity error, read byte but discard it
      *_udr;
    }
  }

  volatile bool isBusFree;
};

RS485 rs485;
AsyncStream<20> serial(&rs485, EOT);

ISR(USART_RX_vect)
{
  rs485._rx_complete_irq();
}

ISR(USART_UDRE_vect)
{
  rs485._tx_udr_empty_irq();
}

void setup() 
{
  swSerial.begin(115200);
  rs485.begin(115200);
  swSerial.setTimeout(5);
  rs485.setTimeout(5);

  pinMode(ledPin, OUTPUT);
  pinMode(RecEnPin, OUTPUT);
  pinMode(DevEnPin, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(ledPin, LOW); 
  digitalWrite(RecEnPin, LOW);
  digitalWrite(DevEnPin, LOW);
}

int sendMsg(Message& msg)
{
  Status status = success;
  long long t = micros();
  while(!rs485.isBusFree) //ToDo rethink
  {
    if ((micros() - t) > timeOut)
    {
      break;
    }
  }
  
  digitalWrite(DevEnPin, HIGH);
  rs485.write((byte*)&msg, sizeof(msg));
  rs485.flush();
  digitalWrite(DevEnPin, LOW);

  // Check if msg delivered
  if (serial.available())
  {
    Message rxMsg(serial.buf);
    status = (rxMsg == msg) ? success : fail;
  }

  return status;
}

void loop() 
{
  char state;
  if (isDetected(state, buttonPin))
  {
    Message txMsg(thisDeviceId, dstDeviceId, state);
    for (int attempts = 5; attempts > 0; attempts--)
    {
      if (success == sendMsg(txMsg))
      {
        break;
      }
    }
  }

  if (serial.available())
  {
    Message rxMsg(serial.buf);
    if (thisDeviceId == rxMsg.dstId)
    {
      if('g' == rxMsg.state)
        digitalWrite(ledPin, HIGH);
      else if('r' == rxMsg.state)
        digitalWrite(ledPin, LOW);

      rxMsg.print(&swSerial);
    }
  }
}


