#include <SoftwareSerial.h>
#include "AsyncStream.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "HardwareSerial_private.h"

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

// const byte dstDeviceId = 1;
const byte thisDeviceId = 0;

class MySerial : public HardwareSerial
{
public:
  MySerial() : 
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

  byte sot;
  byte srcId;
  byte dstId;
  byte state;
  byte eot;
};

SoftwareSerial swSerial(A1, A0); // RX, TX
MySerial mySerial;
AsyncStream<20> serial(&mySerial, sizeof(Message));

ISR(USART_RX_vect)
  {
    mySerial._rx_complete_irq();
  }

ISR(USART_UDRE_vect)
{
  mySerial._tx_udr_empty_irq();
}

void setup() 
{
  swSerial.begin(115200);
  mySerial.begin(115200);
  swSerial.setTimeout(5);
  mySerial.setTimeout(5);

  pinMode(ledPin, OUTPUT);
  pinMode(RecEnPin, OUTPUT);
  pinMode(DevEnPin, OUTPUT);

  digitalWrite(ledPin, LOW); 
  digitalWrite(RecEnPin, LOW);
  digitalWrite(DevEnPin, LOW);
}

void loop() 
{
  if (serial.available())
  {
    Message rxMsg(serial.buf);
    rxMsg.print(&swSerial);
    if (thisDeviceId == rxMsg.dstId)
    {
      byte color = 0;
      if ('y' == rxMsg.state) color = 'g';
      else if ('n' == rxMsg.state) color = 'r';
      Message txMsg(thisDeviceId, rxMsg.srcId, color);
      long long t = micros();
      while(!mySerial.isBusFree)
      {
        if ((micros() - t) > 1000)
        {
          mySerial.isBusFree = true;
        }
      }
      digitalWrite(DevEnPin, HIGH);
      mySerial.write((byte*)&txMsg, sizeof(txMsg));
      mySerial.flush();
      digitalWrite(DevEnPin, LOW);
    }
  }
}
