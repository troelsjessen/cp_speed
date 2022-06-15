#include "Timer.h"

// Constant defines
const int nSensors = 4;
const int pin[nSensors] = {2, 3, 4, 5};
const int pinTimer = 7;
const unsigned long maxLoopTime = 3;
const unsigned int reportInterval = 1000; // ms
const unsigned int numBlocksRequired = 10;
int pinsSet = 0;
int pinsReported = 0;
unsigned long newTime = 0;

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


  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 800;              // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS10);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();
}

ISR(TIMER1_COMPA_vect)
{
  for(int s = 0; s < nSensors; s++)
  {
    if(digitalRead(pin[s]) == HIGH) 
    {
      if(blockedCounter[s] == numBlocksRequired - 1) // -1 to avoid overriding newTime
      {
        {
          newTime = millis();
          pinsSet |= 1 << s;
        }
      }
      else if(blockedCounter[s] < numBlocksRequired)
      {
        blockedCounter[s]++;
      }
    }
    else 
    {
      if(blockedCounter[s] == 0)
      {
        pinsSet &= ~(1 << s);
        pinsReported &= ~(1 << s);
      }
      else
      {
        blockedCounter[s]--;
      }
      lastUnblocked[s] = millis();
    }
  }
}

unsigned long lastTime = 0;
void loop() 
{
  t.update();

  // Iterate through sensors to detect 
  for(int s = 0; s < nSensors; s++)
  {
    if((pinsSet & (1 << s)) != 0 && (pinsReported & (1 << s)) == 0)
    {
      pinsReported |= 1 << s;
      Serial.write("G,");
      Serial.print(s+1);
      Serial.write(",");
      Serial.print(newTime);
      Serial.write(".\r\n");
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
