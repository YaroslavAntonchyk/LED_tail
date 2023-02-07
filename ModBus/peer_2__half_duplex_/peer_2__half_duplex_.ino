const int EnTxPin = 2;
const int ledPin =  13;  // Built-in LED
const int led1 = 3;
void setup ()
{
  Serial.begin (9600); 
  pinMode(EnTxPin, OUTPUT );
  pinMode(ledPin, OUTPUT);
  pinMode(led1, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite (EnTxPin, LOW );
  digitalWrite (led1, LOW );
}

void loop ()
{
  static byte val = 0;
  if ( Serial.available ())
  {
    if ( Serial.read () == 'I' )
    {
      char function = Serial.read();
      if (function == 'S' )
      {
        int brightness = Serial.parseInt ();
        if ( Serial.read () == 'F' )
        {
          if ((brightness >= 0) && brightness <= 255)
          {
            analogWrite(led1, brightness);
          }
        }
      }
      if (function == 'L' )
      {
        if ( Serial.read () == 'F' )
        {
          val++;
          digitalWrite (EnTxPin, HIGH ); //enable to transmit
          Serial.print ( "i" ); 
          Serial.print (val); 
          Serial.println ( "f" ); 
          Serial.flush (); 
          digitalWrite (EnTxPin, LOW ); //enable to receive
        }
      }
    }
  }
}
