
const int ledPin =  13;  // Built-in LED
const int EnTxPin =  2;  // HIGH:Transmitter, LOW:Receiver
const int buttonPin = 5;     // the number of the pushbutton pin
const String msg = "1AF";
constexpr byte DEVICE_ID = 49; // '1'
char inputString[5];

void setup() 
{ 
  Serial.begin(115200);
  Serial.setTimeout(5);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(EnTxPin, OUTPUT);
  digitalWrite(ledPin, LOW); 
  digitalWrite(EnTxPin, HIGH); 
} 
 
void loop() 
{ 
  static unsigned long t = 0;
  digitalWrite(EnTxPin, HIGH); //RS485 as transmitter
  Serial.write(msg[0]);
  Serial.write(msg[1]);
  Serial.write(msg[2]);

  Serial.flush();

  digitalWrite(EnTxPin, LOW); //RS485 as receiver

  char inputString[5];  
  size_t msgLength = Serial.readBytesUntil('F', inputString, 5);
  // for (size_t i = 0; i < msgLength; ++i)
  //   Serial.print(inputString[i]);
  if ((msgLength != 0) && (inputString[1] == '1'))
  {
    Serial.print(inputString[2]);
    Serial.println(micros() - t);
    t = micros();    
  }

} 

bool isButtonPressed(int buttonPin) 
{
  bool buttonState = digitalRead(buttonPin);
  
  if (false == buttonState)
  {
    delay(1);
    if (false == buttonState)
    {
      return true;
    }
  }
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

struct Message
{
  byte id;
  byte color;
  byte reserved;
};

void parseMsg(char* str)
{
  Message* msg = reinterpret_cast<Message*>(str);
  if (DEVICE_ID == msg->id)
  {
    Serial.write((char)msg->id);
    Serial.write((char)msg->color);
    Serial.write((char)msg->reserved);
  }
}
