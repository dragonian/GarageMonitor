#include <Boards.h>
#include <Firmata.h>


const int pinLED = 13;      
const int pinDoorPulse = 4; 
const int pinDoorIsOpen = 5;
const int pinOpenDoorReq = 6;
const int pinDoorSensor = 8;

const int pinToggleLed = 1;

// Variables will change:
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 1000;           // interval at which to blink (milliseconds)

enum DoorState {
  UNKNOWN,
  DOOR_OPENING,
  DOOR_OPEN,
  DOOR_CLOSING,
  DOOR_CLOSED
};
boolean setState(DoorState state);

DoorState doorState = UNKNOWN;
DoorState oldState = UNKNOWN;

volatile bool toggle = false;
volatile bool request = false;

void setup() {

  // set the digital pin as output:
  pinMode(pinLED, OUTPUT); 
  pinMode(pinDoorPulse, OUTPUT);   
  pinMode(pinDoorIsOpen, OUTPUT); 
  pinMode(pinOpenDoorReq, INPUT); 
  pinMode(pinDoorSensor, INPUT_PULLUP);

  Serial.begin(9600);
  
  pinMode(pinToggleLed, OUTPUT); 
  attachInterrupt(2, isr0, RISING);

}

void pulseDoor()
{
  digitalWrite(pinDoorPulse, LOW);
  delay(100);
  digitalWrite(pinDoorPulse, HIGH);    
  delay(100);
}

void isr0()
{
  if (toggle == false)
    toggle = true;
  else
    toggle = false; 

  digitalWrite(pinToggleLed, toggle);
  request = toggle;
}

void setRequest(boolean onoff)
{
  if (onoff == true)
    request = true;
  else
    request = false; 

  toggle = request;
  digitalWrite(pinToggleLed, toggle);
  //digitalWrite(pinOpenDoorReq, request);
}

boolean setState(DoorState state)
{
  if (oldState != state)
  {  
    doorState = state;
    oldState = doorState;
    return true;
  }
  return false;
}

void outputDoorState()
{
    switch (doorState)
    {
      case DOOR_OPEN:  
      case DOOR_OPENING:  
        digitalWrite(pinDoorIsOpen, true);
        setRequest(true);
        break;
        
      case DOOR_CLOSED:  
      case DOOR_CLOSING:  
        digitalWrite(pinDoorIsOpen, false);
        setRequest(false);
        break;
    }
}


void ticLED()
{
  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;  

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(pinLED, ledState);
  }
}

void loop() 
{
  // put your main code here, to run repeatedly: 
  ticLED();

  // If the Door is opening/closing, don't do anything until it's finished
  if (doorState == DOOR_CLOSING)
  {
    if (digitalRead(pinDoorSensor) == LOW)
    {
      if (setState(DOOR_CLOSED) == true)
        Serial.println("Door is closed");
    }
  }
  else if (doorState == DOOR_OPENING)
  {
    if (digitalRead(pinDoorSensor) == HIGH)
    {
      if (setState(DOOR_OPEN) == true)
        Serial.println("Door is open");
    }
  }
  else
  {
    if (digitalRead(pinDoorSensor) == HIGH)
    {
      if (setState(DOOR_OPEN) == true)
      {
        Serial.println("Door is open");
        outputDoorState();
      }
    }
    else
    {
      if (setState(DOOR_CLOSED) == true)
      {
        Serial.println("Door is closed");
        outputDoorState();
      }
    }

    /*outputDoorState();*/
    delay(500);
      
    if (/*digitalRead(pinOpenDoorReq) == HIGH*/ request)
    {
      if (doorState == DOOR_CLOSED)
      {   
        if (setState(DOOR_OPENING) == true)        
          Serial.println("Opening door");
        pulseDoor();
      }
    }
    else
    {
      if (doorState == DOOR_OPEN)
      {
        if (setState(DOOR_CLOSING) == true)
          Serial.println("Closing door");
        pulseDoor();
      }
    }
  }
  
  outputDoorState();
  delay(500);

}

