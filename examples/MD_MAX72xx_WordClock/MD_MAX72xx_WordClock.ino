// Program to implement a Word Clock using the MD_MAX72XX library
//
// The word clock is modelled on the one found at the Adafruit web site
// at https://learn.adafruit.com/neomatrix-8x8-word-clock/overview, but 
// uses a standard 8x8 LED matrix module.
//
// The clock face (word matrix) for the clock can be found in the doc 
// folder of this example (Microsoft Word document and PDF versions). 
// The mask was printed on standard paper and placed over the matrix 
// LEDs, folding over the small flaps on the sides and attaching them 
// to the side of the matrix using double sided tape.
//
// This example has dependencies on the MD_DS1307 RTC library available
// from https://arduinocode.codeplex.com/releases
//

#include <MD_MAX72xx.h>
#include <MD_DS1307.h>
#include <Wire.h>       // I2C library for RTC
#if USE_LIBRARY_SPI
#include <SPI.h>
#endif

// Turn on debug statements to the serial output
#define  DEBUG  0

#if  DEBUG
#define	PRINT(s, x)	{ Serial.print(F(s)); Serial.print(x); }
#define	PRINTS(x)	Serial.print(F(x))
#define	PRINTD(x)	Serial.println(x, DEC)

#else
#define	PRINT(s, x)
#define PRINTS(x)
#define PRINTD(x)

#endif

// Define the number of devices we have in the chain and the hardware interface
// NOTE: For non-native hardware interface the pins will probably not work with 
// your hardware and may need to be adapted.
#define	MAX_DEVICES	1

#define	CLK_PIN		13  // or SCK
#define	DATA_PIN	11  // or MOSI
#define	CS_PIN		10  // or SS

// Global variables
MD_MAX72XX clock = MD_MAX72XX(CS_PIN, MAX_DEVICES);  // SPI hardware interface
//MD_MAX72XX clock = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES); // Arbitrary pins

// -------------------------------------
// Define the data structure for the words on the clock face. 
// The clock face has the following letter matrix
// 7 6 5 4 3 2 1 0  <-- column
// A T W E N T Y D  <-- row 0
// Q U A R T E R Y  <-- row 1
// F I V E H A L F  <-- row 2
// D P A S T O R O  <-- row 3
// F I V E I G H T  <-- row 4
// S I X T H R E E  <-- row 5
// T W E L E V E N  <-- row 6
// F O U R N I N E  <-- row 7 
//
// The words are defined in one or 2 rows. So to define the bit 
// pattern for a word, just need to know the row number(s) and 
// the bit pattern(s) to turn on.
typedef struct clockWord_t
{
  uint8_t row;
  uint8_t data;
};

const clockWord_t M_05 = { 2, 0b11110000 };
const clockWord_t M_10 = { 0, 0b01011000 };
const clockWord_t M_15 = { 1, 0b11111110 };
const clockWord_t M_20 = { 0, 0b01111110 };
const clockWord_t M_30 = { 2, 0b00001111 };

const clockWord_t TO = { 3, 0b00001100 };
const clockWord_t PAST = { 3, 0b01111000 };

const clockWord_t H_01[] = { { 7, 0b01000011 } };
const clockWord_t H_02[] = { { 6, 0b11000000 }, { 7, 0b01000000 } };
const clockWord_t H_03[] = { { 5, 0b00011111 } };
const clockWord_t H_04[] = { { 7, 0b11110000 } };
const clockWord_t H_05[] = { { 4, 0b11110000 } };
const clockWord_t H_06[] = { { 5, 0b11100000 } };
const clockWord_t H_07[] = { { 5, 0b1000000 }, { 6, 0b00001111 } };
const clockWord_t H_08[] = { { 4, 0b00011111 } };
const clockWord_t H_09[] = { { 7, 0b00001111 } };
const clockWord_t H_10[] = { { 6, 0b10000011 } };
const clockWord_t H_11[] = { { 6, 0b00111111 } };
const clockWord_t H_12[] = { { 6, 0b11110110 } };

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

const uint8_t CLOCK_UPDATE_TIME = 5;   // in seconds - display resolution to nearest 5 minutes does not need rapid updates

void displayTime()
{
  PRINTS("\nT: ");
  if (RTC.h < 10) PRINTS("0");
  PRINT("", RTC.h);
  PRINTS(":");
  if (RTC.m < 10) PRINTS("0");
  PRINT("", RTC.m);
  PRINTS(":");
  if (RTC.s < 10) PRINTS("0");
  PRINT("", RTC.s);
}

void updateClock(void)
// Work out what current time it is in words and turn on the right
// parts of the display.

{
  clockWord_t *H;
  uint8_t numElements;

  displayTime();  // debug output only

  // free the clock display while we make changes to the matrix
  clock.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  clock.clear();

  // minutes -  are worked out in band around the time to select 
  // the choice of words. This tries to copy the approximations 
  // people make when reading analog time. It is consistent but 
  // arbitrary - note that any changes need to be made consistently 
  // across all the checks in this part of the code
  switch (RTC.m)
  {
  case 0 ... 2:  
  case 58 ... 59:  break;  // nothing to say
  case 3 ... 7:  
  case 53 ... 57:  clock.setRow(M_05.row, M_05.data);  PRINTS(" FIVE"); break;
  case 8 ... 12: 
  case 48 ... 52:  clock.setRow(M_10.row, M_10.data);  PRINTS(" TEN"); break;
  case 13 ... 17: 
  case 43 ... 47:  clock.setRow(M_15.row, M_15.data);  PRINTS(" QUARTER");   break;
  case 18 ... 22: 
  case 38 ... 42:  clock.setRow(M_20.row, M_20.data);  PRINTS(" TWENTY"); break;
  case 23 ... 27:  
  case 33 ... 37:  clock.setRow(M_05.row, M_05.data); clock.setRow(M_20.row, M_20.data); PRINTS(" TWENTY-FIVE"); break;
  case 28 ... 32:  clock.setRow(M_30.row, M_30.data);  PRINTS(" HALF"); break;
  }

  // to/past
  if (RTC.m > 2 && RTC.m < 58)  // top of the hour displays nothing
  {
    if (RTC.m <= 32)  // before the half hour it is 'past'
    {
      clock.setRow(PAST.row, PAST.data);
      PRINTS(" PAST ");
    }
    else    // after the half hour it becomes 'to' and we have to adjust the hour!
    {
      clock.setRow(TO.row, TO.data);
      if (RTC.h < 12) RTC.h++;
      else RTC.h = 1;
      PRINTS(" TO ");
    }
  }

  // hour - can span more thn one line so the setup data is in arrays
  switch (RTC.h)
  {
  case  1: H = (clockWord_t *)H_01;  numElements = ARRAY_SIZE(H_01);  PRINTS("ONE");  break;
  case  2: H = (clockWord_t *)H_02;  numElements = ARRAY_SIZE(H_02);  PRINTS("TWO");  break;
  case  3: H = (clockWord_t *)H_03;  numElements = ARRAY_SIZE(H_03);  PRINTS("THREE");  break;
  case  4: H = (clockWord_t *)H_04;  numElements = ARRAY_SIZE(H_04);  PRINTS("FOUR");  break;
  case  5: H = (clockWord_t *)H_05;  numElements = ARRAY_SIZE(H_05);  PRINTS("FIVE");  break;
  case  6: H = (clockWord_t *)H_06;  numElements = ARRAY_SIZE(H_06);  PRINTS("SIX");  break;
  case  7: H = (clockWord_t *)H_07;  numElements = ARRAY_SIZE(H_07);  PRINTS("SEVEN");  break;
  case  8: H = (clockWord_t *)H_08;  numElements = ARRAY_SIZE(H_08);  PRINTS("EIGHT");  break;
  case  9: H = (clockWord_t *)H_09;  numElements = ARRAY_SIZE(H_09);  PRINTS("NINE");  break;
  case 10: H = (clockWord_t *)H_10;  numElements = ARRAY_SIZE(H_10);  PRINTS("TEN");  break;
  case 11: H = (clockWord_t *)H_11;  numElements = ARRAY_SIZE(H_11);  PRINTS("ELEVEN");  break;
  case 12: H = (clockWord_t *)H_12;  numElements = ARRAY_SIZE(H_12);  PRINTS("TWELVE");  break;
  }
  for (uint8_t i = 0; i < numElements; i++)
    clock.setRow(H[i].row, H[i].data);

  // finally, update the display with new data
  clock.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void setup()
{
#if  DEBUG
  Serial.begin(115200);
#endif
  PRINTS("\n[MD_MAX72XX_WordClock Demo]");

  clock.begin();

  // turn the clock on to 12H mode, 
  RTC.control(DS1307_12H, DS1307_ON);
  if (!RTC.isRunning())
    RTC.control(DS1307_CLOCK_HALT, DS1307_OFF);
}

void loop() 
{
  static uint32_t timeLastUpdate = 0;

  if (millis() - timeLastUpdate >= CLOCK_UPDATE_TIME * 1000UL)
  {
    RTC.readTime();
    updateClock();
    timeLastUpdate = millis();
  }
}

