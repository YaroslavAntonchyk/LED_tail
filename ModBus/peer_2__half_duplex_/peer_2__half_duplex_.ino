const int EnTxPin = 2;
const int buttonPin = 5;     // the number of the pushbutton pin
const int ledPin = 13;

const char DEVICE_ID = '1';

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

void setup ()
{
  Serial.begin (115200);
  Serial.setTimeout(5);

  pinMode(EnTxPin, OUTPUT );
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite (EnTxPin, LOW);
  digitalWrite (ledPin, LOW);
  // Serial.println("ready");
}

void loop ()
{
  bool state = isButtonPressed(buttonPin);

  Message rxMsg('0', '0', '0');
  size_t msgLength = Serial.readBytes((byte*)&rxMsg, sizeof(rxMsg));
  // for (size_t i = 0; i < msgLength; ++i)
  //   Serial.print(inputString[i]);
  byte crc = crc8_bytes((byte*)&rxMsg, sizeof(rxMsg));
  if ((msgLength != 0) && (crc == 0) && (rxMsg.id == DEVICE_ID))
  {
    digitalWrite (EnTxPin, HIGH);
    Message txMsg(DEVICE_ID, (state ? 'y' : 'n'), rxMsg.color);
    txMsg.crc = crc8_bytes((byte*)&txMsg, sizeof(txMsg) - 1);
    Serial.write((byte*)&txMsg, sizeof(txMsg));
    Serial.flush();

    digitalWrite (EnTxPin, LOW);
  }
}

bool isButtonPressed(int buttonPin) 
{
  bool buttonState = digitalRead(buttonPin);
  
  // if (false == buttonState)
  // {
  //   delay(1);
    if (false == buttonState)
    {
      digitalWrite (ledPin, HIGH);
      return true;
    }
  // }
  digitalWrite (ledPin, LOW);
  return false;
}

bool isRisingEdge(int buttonPin)
{
  bool currState = isButtonPressed(buttonPin);
  static bool prevState = false;
  
  if ((prevState == false) && (currState == true))
  {
    prevState = currState;
    return true;
  }
  prevState = currState;
  return false;
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
