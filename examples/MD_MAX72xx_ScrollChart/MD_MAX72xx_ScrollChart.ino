// Use the MD_MAX72XX library to Display a Scrolling Chart
//
// Scroll Chart Style can be changed from line to bar chart, triggered
// by a switch on the MODE_SWITCH pin.
//
// Uses the MD_UIswitch library found at https://github.com/MajicDesigns/MD_UISwitch

#include <MD_MAX72xx.h>
#include <SPI.h>
#include <MD_UISwitch.h>

#define DEBUG 0   // Enable or disable (default) debugging output

#if DEBUG
#define PRINT(s, v)   { Serial.print(F(s)); Serial.print(v); }      // Print a string followed by a value (decimal)
#define PRINTX(s, v)  { Serial.print(F(s)); Serial.print(v, HEX); } // Print a string followed by a value (hex)
#define PRINTB(s, v)  { Serial.print(F(s)); Serial.print(v, BIN); } // Print a string followed by a value (binary)
#define PRINTC(s, v)  { Serial.print(F(s)); Serial.print((char)v); }  // Print a string followed by a value (char)
#define PRINTS(s)     { Serial.print(F(s)); }                       // Print a string
#else
#define PRINT(s, v)   // Print a string followed by a value (decimal)
#define PRINTX(s, v)  // Print a string followed by a value (hex)
#define PRINTB(s, v)  // Print a string followed by a value (binary)
#define PRINTC(s, v)  // Print a string followed by a value (char)
#define PRINTS(s)     // Print a string
#endif

// --------------------
// MD_MAX72xx hardware definitions and object
// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
//
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 8
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);                      // SPI hardware interface
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES); // Arbitrary pins

// --------------------
// Mode keyswitch parameters and object
//
#define MODE_SWITCH 9 // Digital Pin

MD_UISwitch_Digital  ks = MD_UISwitch_Digital(MODE_SWITCH, LOW);

// --------------------
// Constant parameters
//
// Various delays in milliseconds
#define Next_POINT_DELAY  40


// ========== General Variables ===========
//
uint32_t prevTime = 0;    // Used for remembering the mills() value

// ========== Graphic routines ===========
//
bool graphDisplay(bool bInit, uint8_t nType)
{
  static int8_t  curPoint = 0;
  uint8_t curCol = 0;

  // are we initializing?
  if (bInit)
  {
    resetDisplay();
    curPoint = 4;
    bInit = false;
  }
  else if (millis() - prevTime >= Next_POINT_DELAY)
  {
    prevTime = millis();    // rest for next time

    // work out the new value for the height depending on the chart type
    switch (nType)
    {
      case 0:   // continuous display next point should be +/-1 or 0
        curPoint += random(3) - 1;
        if (curPoint < 0) curPoint = 0;
        if (curPoint > 7) curPoint = 7;
        break;

      case 1:  // random height
      case 2:
        curPoint = random(8);
        break;
    }

    // now work out the new column value
    switch (nType)
    {
      case 0:   // just a dot
      case 1:
        curCol = (1 << curPoint);
        break;

      case 2:   // bar chart
        for (uint8_t i=0; i<8; i++)
          curCol |= (i<curPoint ? 0 : 1) << i;
        break;
    }

    // Shift over and insert the new column
    mx.transform(MD_MAX72XX::TSL);
    mx.setColumn(0, curCol);
  }

  return(bInit);
}

// ========== Control routines ===========
//
void resetDisplay(void)
{
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY/2);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  mx.clear();
  prevTime = 0;
}

void runAnimation(void)
// Schedule the animations, switching to the next one when the
// the mode switch is pressed.
{
  static  uint8_t state = 0;
  static  bool    bRestart = true;

  // check if the switch is pressed and handle that first
  if (ks.read() == MD_UISwitch::KEY_PRESS)
  {
    state = (state + 1) % 3;
    bRestart = true;
  };

  // now do whatever we do in the current state
  bRestart = graphDisplay(bRestart, state);
}

void setup()
{
  mx.begin();
  ks.begin();

#if DEBUG
  Serial.begin(57600);
#endif
  PRINTS("\n[MD_MAX72XX Scroll Chart]");
}

void loop()
{
  runAnimation();
}

