// Use the MD_MAX72XX library to Print some text on the display
//
// Demonstrates the use of the library to print text on multiple lines
// by using separate matrix displays (no zones). The DAT and CLK lines
// are shared with one LD/CS per string of matrix devices
//
// User can enter text on the serial monitor and this will display as a
// message on the display.

#include <MD_MAX72xx.h>
#include <SPI.h>

#define	PRINT(s, v)	{ Serial.print(F(s)); Serial.print(v); }

#define	BUF_SIZE	    75  // text buffer size
#define	CHAR_SPACING	1   // pixels between characters

// Define the number of devices we have in the chain and the hardware interface
#define	MAX_DEVICES	4

struct LineDefinition
{
  MD_MAX72XX  mx;                 // object definition
  char    message[BUF_SIZE];      // message for this display
  boolean newMessageAvailable;    // true if new message arrived
};

// Add new entries for more lines.
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
struct LineDefinition  Line[] =
{
  { MD_MAX72XX(11, 13, 10, MAX_DEVICES), "abc", true },
  { MD_MAX72XX(11, 13,  9, MAX_DEVICES), "def", true }
};

#define MAX_LINES   (sizeof(Line)/sizeof(LineDefinition))


void readSerial(void)
{
  static int8_t	putIndex = -1;
  static uint8_t  putLine = 0;
  char  c;

  while (Serial.available())
  {
    c = (char)Serial.read();
    if (putIndex == -1)  // first character should be the line number
    {
      if ((c >= '0') && (c < '0' + MAX_LINES))
      {
        putLine = c - '0';
        putIndex = 0;
      }
    }
    else if ((c == '\n') || (putIndex >= BUF_SIZE-3))	// end of message character or full buffer
    {
      // put in a message separator and end the string
      Line[putLine].message[putIndex] = '\0';
      // restart the index for next filling spree and flag we have a message waiting
      putIndex = -1;
      Line[putLine].newMessageAvailable = true;
    }
    else
      // Just save the next char in next location
      Line[putLine].message[putIndex++] = c;
  }
}

void printText(uint8_t lineID, uint8_t modStart, uint8_t modEnd, char *pMsg)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
  uint8_t   state = 0;
  uint8_t	  curLen;
  uint16_t  showLen;
  uint8_t	  cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  Line[lineID].mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0:	// Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        showLen = Line[lineID].mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1:	// display the next part of the character
        Line[lineID].mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3:	// display inter-character spacing or end of message padding (blank columns)
        Line[lineID].mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  Line[lineID].mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void setup()
{
  Serial.begin(57600);
  Serial.print("\n[MD_MAX72XX ");
  Serial.print(MAX_LINES);
  Serial.print(" Line Message Display]\n");
  Serial.print("\nType a message for the scrolling display\nStart message with line number\nEnd message line with a newline");

  for (uint8_t i=0; i<MAX_LINES; i++)
    Line[i].mx.begin();
}

void loop()
{
  readSerial();
  for (uint8_t i=0; i<MAX_LINES; i++)
  {
    if (Line[i].newMessageAvailable)
    {
      PRINT("\nProcessing new message: ", Line[i].message);
      printText(i, 0, MAX_DEVICES-1, Line[i].message);
      Line[i].newMessageAvailable = false;
    }
  }
}

