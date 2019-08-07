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
 2.This code is tested on Arduino Uno board
 ****************************************************/

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Stepper.h>

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
void PulseLed();

boolean resetSensor = false;
boolean playing = false; 

#define PIN_OUTPUT_pulse_LED              9
#define PIN_OUTPUT_voice_LED              3
#define PIN_OUTPUT_holo_LED               6
#define PIN_OUTPUT_slaveArduinoNano       8

#define PIN_INPUT_sound                  A3
#define PIN_INPUT_surround_sound         A2
#define PIN_INPUT_voice_busy             12

#define VOLUME                           25 // From 0 to 30
#define TRACK_COUNTER_SWITCH             10
#define TIME_BETWEEN_PLAY              2000
#define TIME_MOVING                     500

int numberOfTracks = 0;
int pulseBrightness = 0;
int holoBrightness = 0;
int voiceBrightness = 0;
int trackCounter = 0;
int currentTrack = 0;
int randomEndTimer = 0; 

long playTimer = 0;

enum soundState 
{
  idle,
  playingTrack,
  startPlayingMessage,
  playingMessageTrack,
  waitPlayingMessageTrack,  
  stopped,
  waitTimer,
  startPlayingTrack,
};

soundState currentSoundState = idle;

SoftwareSerial mySoftwareSerial(10, 11);           // 10=RX, 11=TX

void setup()
{
  // output pins
  pinMode(PIN_OUTPUT_pulse_LED, OUTPUT);
  pinMode(PIN_OUTPUT_voice_LED, OUTPUT);
  pinMode(PIN_OUTPUT_holo_LED, OUTPUT);
  pinMode(PIN_OUTPUT_slaveArduinoNano, OUTPUT);  

  // input pins
  pinMode(PIN_INPUT_sound, INPUT);
  pinMode(PIN_INPUT_voice_busy, INPUT);
 
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);

  // init the MP3 player
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(VOLUME);  //Set volume value. 
  numberOfTracks = myDFPlayer.readFileCounts();   
  randomSeed(analogRead(PIN_INPUT_sound));

  Serial.println((numberOfTracks));
  StateSoundChanged(idle);
}

void loop()
{
  static unsigned long soundTimer = millis();

  PulseLed();

  switch (currentSoundState)
  {
    case idle:
      PlayATrack();
      //StateChanged(waitPlayingTrack);
      StateSoundChanged(playingTrack);
      break;
    case playingTrack:
      if (digitalRead(PIN_INPUT_voice_busy) == LOW)
      {
        VoiceLedOn();
      }
      else
      {
        if (myDFPlayer.available()) 
        {
          if ((myDFPlayer.readType() == DFPlayerPlayFinished) && (myDFPlayer.read() == currentTrack))
          {
            printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
            StateSoundChanged(stopped);
          }
        }        
      }
      break;
    case startPlayingMessage:
      myDFPlayer.playFolder(01, 001);
      playTimer = millis();
      StateSoundChanged(waitPlayingMessageTrack);
      break;
    case waitPlayingMessageTrack:
      HoloLedOn();
      if (millis() - playTimer > TIME_BETWEEN_PLAY) 
      {        
        StateSoundChanged(playingMessageTrack);        
      }
      break;
    case playingMessageTrack:
      if (digitalRead(PIN_INPUT_voice_busy) == LOW)
      {
        HoloLedOn();
      }
      else
      {
        if (myDFPlayer.available()) 
        {
          if ((myDFPlayer.readType() == DFPlayerPlayFinished))
          {
            HoloLedOff();
            StateSoundChanged(stopped);
          }
        }        
      }
      break;      
    case startPlayingTrack:
      if (trackCounter != TRACK_COUNTER_SWITCH)
      {
        // Continue moving head
        digitalWrite(PIN_OUTPUT_slaveArduinoNano, LOW);
        trackCounter ++;
        PlayATrack();
        //StateChanged(waitPlayingTrack);
        StateSoundChanged(playingTrack);
      }
      else
      {
        trackCounter = 0;
        // Stop moving head
        digitalWrite(PIN_OUTPUT_slaveArduinoNano, HIGH);
        StateSoundChanged(startPlayingMessage);
      }
      break;
    case stopped:
      soundTimer = startRandomTimer();
      StateSoundChanged(waitTimer);
      break;
    case waitTimer:
      // wait till timer has ended
      if (millis() - soundTimer > randomEndTimer) 
      {
        StateSoundChanged(startPlayingTrack);
      }
      break;  
  } 
}

int startRandomTimer()
{
  randomEndTimer = random(500, 3000);
  return millis(); // start timer
}

void PlayATrack()
{
  currentTrack = random(1, numberOfTracks);
  myDFPlayer.play(currentTrack);
  //myDFPlayer.next();  
  playTimer = millis();
  Serial.print(F("Play track number: "));  
  Serial.println(currentTrack);
}

void StateSoundChanged(soundState newState)
{
  currentSoundState = newState;
  switch (currentSoundState)
  {
    case idle:
      Serial.println(F(" +> SoundState = Idle"));  
      break;
    case playingTrack:
      Serial.println(F(" => SoundState = PlayingTrack"));  
      break;
    case startPlayingMessage:
      Serial.println(F(" -> SoundState = startPlayingMessage"));  
      break;
    case waitPlayingMessageTrack:
      Serial.println(F(" -> SoundState = waitPlayingMessageTrack"));  
      break;
    case playingMessageTrack:
      Serial.println(F(" -> SoundState = playingMessageTrack"));  
      break;      
    case startPlayingTrack:
      Serial.println(F(" => SoundState = startPlayingTrack"));  
      break;
    case stopped:
      Serial.println(F(" => SoundState = Stopped"));  
      break;
    case waitTimer:
      Serial.println(F(" => SoundState = waitTimer"));  
      break;     
  } 
}

void HoloLedOn()
{
   holoBrightness = 255+(sin(millis()/10.00)*50);
   analogWrite(PIN_OUTPUT_holo_LED, holoBrightness);   
}

void HoloLedOff()
{
   analogWrite(PIN_OUTPUT_holo_LED, 0);   
}

void VoiceLedOn()
{
  // Sets the brightness of the light based on the loudness of the voice
  voiceBrightness = constrain(map(analogRead(PIN_INPUT_sound), 400, 500, 0, 255), 255, 0);
  //Serial.println(analogRead(PIN_INPUT_sound));
  analogWrite(PIN_OUTPUT_voice_LED, voiceBrightness);  
}

void PulseLed()
{
   pulseBrightness = 170+(sin(millis()/400.00)*80);
   analogWrite(PIN_OUTPUT_pulse_LED, pulseBrightness); 
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

}
