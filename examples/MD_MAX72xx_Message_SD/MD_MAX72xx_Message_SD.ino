// Use the MD_MAX72XX library to scroll text on the display
//
// Demonstrates the use of the callback function to control what
// is scrolled on the display text.
//
// Text to be displayed is stored n an SD card. Each line is scrolled
// continuously on the display and run off before the next one is shown.
// At end of file the display loops back to the first line.
// Speed for the display is controlled by a pot on SPEED_IN analog in.
//
// SD library used is SDFat found at https://github.com/greiman/SdFat
// Note that there is a high chance that pin definitions will clash between the SPI for
// MD_MAX72xx and the SD card. Beware!

#include <MD_MAX72xx.h>
#include <SPI.h>
#include <SdFat.h>

#define USE_POT_CONTROL 0

#define PRINT_CALLBACK  0
#define DEBUG 0

#if DEBUG
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }
#define PRINTS(s)   Serial.print(F(s))
#else
#define PRINT(s, v)
#define PRINTS(s)
#endif

// ** MD_MAX72xx hardware definitions
// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define CLK_PIN   6  // or SCK
#define DATA_PIN  7  // or MOSI
#define CS_PIN    8  // or SS or LD

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 11

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// ** SDFat hardware definitions
// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
// MOSI - pin 11
// MISO - pin 12
// CLK - pin 13
// CS - pin 10
const char fName[] = "MESSAGE.TXT";
const uint8_t chipSelect = 10;
SdFat sd;
SdFile myFile;

// Scrolling parameters
#if USE_POT_CONTROL
#define SPEED_IN  A5
#else
#define SCROLL_DELAY  75  // in milliseconds
#endif // USE_POT_CONTROL

#define CHAR_SPACING  1 // pixels between characters

// Global data
uint16_t  scrollDelay;  // in milliseconds

int readFile(void)
// Return the next character from the file or a -1 if eof.
// End of line is marked by a '\n' returned to the caller, '\r' is skipped.
{
  int c = '\0';

  if (!myFile.isOpen())
  {
    PRINT("\nOpening ", fName);
    // open the file for read
    if (!myFile.open(fName, O_READ))
      sd.errorHalt("Cannot open file for read");
    PRINTS("- open\n");
  }

  do
    c = myFile.read();
  while (c == '\r');

  if (c == -1)	// end of file or error
  {
    PRINTS("\nRewind\n");
    myFile.rewind();
  }

  return(c);
}

void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col)
// Callback function for data that is being scrolled off the display
{
#if PRINT_CALLBACK
  Serial.print("\n cb ");
  Serial.print(dev);
  Serial.print(' ');
  Serial.print(t);
  Serial.print(' ');
  Serial.println(col);
#endif
}

uint8_t scrollDataSource(uint8_t dev, MD_MAX72XX::transformType_t t)
// Callback function for data that is required for scrolling into the display
{
  static uint8_t  state = 0;
  static uint8_t  curLen, showLen;
  static uint8_t  cBuf[8];
  int c;
  uint8_t colData = 0;

  // finite state machine to control what we do on the callback
  switch(state)
  {
    case 0: // Load the next character from the font table
      // if we reached end of message, reset the message pointer
      c = readFile();
      if ((c == -1) || (c == '\n'))	// end of file/error or end of line
      {
        state = 2;
        break;
      }

      PRINT("", (char)c);
      showLen = mx.getChar(c, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state++;
      // !! deliberately fall through to next state to start displaying

    case 1: // display the next part of the character
      colData = cBuf[curLen++];
      if (curLen == showLen)
      {
        showLen = CHAR_SPACING;
        curLen = 0;
        state = 3;
      }
      break;

    case 2: // scroll off the whole display
      PRINTS("\n-> CLEAR\n");
      showLen = mx.getColumnCount();
      curLen = 0;
      state = 3;
      break;

    case 3: // display inter-character spacing (blank column)
      colData = 0;
      curLen++;
      if (curLen == showLen)
        state = 0;
      break;

    default:
      state = 0;
  }

  return(colData);
}

 void scrollText(void)
{
  static uint32_t	prevTime = 0;

  // Is it time to scroll the text?
  if (millis()-prevTime >= scrollDelay)
  {
    mx.transform(MD_MAX72XX::TSL);	// scroll along - the callback will load all the data
    prevTime = millis();			// starting point for next time
  }
}

uint16_t getScrollDelay(void)
{
#if USE_POT_CONTROL
  uint16_t	t;

  t = analogRead(SPEED_IN);
  t = map(t, 0, 1023, 25, 250);

  return(t);
#else
  return(SCROLL_DELAY);
#endif
}

void setup(void)
{
#if DEBUG
  Serial.begin(57600);
#endif

  // Initialize MD_MAX72xx library with callbacks
  mx.begin();
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);

  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library, change to SPI_FULL_SPEED for more performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED))
    sd.initErrorHalt();

  // if we are using POT control, get that going too
#if USE_POT_CONTROL
  pinMode(SPEED_IN, INPUT);
#else
  scrollDelay = SCROLL_DELAY;
#endif
}

void loop(void)
{
  scrollDelay = getScrollDelay();
  scrollText();
}

