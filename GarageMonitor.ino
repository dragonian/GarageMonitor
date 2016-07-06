#include <Boards.h>
#include <Firmata.h>


const int pinLED = 13;      
const int pinDoorPulse = 4; 
const int pinDoorIsOpen = 5;
const int pinOpenDoorReq = 6;
const int pinDoorSensor = 8;

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

void setup() {

  // set the digital pin as output:
  pinMode(pinLED, OUTPUT); 
  pinMode(pinDoorPulse, OUTPUT);   
  pinMode(pinDoorIsOpen, OUTPUT); 
  pinMode(pinOpenDoorReq, INPUT_PULLUP); 
  pinMode(pinDoorSensor, INPUT_PULLUP);
  
  // Set initial high state
  digitalWrite(pinDoorPulse, HIGH); 

  Serial.begin(9600);
}

void pulseDoor()
{
  digitalWrite(pinDoorPulse, LOW);
  delay(100);
  digitalWrite(pinDoorPulse, HIGH);    
  delay(100);
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
        break;
        
      case DOOR_CLOSED:  
      case DOOR_CLOSING:  
        digitalWrite(pinDoorIsOpen, false);
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

  // If the Door is closing, don't do anything until it is closed
  if (doorState == DOOR_CLOSING)
  {
    if (digitalRead(pinDoorSensor) == LOW)
    {
      if (setState(DOOR_CLOSED) == true)
        Serial.println("Door is closed");
    }
  }
  else
  {
    if (digitalRead(pinDoorSensor) == HIGH)
    {
      if (setState(DOOR_OPEN) == true)
        Serial.println("Door is open");
    }
    else
    {
      if (setState(DOOR_CLOSED) == true)
        Serial.println("Door is closed");
    }

    outputDoorState();
    delay(200);  // need this delay, so the vcrx can update its state
      
    if (digitalRead(pinOpenDoorReq) == LOW)
    {
      if (doorState == DOOR_CLOSED)
      {   
        if (setState(DOOR_OPENING) == true)
        {        
          Serial.println("Opening door");
          pulseDoor();
          
          delay(5000);  // Wait for the door to react
        }
      }
    }
    else
    {
      if (doorState == DOOR_OPEN)
      {
        if (setState(DOOR_CLOSING) == true)
        {
          Serial.println("Closing door");
          pulseDoor();
          
          delay(1000);   // Wait for the door to react
        }
      }
    }
  }
  
  outputDoorState();

}

