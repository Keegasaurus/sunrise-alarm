#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

const int button_pin = 10;
const int led_pin = 6;

unsigned int fade_in_time = 5; // in minutes

unsigned int increment;

unsigned int h = 8;  // 0 to 23
unsigned int m = 45; // 0 to 59

volatile boolean active = true;

void button_ISR() { active = false; }

void setup() 
{
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  
  Serial.begin(9600); // default; opens serial port and 
                      // set bit rate to 9600
  if (!rtc.begin()) 
  {
    Serial.println("RTC not found.");
    Serial.flush();
    abort();
  }
  
  if (rtc.lostPower()) 
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  fade_in_time = (fade_in_time < 60 && fade_in_time > 1) ? fade_in_time : 10; // limit to 60 
                                                                              // and default to 10
  if (m - fade_in_time < 0) 
  {
    m = 60 - m - fade_in_time;
    h = h - 1;
  } else
  {
    m = m - fade_in_time; 
  }

  increment = fade_in_time * 60000 / 256; // mins x millis per minute / levels to climb 
                                          // = millis per level
                                          // sunrise will lighly undershoot fade_in_time unless
                                          // a multiple of 8

  attachInterrupt(digitalPinToInterrupt(button_pin), button_ISR, FALLING);  
}

void loop() 
{
  DateTime present = rtc.now();

  if ((unsigned int)present.hour() == h && (unsigned int)present.minute() == m)
  {
    if (present.dayOfTheWeek() % 6) // not zero (Sunday) nor 6 (Saturday)
    {
      int i=0;
      while (i<255 && active)
      {
        analogWrite(led_pin, i++);
        delay(increment);
      }
      while (i>0) 
      {
        analogWrite(led_pin, i--);
        delay(10); // fade out between two and three seconds (if uninterrupted)
      }
      active = true;
    }
  }
}
