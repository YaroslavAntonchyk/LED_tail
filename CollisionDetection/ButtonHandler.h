#ifndef _ButtonHandler_h
#define _ButtonHandler_h

bool isButtonPressed(int buttonPin) 
{
  bool buttonState = digitalRead(buttonPin);
  
  if (false == buttonState)
  {
    delayMicroseconds(20);
    if (false == buttonState)
    {
      return true;
    }
  }

  return false;
}


bool isRisingEdge(int buttonPin)
{
  static bool prevState = true;
  bool currState = isButtonPressed(buttonPin);
  if ((prevState == false) && (currState == true))
  {
    prevState = currState;
    return true;
  }
  prevState = currState;
  return false;
}

bool isFallingEdge(int buttonPin)
{
  bool currState = isButtonPressed(buttonPin);
  static bool prevState = false;
  
  if ((prevState == true) && (currState == false))
  {
    prevState = currState;
    return true;
  }
  prevState = currState;
  return false;
}

bool isDetected(char& state, int buttonPin)
{
  if (isRisingEdge(buttonPin))
  {
    state = 'y';
    return true;
  }
  if (isFallingEdge(buttonPin))
  {
    state = 'n';
    return true;
  }

  return false;
}

#endif //_ButtonHandler_h