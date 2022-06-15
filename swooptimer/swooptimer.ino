#include "Timer.h"

// Constant defines
const int nSensors = 4;
const int pin[nSensors] = {2, 3, 4, 5};
const int pinTimer = 7;
const unsigned long maxLoopTime = 3;
const unsigned int reportInterval = 1000; // ms
const unsigned int numBlocksRequired = 200;

// Global variables
int blockedCounter[nSensors];
unsigned long lastUnblocked[nSensors];
Timer t;

void setup() 
{
  Serial.begin(38400);
  for(int s = 0; s < nSensors; s++)
  {
    pinMode(pin[s], INPUT);
  }

  pinMode(pinTimer, OUTPUT);
  t.oscillate(pinTimer, 100, LOW);
  t.every(reportInterval, reportQuality);

}

unsigned long lastTime = 0;
void loop() 
{
  // Check if loop is taking too long
  unsigned long newTime = millis();
  if(lastTime + maxLoopTime < millis())
  {
    Serial.write("D,");
    Serial.print(lastTime);
    Serial.write(",");
    Serial.print(newTime);
    Serial.write(".\r\n");

    // Reading time after printing delay error to avoid infinite loop
    newTime = millis();
  }
  lastTime = newTime;

  t.update();

  // Iterate through sensors to detect 
  for(int s = 0; s < nSensors; s++)
  {
    if(digitalRead(pin[s]) == HIGH) 
    {
      if(blockedCounter[s] == numBlocksRequired)
      {
        Serial.write("G,");
        Serial.print(s+1);
        Serial.write(",");
        Serial.print(newTime);
        Serial.write(".\r\n");
      }
      if(blockedCounter[s] < numBlocksRequired+1)
      {
        blockedCounter[s]++;
      }
    }
    else
    {
      blockedCounter[s] = 0;
      lastUnblocked[s] = newTime;
    }
  }
}

const unsigned long maxQuality = 1023;
void reportQuality()
{
  Serial.write("S,0,");
  for(int s = 0; s < nSensors; s++)
  {
    unsigned long diff = (millis() - lastUnblocked[s]) / 5;
    if(diff > maxQuality) diff = maxQuality;
    Serial.print(maxQuality - diff);      
    Serial.write(",");
  }
  Serial.write("0,0,0,0.\r\n");
}
