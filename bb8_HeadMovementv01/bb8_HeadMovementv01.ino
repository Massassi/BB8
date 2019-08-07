/***************************************************
 This code is the full implementation of BB8
 
 Created 2018-8-28
 By [massassi order](Massassi.Order@gmail.com)
 
 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here
 <https://github.com/Massassi/BB8/tree/master/Electronics>
 2.This code is tested on Arduino Nano board
 ****************************************************/
#include <Stepper.h>
#include "SoftwareSerial.h"

const int stepsPerRevolution = 200; 
int masterInput = 3;

enum movingState {
  standby,
  firstLeft,
  secondRight,
  thirdLeft,
  firstRight,
  secondLeft,
  thirdRight,
  moveLeft,
  moveRight,
  movingDoneWaitTimer,
};

movingState currentMovingState = standby;
movingState nextStateMove = standby;

int randomEndTimer = 0; 
int prevMovingsteps = 0;
int movingsteps = 0;
int nextMove = 0;
int movingTimer = 0;
Stepper myStepper(stepsPerRevolution, 8, 7, 5, 4); // in1, in2, in3, in4 
SoftwareSerial mySoftwareSerial(10, 11);           // 10=RX, 11=TX

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  // put your setup code here, to run once:
  // set the speed at 60 rpm:
  myStepper.setSpeed(120);

  pinMode(masterInput, INPUT);    

  mySoftwareSerial.begin(9600);
  Serial.begin(9600);

  StateMovingChanged(standby);
}

void loop() 
{
  if (digitalRead(masterInput) == LOW)    
  {
    delay (1000);
    //digitalWrite(LED_BUILTIN, LOW);    
    //Serial.println(F(" => Low"));  
  }
  else // == HIGH
  {
    //delay (1000);
    //digitalWrite(LED_BUILTIN, HIGH);
    //Serial.println(F(" => HIGH"));  
    switch (currentMovingState)
    {    
      case standby:
        movingsteps = random(150, 300);
        nextMove = 0;
        switch (nextStateMove)
        {
          case standby:
            nextMove = random(0, 4);
            switch(nextMove)
            {
              case 0:
                // start left;
                StateMovingChanged(firstLeft);
                nextStateMove = standby;
                break;
              case 1:
                // start right
                StateMovingChanged(firstRight);
                nextStateMove = standby;
                break;
              case 2:
                // move left then right
                prevMovingsteps = movingsteps;
                StateMovingChanged(moveLeft);
                nextStateMove = moveRight;
                break;
              case 3:
                // move right then left
                prevMovingsteps = movingsteps;
                StateMovingChanged(moveRight);
                nextStateMove = moveLeft;
                break;
            }
            break;
          case moveRight:
            StateMovingChanged(moveRight);
            nextStateMove = standby;
            break;
          case moveLeft:
            StateMovingChanged(moveLeft);
            nextStateMove = standby;
            break;
        }
        break;
      // start left //////////////////////////////////////////////////////////////////////////////////////////////////////////////
      case firstLeft:
        myStepper.step(movingsteps);      
        StateMovingChanged(secondRight);      
        break;    
      case secondRight:    
        myStepper.step(-movingsteps*2);      
        StateMovingChanged(thirdLeft);        
      case thirdLeft:
        myStepper.step(movingsteps);
        movingTimer = startRandomTimer();
        StateMovingChanged(movingDoneWaitTimer);
        break;      
      // start right //////////////////////////////////////////////////////////////////////////////////////////////////////////////  
      case firstRight:
        myStepper.step(-movingsteps);
        StateMovingChanged(secondLeft);      
        break;    
      case secondLeft:
        myStepper.step(movingsteps*2);
        StateMovingChanged(thirdRight);
        break;          
      case thirdRight:      myStepper.step(-movingsteps);
        movingTimer = startRandomTimer();
        StateMovingChanged(movingDoneWaitTimer);
        break; 
      //  move left then right //////////////////////////////////////////////////////////////////////////////////////////////////////////////
      case moveLeft:
        myStepper.step(movingsteps);
        movingTimer = startRandomTimer();
        StateMovingChanged(movingDoneWaitTimer);      
        break;
      //  move right then left //////////////////////////////////////////////////////////////////////////////////////////////////////////////
      case moveRight:
        myStepper.step(-movingsteps);
        movingTimer = startRandomTimer();
        StateMovingChanged(movingDoneWaitTimer);      
        break; 
      case movingDoneWaitTimer:
        // wait till timer has ended
        delay(movingTimer);
        //if (millis() - movingTimer > randomEndTimer) 
        {
          StateMovingChanged(standby);
        }
        break;
    }    
  }
}

void StateMovingChanged(movingState newState)
{
  currentMovingState = newState;
  switch (currentMovingState)
  {   
    case standby:
      Serial.println(F(" => MovingState = standby"));  
      digitalWrite(LED_BUILTIN, LOW);
      break;  
    case firstLeft:
      Serial.println(F(" => MovingState = firstLeft"));  
      digitalWrite(LED_BUILTIN, HIGH);
      break;    
    case secondRight:
      Serial.println(F(" => MovingState = secondRight"));  
      digitalWrite(LED_BUILTIN, HIGH);
      break;         
    case thirdLeft:
      Serial.println(F(" => MovingState = thirdLeft"));  
      digitalWrite(LED_BUILTIN, HIGH);
      break;      
    case firstRight:
      Serial.println(F(" => MovingState = firstRight"));  
      digitalWrite(LED_BUILTIN, HIGH);
      break;    
    case secondLeft:
      Serial.println(F(" => MovingState = secondLeft"));  
      digitalWrite(LED_BUILTIN, HIGH);
      break;          
    case thirdRight:
      Serial.println(F(" => MovingState = thirdRight"));  
      digitalWrite(LED_BUILTIN, HIGH);
      break;
    case moveLeft:
      Serial.println(F(" => MovingState = moveLeft"));  
      digitalWrite(LED_BUILTIN, HIGH);
      break;          
    case moveRight:
      Serial.println(F(" => MovingState = moveRight"));  
      digitalWrite(LED_BUILTIN, HIGH);
      break;
    case movingDoneWaitTimer:
      Serial.println(F(" => MovingState = movingDoneWaitTimer"));  
      digitalWrite(LED_BUILTIN, LOW);
      break;
  } 
}


int startRandomTimer()
{
  return randomEndTimer = random(500, 3000);
  //Serial.println(randomEndTimer);    
  //return millis(); // start timer
}
