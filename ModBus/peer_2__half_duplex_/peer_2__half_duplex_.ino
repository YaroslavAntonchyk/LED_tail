const int EnTxPin = 2;
const int buttonPin = 5;     // the number of the pushbutton pin
const int led1 = 9;
constexpr byte DEVICE_ID = 49; // '1'

bool stringComplete = false;
void setup ()
{
  Serial.begin (115200);
  Serial.setTimeout(5);
  pinMode(EnTxPin, OUTPUT );
  pinMode(led1, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite (EnTxPin, LOW);
  digitalWrite (led1, LOW);
  // Serial.println("ready");
}

void loop ()
{
  bool state = isButtonPressed(buttonPin);
  char inputString[5];
  size_t msgLength = Serial.readBytesUntil('F', inputString, 5);
  // for (size_t i = 0; i < msgLength; ++i)
  //   Serial.print(inputString[i]);
  // Serial.println();
  if ((msgLength != 0) && (inputString[0] == '1'))
  {
    digitalWrite (EnTxPin, HIGH);
    Serial.write('\n');
    Serial.write('1');
    Serial.write((state ? 'y' : 'n'));
    Serial.write('F');
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
      return true;
    }
  // }
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

// void parseMsg(char* str)
// {
//   Message* msg = reinterpret_cast<Message*>(str);
//   if (DEVICE_ID == msg->id)
//   {
//     analogWrite(led1, msg->color);
//     digitalWrite (EnTxPin, HIGH ); //enable to transmit
// //    delay(5);
//     Serial.write(msg->id);
//     if (state) Serial.write('y');
//     else Serial.write('n');
//     Serial.write('F');
//     Serial.flush();
//     delay(1);
//     digitalWrite (EnTxPin, LOW ); //enable to receive
//   }
// }

// void serialEvent() 
// {
//   static int i = 0;
//   while (Serial.available()) 
//   {
//     // get the new byte:
//     byte inChar = (byte)Serial.read();
//     Serial.print((char)inChar);
//     // add it to the inputString:
//     inputString[i] = inChar;
//     i++;
//     if (('F' == inChar)) 
//     {
//       parseMsg(inputString);
//       i = 0;
//     }
//     else if(i > 2)
//     {
//       i = 0;
//     }
//   }
// }
