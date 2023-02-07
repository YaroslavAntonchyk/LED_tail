
const int ledPin =  13;  // Built-in LED
const int EnTxPin =  2;  // HIGH:Transmitter, LOW:Receiver
const int buttonPin = 5;     // the number of the pushbutton pin
void setup() 
{ 
  Serial.begin(9600);
  Serial.setTimeout(100);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(EnTxPin, OUTPUT);
  digitalWrite(ledPin, LOW); 
  digitalWrite(EnTxPin, HIGH); 
} 
 
void loop() 
{   
  static byte angle = 0;
  if(isRisingEdge(buttonPin))
  {
    angle += 20;
    //transmitter data packet
    Serial.print("I"); //initiate data packet
    Serial.print("S"); //code for servo
    Serial.print(angle); //servo angle data
    Serial.print("F"); //finish data packet

    //receiver data packet
    Serial.print("I"); //initiate data packet
    Serial.print("L"); //code for sensor
    Serial.print("F"); //finish data packet
    Serial.flush();    
    
    digitalWrite(EnTxPin, LOW); //RS485 as receiver
  
    if(Serial.find("i"))
    {
        int data=Serial.parseInt(); 
        if(Serial.read()=='f') //finish reading
         {
           onLED(data);
           Serial.print("Received ");
           Serial.println(data);     
        }
    }
    digitalWrite(EnTxPin, HIGH); //RS485 as transmitter
 }
} 

void onLED(int data)
{
  if(data > 130)
     digitalWrite(ledPin, HIGH); 
  else
     digitalWrite(ledPin, LOW); 
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
