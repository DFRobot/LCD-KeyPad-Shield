#include <LiquidCrystal.h>
#include "DFRkeypad.h"
// #define DEBUG // uncomment this if you want verbose console logging

enum ePins { LCD_RS=8, LCD_EN=9, LCD_D4=4, LCD_D5=5, LCD_D6=6, LCD_D7=7, LCD_BL=10 }; // define LCD pins
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7); // initialize the library with the numbers of the interface pins

byte digits(unsigned int iNum) // calculate the number of digits
{
  byte bDigits=0;
  do
  {
    ++bDigits;
    iNum/=10;
  } while(iNum);
  return bDigits;
}

void printNumber(unsigned int number, byte numd) // print fixed-width uint number
{
  byte d=digits(number);
  for(byte i=d; i<numd; ++i) lcd.print((char)' '); // padding
  lcd.print(number);
}

byte lastKey=DFRkeypad::eINVALID_KEY;
float sum[DFRkeypad::eNUM_KEYS], sumq[DFRkeypad::eNUM_KEYS];  // arrays to sum up values for mean and deviation
unsigned int values[DFRkeypad::eNUM_KEYS];                    // counter for number of samples
unsigned long LastTime;
enum eSymbols { symPLUSMINUS=0 };                             // deviation LCD Symbol "+/-"
byte char_plusminus[8]=
  {
  B00100,
  B00100,
  B11111,
  B00100,
  B00100,
  B00000,
  B11111,
  B00000,
  };

void clearStat() // clear statistics
{
  memset(sum, 0, sizeof(sum));
  memset(sumq, 0, sizeof(sumq));
  memset(values, 0, sizeof(values));
}

void setup()
{
	#ifdef DEBUG
	Serial.begin(9600);
	Serial.println("* Setup...");
	#endif

  lcd.begin(16, 2); 						                  // set up the LCD's number of columns and rows (16x2)
  lcd.createChar(symPLUSMINUS, char_plusminus);   // create +/- character

	pinMode(LCD_BL, OUTPUT); 			                  // pin LCD_BL is LCD backlight brightness (PWM)
	analogWrite(LCD_BL, 255);                       // set the PWM brightness to maximum
  lcd.setCursor(0, 0);
  lcd.print("DFR Keypad Test");

	DFRkeypad::FastADC(true);                       // increase ADC sample frequency
  DFRkeypad::iDEFAULT_THRESHOLD=140;              // maximum threshold acceptable so bounds in DFRkeypad::iARV_VALUES are not overlapping
  clearStat();                                    // clear statistics
  LastTime=millis();                              // remember tick counter

  #ifdef DEBUG
	Serial.println("  Done.");
	#endif
}

void loop()
{
	byte key=DFRkeypad::GetKey();                   // read a key identifier

  #ifdef DEBUG
	Serial.print("*  Key: ");
	Serial.println(DFRkeypad::KeyName(key));
  #endif

  if(DFRkeypad::eNO_KEY<key && key<DFRkeypad::eINVALID_KEY) // if a valid key has been identified
  {
    int val=analogRead(KEYPAD);                             // ... get the analog value for it
    sum[key]+=val;                                          // add val into array
    sumq[key]+=(float)val*(float)val;                       // add val^2 into array
    ++values[key];                                          // increase sample counter

    #ifdef DEBUG
    Serial.print("*  ADC Value: ");
    Serial.println(val);
    #endif

    if(millis()>LastTime+1000)                              // if sampled for one second...
    {
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(DFRkeypad::KeyName(key));                                               // print key name

      lcd.setCursor(7, 1);
      float mean=sum[key]/values[key], deviation=sqrt(sumq[key]/values[key]-mean*mean); // calculate ADC mean and deviation for that key
      printNumber((unsigned int)mean, 4);                                               // print mean value (4 digits)
      lcd.write((uint8_t)symPLUSMINUS);
      printNumber((unsigned int)deviation, 4);                                          // print deviation value (4 digits)

      #ifdef DEBUG
      Serial.print("*  ADC Mean: ");
      Serial.println((unsigned int)mean);
      Serial.print("*  ADC Deviation: ");
      Serial.println((unsigned int)deviation);
      #endif

      LastTime=millis();
    } // if(millis()>LastTime+1000)
  } // if(DFRkeypad::eNO_KEY<key && key<DFRkeypad::eINVALID_KEY)

  if(millis()>LastTime+5000)        // if 5s have passed...
  {
    clearStat();                    // clear statistics
    lcd.setCursor(0, 1);
    lcd.print("                ");  // blank out row
    lcd.setCursor(0, 1);
    LastTime=millis();
  }

} // void loop()
