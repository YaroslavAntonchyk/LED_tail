#include <SoftwareSerial.h>
#include "AsyncStream.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "HardwareSerial_private.h"

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

class MySerial : public HardwareSerial
{
public:
  MySerial() : HardwareSerial (&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0) { }
  void _rx_complete_irq(void)
  {
    HardwareSerial::_rx_complete_irq();
    swSerial.println(_rx_buffer[_rx_buffer_head-1]);
  }
};

MySerial mySerial;
AsyncStream<20> serial(&mySerial, sizeof(Message));

#if defined(USART_RX_vect)
  ISR(USART_RX_vect)
#elif defined(USART0_RX_vect)
  ISR(USART0_RX_vect)
#elif defined(USART_RXC_vect)
  ISR(USART_RXC_vect) // ATmega8
#else
  #error "Don't know what the Data Received vector is called for Serial"
#endif
  {
    mySerial._rx_complete_irq();
  }

#if defined(UART0_UDRE_vect)
ISR(UART0_UDRE_vect)
#elif defined(UART_UDRE_vect)
ISR(UART_UDRE_vect)
#elif defined(USART0_UDRE_vect)
ISR(USART0_UDRE_vect)
#elif defined(USART_UDRE_vect)
ISR(USART_UDRE_vect)
#else
  #error "Don't know what the Data Register Empty vector is called for Serial"
#endif
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
  byte id = 1;
  digitalWrite(DevEnPin, HIGH);
  byte buff[] = {1, 2, 3, 4};
  mySerial.write(buff, sizeof(buff) / sizeof(byte));
  mySerial.flush();

  swSerial.print(mySerial.available());
  swSerial.print('-');
  // while (mySerial.available())
  // {
  //   swSerial.print(static_cast<int>(mySerial.read()));
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
