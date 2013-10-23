/*
  DFRkeypad.cpp - Keyboard routines for DFRobot LCD Keypad Shield
  Copyright (C) 2013 Volker Bartheld.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#include "DFRkeypad.h"

const char* DFRkeypad::sKEY[]=                          { "---",       "Right",   "Up", "Down", "Left", "Select", "???" };
const int DFRkeypad::iARV_VALUES[DFRkeypad::eNUM_KEYS]= { 1023,        0,         143,  328,    504,     740,      -1   };
int DFRkeypad::iKEY[DFRkeypad::eNUM_KEYS]=              { 0,           0,         0,    0,      0,       0,         0   };

int DFRkeypad::iDEFAULT_THRESHOLD=50;       // default threshold for keypad ADC and resistor network inaccuracies
int DFRkeypad::iKEY_DEBOUNCE_DELAY=100;     // default debounce time for keys on keypad (100ms)
int DFRkeypad::iKEY_REPEAT_DELAY=250;       // default repeat delay for keys on keypad (250ms)
byte DFRkeypad::LastKey=DFRkeypad::eNO_KEY;
unsigned long DFRkeypad::LastTime=0;
bool DFRkeypad::bRepeat=false;

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void DFRkeypad::FastADC(bool bFast) // modify ADV prescale divider to accelerate analogRead() if necessary
{
  if(bFast)
  {
    // set prescaler to 16 (16MHz ADC clock, divided by prescale 16 divided by 13 ADC clocks per conversion=77kHz sample rate)
    sbi(ADCSRA,ADPS2);
    cbi(ADCSRA,ADPS1);
    cbi(ADCSRA,ADPS0);
  }
  else // if(bFast)
  {
      // set prescaler to 128 (16MHz ADC clock, divided by prescale 128 divided by 13 ADC clocks per conversion=9.6kHz sample rate)
    sbi(ADCSRA,ADPS2);
    sbi(ADCSRA,ADPS1);
    sbi(ADCSRA,ADPS0);
  } // if(bFast) ... else
} // void DFRkeypad::FastADC(bool bFast)

int bounds(int val, int lower, int upper) // restrict val to interval [lower;upper]
{
  if(val<lower) return lower;
  if(val>upper) return upper;
  return val;
} // int bounds(int val, int lower, int upper)

bool isin(int val, int min, int max, int center, int threshold) // find out if value is within +/- threshold around center
{
  return val>=bounds(center-threshold, min, max) && val<=bounds(center+threshold, min, max);
} // bool isin(int val, int min, int max, int center, int threshold)

const char* const DFRkeypad::KeyName(int val) // get verbose key name
{
  return sKEY[bounds(val, eNO_KEY, eINVALID_KEY)];
} // const char* const DFRkeypad::KeyName(int val)

bool DFRkeypad::AnyKey() // quick test if any key is pressed on the keypad
{
  return analogRead(KEYPAD)<1023-iDEFAULT_THRESHOLD; // anything _not_ eNO_KEY is a valid key
} // bool DFRkeypad::AnyKey()

byte DFRkeypad::GetKey() // get key ID on DFR LCD keypad shield
{
  if(bRepeat && millis()<LastTime+iKEY_REPEAT_DELAY) return eNO_KEY;

  int i, count=0, key=eNO_KEY, val;
  memset(iKEY, 0, sizeof(iKEY));                // clear the key histogram

  while(millis()<LastTime+iKEY_DEBOUNCE_DELAY)  // sample analog values from keyboard until debounce time exceeded
  {
    val=analogRead(KEYPAD);                     // read analog value from keyboard resistor array
    for(i=eNO_KEY; i<eNUM_KEYS-1; ++i)
      if(isin(val, 0, 1023, iARV_VALUES[i], iDEFAULT_THRESHOLD)) break; // if within threshold for key, the ID has been found

    ++iKEY[i];                                  // increment histogram bin for this key
  } // while(millis()<LastTime+iKEY_DEBOUNCE_TIME)

  for(i=eNO_KEY; i<eNUM_KEYS-1; ++i)            // enumerate histogram bins
    if(iKEY[i]>count) { key=i; count=iKEY[i]; } // find key ID that has been sampled most of the time during debouncing

  if(eNO_KEY!=key && LastKey==key)   { bRepeat=true;   key=eNO_KEY; }
  else                               { bRepeat=false;   LastKey=key; }

  LastTime=millis();
  return key;
} // byte DFRkeypad::GetKey()
