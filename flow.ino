/*
Project title: Weathered flowers
Description: Takes in the values of wind, temperature, and light and plays music 
based on the values, uses: 
Light sensor: Adafruit VEML7700
Music player: Adafruit VS1053
Wind sensor: Modern Device Wind Sensor Rev. P 

Credit: Example files from Adafruit and Modern Device for each of the sensors
Track001 = 
"Almost Bliss" Kevin MacLeod (incompetech.com)
Licensed under Creative Commons: By Attribution 4.0 License
http://creativecommons.org/licenses/by/4.0/

Track002 = 
"Sincerely" Kevin MacLeod (incompetech.com)
Licensed under Creative Commons: By Attribution 4.0 License
http://creativecommons.org/licenses/by/4.0/

Track003 = 
"Ultralounge" Kevin MacLeod (incompetech.com)
Licensed under Creative Commons: By Attribution 4.0 License
http://creativecommons.org/licenses/by/4.0/

Track004 = 
"Blippy Trance" Kevin MacLeod (incompetech.com)
Licensed under Creative Commons: By Attribution 4.0 License
http://creativecommons.org/licenses/by/4.0/
*/


#include "Adafruit_VEML7700.h"
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <math.h>

/*
* Hardware hookup 
* Sensor     Arduino Pin (not used)
* Ground     Ground
* +10-12V      power supply directly
* Out          A0
* TMP          A2
*/

// define the pins used
//#define CLK 13       // SPI Clock, shared with SD card
//#define MISO 12      // Input data, from VS1053/SD card
//#define MOSI 11      // Output data, to VS1053/SD card
// Connect CLK, MISO and MOSI to hardware SPI pins. 
// See http://arduino.cc/en/Reference/SPI "Connections"

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  // create breakout-example object!
  // Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
  // create shield-example object!
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

const int OutPin = A0;   // wind sensor analog pin  hooked up to Wind P sensor "OUT" pin
const int TempPin = A2;  // temp sesnsor analog pin hooked up to Wind P sensor "TMP" pin

Adafruit_VEML7700 veml = Adafruit_VEML7700();

int loopCount;

void setup() {
  Serial.begin(9600);
  // while (!Serial) { delay(10); }
  Serial.println("Flow Project Start");

  if (!veml.begin()) {
    Serial.println("VEML Sensor not found");
    while (1);
  }
  Serial.println("Sensor found");

  if (! musicPlayer.begin()) { // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1);
  }
  Serial.println(F("VS1053 found"));
  
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(30,30); 
  // lowest volume set to 30, practically inaudible

  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  // init loopCount
  loopCount = 0;
}

void loop() {
  // read wind
  int windADunits = analogRead(OutPin);
  float windMPH = pow((((float)windADunits - 264.0) / 85.6814), 3.36814);
  Serial.print(windMPH);
  Serial.print(" MPH\t");

  // read temp
  int tempRawAD = analogRead(TempPin);
  float tempC = ((((float)tempRawAD * 5.0) / 1024.0) - 0.400) / .0195;
  Serial.print(tempC);
  Serial.println(" C");
  
  // read lux
  float lux = veml.readLux(VEML_LUX_AUTO);
  Serial.print(lux); 
  Serial.println(" lux");

  // normalize values
  float windCalc = min(1, abs(windMPH / 12)); // range 0 - 12, .33 --> .66, predicted weather for 11/20/23 is 5-10 mph = 0.0833333333
  float tempCalc = min(1, abs(22 - tempC) / (44 - 0)); // range 0 - 50°C, predicted weather for 11/20/23 is 9 so 13/50 = 0.26 
  float luxCalc = min(1, abs(1 - lux / (10000 - 0))); // range 0 - 10,000

  // calculate score (smaller number is better)
  float score = windCalc * 0.25 + tempCalc * 0.25 + luxCalc * 0.5;
  // tempCalc * .3 = 0.222 so we will use that as baseline for the music changes
  // windCalc * .4 = 0.264 at 0 windmph; 16mph --> 0; 4mph --> .4; 
  // lux = 10000 --> 1 --> .3
  Serial.print("Score: ");
  Serial.println(score);
  Serial.println("------------------------------");

  // play music based on score
  if(!musicPlayer.playingMusic || loopCount % 20 == 0){ // every 2 min, alter track
    if(score >= 0 && score < .25){
      // play track004
      Serial.println(F("Playing track004"));
      if(strcmp(musicPlayer.currentTrack.name(), "track004.mp3")){
        musicPlayer.stopPlaying();
        musicPlayer.startPlayingFile("/track004.mp3");
      }
    }
    else if(score >= .25 && score < .50){
      // play track003
      Serial.println(F("Playing track003"));
      Serial.print(musicPlayer.currentTrack.name());
      if(strcmp(musicPlayer.currentTrack.name(), "track003.mp3")){
        musicPlayer.stopPlaying();
        musicPlayer.startPlayingFile("/track003.mp3");
      }
    }
    else if(score >= .50 && score < .70){
      // play track002      
      Serial.println(F("Playing track002"));
      if(strcmp(musicPlayer.currentTrack.name(), "track002.mp3")){
        musicPlayer.stopPlaying();
        musicPlayer.startPlayingFile("/track002.mp3");
      }
    }
    else {// score >= 70 && score < 1{
      // play track001      
      Serial.println(F("Playing track001"));
      if(strcmp(musicPlayer.currentTrack.name(), "track001.mp3")){
        musicPlayer.stopPlaying();
        musicPlayer.startPlayingFile("/track001.mp3");
      }
    }
    Serial.println(musicPlayer.stopped());
  }

  float volume = - 1 / pow(50, 2) * pow(score * 100 - 50, 2) + 1;
  volume = 30 * volume + 1;
  Serial.print("Volume: ");
  Serial.println(volume);
  musicPlayer.setVolume(volume, volume);

  // lower number = louder volume!
  // if(loopCount % 2 == 0){ //check every 10 seconds, alter volume
    // if(score >= 0 && score < .35) musicPlayer.setVolume((1-score) * 30, (1-score) * 30);
    // else if(score >= .35 && score < .5) musicPlayer.setVolume(abs((1-score) - .33) * 30, abs((1-score) - .33) * 30);
    // else if(score >= .50 && score < .7) musicPlayer.setVolume(abs((1-score) - .6) * 30, abs((1-score) - .6) * 30);
    // else musicPlayer.setVolume((1-score) * 20, (1-score) * 20);
  // }
  
  loopCount++;
  delay(100);
}
