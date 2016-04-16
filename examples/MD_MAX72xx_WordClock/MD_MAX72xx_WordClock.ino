// Program to implement a Word Clock using the MD_MAX72XX library.
// This is a full-featured and complex example!
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
// To see the time in digits, press the setup switch once.
//
// To set up the time:
// - Double click the setup switch
// - Then click to progress the hours
// - Double click to stop editing hours and edit minutes
// - Then click to progress the minutes
// - Double click to exit editing and set the new time
// Setup mode has a timeout for no inactivity, seting new time and 
// returning to normal word display.
//
// This example has dependencies on the MD_DS1307 RTC library available
// from https://arduinocode.codeplex.com/releases. Any other RTC may be 
// substitiuted with few changes as the current time is passed to all 
// matrix display functions.
//
// Dependency also on the MD_KeySwitch library to handle tact switch 
// input, found at https://arduinocode.codeplex.com/releases.
//

#include <MD_MAX72xx.h>
#include <MD_KeySwitch.h>
#include <MD_DS1307.h>
#include <Wire.h>       // I2C library for RTC
#if USE_LIBRARY_SPI
#include <SPI.h>
#endif

// --------------------------------------
// Hardware definitions
// // NOTE: For non-integrated SPI interface the pins will probably 
// not work with your hardware and may need to be adapted.
const uint8_t CLK_PIN = 13;  // or SCK
const uint8_t DATA_PIN = 11; // or MOSI
const uint8_t CS_PIN = 10;   // or SS

const uint8_t SETUP_SW_PIN = 4; // setup switch pin

// --------------------------------------
// Miscelaneous defines
#define  DEBUG  0
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
const uint8_t CLOCK_UPDATE_TIME = 5;   // in seconds - display resolution to nearest 5 minutes does not need rapid updates
const uint32_t SHOW_DELAY_TIME = 1000;  // in millisecnds - how long to show time in digits
const uint32_t SETUP_TIMEOUT = 10000;   // in milliseconds - timeout for setup mode

// --------------------------------------
// Global variables
MD_KeySwitch  swSetup(SETUP_SW_PIN);        // setup switch object
MD_MAX72XX clock = MD_MAX72XX(CS_PIN, 1);   // SPI hardware interface
//MD_MAX72XX clock = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, 1); // Arbitrary pins

// --------------------------------------
// *** END OF USER CONFIG INFORMATION ***
// --------------------------------------

#if  DEBUG
#define	PRINT(s, x)	{ Serial.print(F(s)); Serial.print(x); }
#define	PRINTS(x)	Serial.print(F(x))
#define	PRINTD(x)	Serial.println(x, DEC)
#else
#define	PRINT(s, x)
#define PRINTS(x)
#define PRINTD(x)
#endif

// --------------------------------------
// Font data used to set the time on the clock.
// the characters are 4 pixels wide so that 2 can 
// fit on the display by shifting the data for the 
// leftmost character and 'OR'ing in the rightmost
// character.
// Font data is stored in rows.

#define FONT_ROWS 8

const PROGMEM uint8_t fontMap[][FONT_ROWS] =
{
  { 0x7, 0x5, 0x5, 0x5, 0x5, 0x5, 0x7, 0x0 }, // 0
  { 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0 }, // 1
  { 0x7, 0x1, 0x1, 0x7, 0x4, 0x4, 0x7, 0x0 }, // 2
  { 0x7, 0x1, 0x1, 0x7, 0x1, 0x1, 0x7, 0x0 }, // 3
  { 0x4, 0x4, 0x5, 0x5, 0x7, 0x1, 0x1, 0x0 }, // 4
  { 0x7, 0x4, 0x4, 0x7, 0x1, 0x1, 0x7, 0x0 }, // 5
  { 0x7, 0x4, 0x4, 0x7, 0x5, 0x5, 0x7, 0x0 }, // 6
  { 0x7, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0 }, // 7
  { 0x7, 0x5, 0x5, 0x7, 0x5, 0x5, 0x7, 0x0 }, // 8
  { 0x7, 0x5, 0x5, 0x7, 0x1, 0x1, 0x7, 0x0 }, // 9
};

// --------------------------------------
// Define the data for the words on the clock face. 
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
// - Minutes to/past the hour are all in the rows 0-2 of 
// the display.
// - Past/to text is on row 3
// - The hour name is in rows 4-7
//
// The words may be defined in one or more rows. So to 
// define the bit pattern to illuminate for a word, just 
// need to know the row number(s) and the bit pattern(s) 
// to turn on for that row.
typedef struct clockWord_t
{
  uint8_t row;
  uint8_t data;
};

// Minutes and to/past are all on the same row
const clockWord_t M_05 = { 2, 0b11110000 };
const clockWord_t M_10 = { 0, 0b01011000 };
const clockWord_t M_15 = { 1, 0b11111110 };
const clockWord_t M_20 = { 0, 0b01111110 };
const clockWord_t M_30 = { 2, 0b00001111 };

const clockWord_t TO = { 3, 0b00001100 };
const clockWord_t PAST = { 3, 0b01111000 };

// Some hour names have some split across rows, so use more
// than one definition per word - make them all arrays for 
// consistent handling in code.
const clockWord_t H_01[] = { { 7, 0b01000011 } };
const clockWord_t H_02[] = { { 6, 0b11000000 }, { 7, 0b01000000 } };
const clockWord_t H_03[] = { { 5, 0b00011111 } };
const clockWord_t H_04[] = { { 7, 0b11110000 } };
const clockWord_t H_05[] = { { 4, 0b11110000 } };
const clockWord_t H_06[] = { { 5, 0b11100000 } };
const clockWord_t H_07[] = { { 5, 0b10000000 }, { 6, 0b00001111 } };
const clockWord_t H_08[] = { { 4, 0b00011111 } };
const clockWord_t H_09[] = { { 7, 0b00001111 } };
// const clockWord_t H_10[] = { { 6, 0b10000011 } };	// horizontal option
const clockWord_t H_10[] = { { 4, 0b00000001 }, { 5, 0b00000001 }, { 6, 0b00000001 } };		// vertical option
const clockWord_t H_11[] = { { 6, 0b00111111 } };
const clockWord_t H_12[] = { { 6, 0b11110110 } };

// --------------------------------------
// Code
void displayTime()
// Display current time to the debug display
{
  if (RTC.h < 10) PRINTS("0");
  PRINT("", RTC.h);
  PRINTS(":");
  if (RTC.m < 10) PRINTS("0");
  PRINT("", RTC.m);
  PRINTS(":");
  if (RTC.s < 10) PRINTS("0");
  PRINT("", RTC.s);
  PRINTS(" ");
}

void mapBuild(uint8_t *map, uint8_t num)
// *map is a pointer to a FONT_ROWS byte buffer to capture the 
// rows of the mapped number, num is the decimal number to convert
{
  uint8_t hi = num / 10;
  uint8_t lo = num % 10;

  for (uint8_t i = 0; i < FONT_ROWS; i++)
  {
    *map = pgm_read_byte(&fontMap[hi][i]) << 4;
    *map |= pgm_read_byte(&fontMap[lo][i]);
    map++;
  }
}

void mapShow(uint8_t *map)
// *map is a pointer to a FONT_ROWS byte buffer to display on the
// clock face.
{
  clock.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  clock.clear();

  for (uint8_t i = 0; i < FONT_ROWS; i++)
    clock.setRow(i, *map++);

  clock.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void setupTime(uint8_t &h, uint8_t &m)
// Handle the user interface to set the current time 
{
  uint32_t  timeLastActivity = millis();
  uint8_t map[FONT_ROWS];
  uint8_t state = 0;

  while (state != 99)
  {
    // check if we time out
    if (millis() - timeLastActivity >= SETUP_TIMEOUT)
    {
      PRINTS("\nSetup inactivity timeout");
      state = 99;
    }

    // process current state
    switch (state)
    {
    case 0:   // show the hour
      mapBuild(map, h);
      mapShow(map);
      state = 1;
      break;

    case 1:   // handle setting hours
      switch (swSetup.read())
      {
      case MD_KeySwitch::KS_DPRESS:   // move on to minutes
        timeLastActivity = millis();
        state = 2;
        break;
      case MD_KeySwitch::KS_PRESS:    // increment the hours
        timeLastActivity = millis();
        h++;
        if (h == 13) h = 1;
        mapBuild(map, h);
        mapShow(map);
        break;
      }
      break;

    case 2:   // show the minutes
      mapBuild(map, m);
      mapShow(map);
      state = 3;
      break;

    case 3:   // handle setting minutes
      switch (swSetup.read())
      {
      case MD_KeySwitch::KS_DPRESS:   // move on to end
        timeLastActivity = millis();
        state = 4;
        break;
      case MD_KeySwitch::KS_PRESS:    // increment the minutes
        timeLastActivity = millis();
        m = (m + 1) % 60;
        mapBuild(map, m);
        mapShow(map);
        break;
      }
      break;

    default:  // our work is done
      state = 99;
    }
  }
}

void showTime(uint8_t h, uint8_t m)
// Display the current time in digits on the matrix
{
  uint8_t map[FONT_ROWS];

  mapBuild(map, h);
  mapShow(map);
  delay(SHOW_DELAY_TIME);
  mapBuild(map, m);
  mapShow(map);
  delay(SHOW_DELAY_TIME);
}

void updateClock(uint8_t h, uint8_t m)
// Work out what current time it is in words and turn on the right
// parts of the display. The time is passed to the function so that
// it is dependent of the time source.

{
  PRINTS("\nT: ");
  displayTime();  // debug output only

  // free the clock display while we make changes to the matrix
  clock.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  clock.clear();

  // minutes -  are worked out in band around the time to select 
  // the choice of words. This tries to copy the approximations 
  // people make when reading analog time. It is consistent but 
  // arbitrary - note that any changes need to be made consistently 
  // across all the checks in this part of the code
  switch (m)
  {
  case 0 ... 2:  
  case 58 ... 59:  break;  // nothing to say
  case 3 ... 7:  
  case 53 ... 57:  clock.setRow(M_05.row, M_05.data);  PRINTS("FIVE"); break;
  case 8 ... 12: 
  case 48 ... 52:  clock.setRow(M_10.row, M_10.data);  PRINTS("TEN"); break;
  case 13 ... 17: 
  case 43 ... 47:  clock.setRow(M_15.row, M_15.data);  PRINTS("QUARTER");   break;
  case 18 ... 22: 
  case 38 ... 42:  clock.setRow(M_20.row, M_20.data);  PRINTS("TWENTY"); break;
  case 23 ... 27:  
  case 33 ... 37:  clock.setRow(M_05.row, M_05.data); clock.setRow(M_20.row, M_20.data); PRINTS(" TWENTY-FIVE"); break;
  case 28 ... 32:  clock.setRow(M_30.row, M_30.data);  PRINTS("HALF"); break;
  }

  // To/past display
  // Note that after the half hour we have also have to adjust the hour number!
  if (m > 2 && m < 58)  // top of the hour displays the hour only
  {
    if (m <= 32)  // in the first half hour it is 'past' and ...
    {
      clock.setRow(PAST.row, PAST.data);
      PRINTS(" PAST ");
    }
    else    // ... after the half hour it becomes 'to'
    {
      clock.setRow(TO.row, TO.data);
      PRINTS(" TO ");
    }
  }

  if (m > 32)		// adjust the hour
  {
    if (h < 12) h++;
    else h = 1;
  }

  // hour - can span more than one line so the setup data is in arrays
  {
	clockWord_t *H;
	uint8_t numElements;

	switch (h)
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
  }

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
  swSetup.begin();

  // turn the clock on to 12H mode, and start it if not running
  RTC.control(DS1307_12H, DS1307_ON);
  if (!RTC.isRunning())
    RTC.control(DS1307_CLOCK_HALT, DS1307_OFF);
}

void loop() 
{
  static uint8_t  state = 0;
  static uint32_t timeLastUpdate = 0;

  switch (state)
  {
  case 0:   // update the display
    timeLastUpdate = millis();
    RTC.readTime();
    updateClock(RTC.h, RTC.m);
    state = 1;
    break;

  case 1:   // wait for ...
    // ... update time or ...
    if (millis() - timeLastUpdate >= CLOCK_UPDATE_TIME * 1000UL)
      state = 0;

    // ... user input
    switch (swSetup.read())
    {
    case MD_KeySwitch::KS_DPRESS: state = 2; break;
    case MD_KeySwitch::KS_PRESS:  state = 3; break;
    }
    break;

  case 2:   // time setup
    setupTime(RTC.h, RTC.m);
    // write new time to the RTC
    RTC.s = 0;
    RTC.writeTime();
    PRINTS("\nNew T: ");
    displayTime();
    state = 0;
    break;

  case 3:   // show time as digits
    showTime(RTC.h, RTC.m);
    state = 0;
    break;

  default:
    state = 0;
  }
}
