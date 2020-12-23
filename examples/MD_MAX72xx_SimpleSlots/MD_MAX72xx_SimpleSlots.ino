// Use the MD_MAX72XX library to implements a slot machine 
// type display with scrolling symbols and simple sound effects.
// When numbers change they are scrolled up or down as if on a cylinder.
//
// Switch is on the SW_PIN digital input, buzzer driver by the toen() on 
// BUZZ_PIN digital output.
//

#include <MD_MAX72xx.h>
#include <MD_UISwitch.h>
#include <SPI.h>
#include "FontSymbols.h"

#define ALWAYS_WIN 0 // for testing
#define DEBUG 0

#if DEBUG
#define PRINT(s, v)  do { Serial.print(F(s)); Serial.print(v); } while (false);
#define PRINTX(s, v) do { Serial.print(F(s)); Serial.print(v, HEX); } while (false);
#define PRINTS(s)    do { Serial.print(F(s)); } while (false);
#else
#define PRINT(s, v)
#define PRINTS(s)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
const uint8_t MAX_DEVICES = 4;

const uint8_t CLK_PIN = 13;  // or SCK
const uint8_t DATA_PIN = 11; // or MOSI
const uint8_t CS_PIN = 10;   // or SS

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Tone buzzer definition
const uint8_t  BUZZ_PIN = 3;    // pin
const uint16_t BUZZ_FREQ = 180; // frequency
const uint8_t  BUZZ_TIME = 3;   // duration

// Switch definition
const uint8_t SW_PIN = 8;

MD_UISwitch_Digital S(SW_PIN);

// Tumbler animation definitions
const uint32_t TIME_BASE = 5000;      // base animation time in milliseconds
const uint32_t TIME_INCREMENT = 300;  // units of time increments for random duration
const uint32_t TIME_FRAME_DELAY = 50; // in milliseconds

// Display parameters
const uint8_t SPACING = 1;        // pixels between characters
const uint8_t SYMBOL_COLS = 7;    // should accomodate the fixed width font columns
const uint8_t MAX_SYMBOLS = 3;    // number of slot machine symbols
const uint8_t TOTAL_SYMBOLS = 20; // total number of symbols in the font file (exclude 0)

// Structure to hold the data for each symbol to be displayed and animated
// this could be expanded to include other character specific data (eg, column
// where it starts if display is spaced irregularly).
typedef struct
{
  enum:uint8_t { ST_INIT, ST_WAITFRAME, ST_ANIM, ST_END } state;
  uint8_t oldValue, newValue;   // code for the value for the symbol
  uint8_t index;                // animation progression index
  bool    scrollUp;             // scroll up animation flag
  uint32_t timeDuration;        // total animaton duration for this symbol
  uint32_t timeStart;           // time the animation started
  uint32_t timeLastFrame;       // time the last frame started animating
  uint8_t cols;                 // number of valid cols in the charMap
  uint8_t charMap[SYMBOL_COLS]; // blended character font bitmap
}  symbolData_t;

symbolData_t symbol[MAX_SYMBOLS];

void updateDisplay(bool bInvert = false)
// do the necessary to display current bitmap buffer to the LED display
{
  uint8_t   curCol = 0;

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();

  for (int8_t i = MAX_SYMBOLS - 1; i >= 0; i--)
  {
    for (int8_t j = symbol[i].cols - 1; j >= 0; j--)
    {
      mx.setColumn(curCol++, bInvert ? ~symbol[i].charMap[j] : symbol[i].charMap[j]);
    }
    curCol += SPACING;
  }

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

bool animateSingle(uint8_t idx)
// Work out the character map for the symbol specified
// Return true if there has been a change.
{
  bool b = false;

  // finite state machine to control what we do
  switch(symbol[idx].state)
  {
    case symbolData_t::ST_INIT: // Initialize the display - done once only on first call
      PRINTS("\nST_INIT");

      // Display the starting number
      symbol[idx].cols = mx.getChar(symbol[idx].oldValue, SYMBOL_COLS, symbol[idx].charMap);
      symbol[idx].timeStart = symbol[idx].timeLastFrame = millis();
      symbol[idx].newValue = random(TOTAL_SYMBOLS) + 1;
      symbol[idx].scrollUp = (random(100) >= 50);
      symbol[idx].index = 0;
      b = true;

      // Now we wait for a animation timer to expire
      symbol[idx].state = symbolData_t::ST_WAITFRAME;
      break;

    case symbolData_t::ST_WAITFRAME: // not animating waiting for the frame timer
      if (millis() - symbol[idx].timeLastFrame < TIME_FRAME_DELAY)
        break;

      // a change has been found - we will be animating something
      symbol[idx].state = symbolData_t::ST_ANIM;

      // initialize animation parameters for this symbol
      symbol[idx].timeLastFrame = millis();   // for the next time around
      break;

    case symbolData_t::ST_ANIM: // currently animating a change
      // work out the new intermediate bitmap for each character
      // 1. Get the 'new' character bitmap into temp buffer
      // 2. Shift this buffer down or up by current index amount
      // 3. Shift the current character by one pixel up or down
      // 4. Combine the new partial character and the existing character to produce a frame
      {
        uint8_t newChar[SYMBOL_COLS] = { 0 };
/*
        PRINT("\nST_ANIM Symbol ", idx);
        PRINT(" from '", symbol[idx].oldValue);
        PRINT("' to '", symbol[idx].newValue);
        PRINT("' index ", symbol[idx].index);
*/
        b = true;   // modify return indicator

        mx.getChar(symbol[idx].newValue, SYMBOL_COLS, newChar);

        // make a sound
        tone(BUZZ_PIN, BUZZ_FREQ, BUZZ_TIME);

        // now work out the scroll character map
        if (symbol[idx].scrollUp)
        {
          // scroll up
          for (uint8_t j = 0; j < symbol[idx].cols; j++)
          {
            newChar[j] = newChar[j] << (COL_SIZE - 1 - symbol[idx].index);
            symbol[idx].charMap[j] = symbol[idx].charMap[j] >> 1;
            symbol[idx].charMap[j] |= newChar[j];
          }
        }
        else
        {
          // scroll down
          for (uint8_t j = 0; j < symbol[idx].cols; j++)
          {
            newChar[j] = newChar[j] >> (COL_SIZE - 1 - symbol[idx].index);
            symbol[idx].charMap[j] = symbol[idx].charMap[j] << 1;
            symbol[idx].charMap[j] |= newChar[j];
          }
        }

        // Set new parameters for next animation and check if we are done.
        // We are done when the time for the animation exceeds the total 
        // duration and the character is full in place.
        symbol[idx].index++;
        symbol[idx].state = symbolData_t::ST_WAITFRAME;

        if (symbol[idx].index >= COL_SIZE)  // reached the end of this symbol
        {
          symbol[idx].oldValue = symbol[idx].newValue;  // done animating this transition
          symbol[idx].index = 0;

          if (millis() - symbol[idx].timeStart >= symbol[idx].timeDuration) // check duration timer
          {
            PRINT("\nSymbol ", idx);
            PRINTS(" ENDED");
            symbol[idx].state = symbolData_t::ST_END;
          }
          else  // still animating, get the next symbol to show
          {
            do
            {
              symbol[idx].newValue = random(TOTAL_SYMBOLS) + 1;
            } while (symbol[idx].newValue == symbol[idx].oldValue); // make sure we have something different

            PRINT("\nSymbol ", idx);
            PRINT(" new value ", symbol[idx].newValue);
          }
        }
      }
    break;

    case symbolData_t::ST_END:    // do nothing. This will be reset externally once all have completed.
      break;

    default:
      symbol[idx].state = symbolData_t::ST_END;
  }

  return(b);  // true if changes occurred
}

bool animateAll(void)
// Animate the symbols, one at a time.
// return true if all the animations are completed
{
  bool b = false;

  // run the animations and update the display if there is a change
  for (uint8_t i = 0; i < MAX_SYMBOLS; i++)
    b |= animateSingle(i);

  if (b) updateDisplay();

  // Now check if we are done with all animations
  b = true;
  for (uint8_t i = 0; i < MAX_SYMBOLS; i++)
    b &= symbol[i].oldValue == symbol[i].newValue;

  return(b);
}

bool checkWinner(void)
// currently only 3 of a kind wins
{
  bool b = true;

#if ALWAYS_WIN
  b = true;
#else
  for (uint8_t i = 1; i < MAX_SYMBOLS; i++)
    b &= symbol[i - 1].newValue == symbol[i].newValue;
#endif

  if (b) PRINTS("\nWINNER!");

  return(b);
}

void setup()
{
#if DEBUG
  Serial.begin(57600);
#endif // DEBUG
  PRINTS("\n[MD_MAX72xx SimpleSlots]")

  // Matrix initialization
  mx.begin();
  mx.setFont(slotSymbols);

  // Switch initialization
  S.begin();
  S.enableDoublePress(false);
  S.enableLongPress(false);
  S.enableRepeat(false);

  // Tone Initialization
  pinMode(BUZZ_PIN, OUTPUT);
}

void loop()
{
  static enum:uint8_t { S_IDLE, S_INIT, S_RUN, S_CHECK, S_WINNER, S_END } state = S_INIT;

  switch (state)
  {
  case S_IDLE:  // wait for the switch to be released
    if (S.read() == MD_UISwitch::KEY_UP)
    {
      PRINTS("\n-> Key release!");
      state = S_INIT;
    }
    break;

  case S_INIT:  // initialise the timing parameters
    randomSeed(millis());   // should be good as user timing is unpredictable

    // now set up timing parameters for each symbol
    for (uint8_t i = 0; i < MAX_SYMBOLS; i++)
    {
      symbol[i].state = symbolData_t::ST_INIT;
      symbol[i].timeDuration = TIME_BASE + (random(10) * TIME_INCREMENT);
      PRINT("\nsym[", i);
      PRINT("] duration ", symbol[i].timeDuration);
    }
    state = S_RUN;
    break;

  case S_RUN:   // run the animation
    if (animateAll())
    {
      PRINTS("\n-> Ended with");
      for (uint8_t i = 0; i < MAX_SYMBOLS; i++)
      {
        PRINT(" [", i);
        PRINT("]=", symbol[i].newValue)
      }
      state = S_CHECK;
    }
    break;

  case S_CHECK:  // work out if there is a winner
    if (checkWinner())
      state = S_WINNER;
    else
      state = S_END;
    break;

  case S_WINNER:
    PRINTS("\nWINNER!");
    for (uint8_t j = 0; j < 3; j++)
    {
      for (uint8_t i = 0; i < MAX_SYMBOLS; i++)
      {
        uint8_t temp[SYMBOL_COLS] = { 0 };

        memcpy(temp, symbol[i].charMap, sizeof(symbol[i].charMap));
        memset(symbol[i].charMap, 0, sizeof(symbol[i].charMap));
        updateDisplay();    // do all the even counts
        memcpy(symbol[i].charMap, temp, sizeof(symbol[i].charMap));
        delay(300);
      }
    }
    updateDisplay();  // make sure it is clean
    state = S_END;
    break;

  case S_END:
    state = S_IDLE;
    break;

  default:
    state = S_IDLE;
  }
}

