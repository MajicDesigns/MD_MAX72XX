// Use the MD_MAX72XX library to Print some text on the display
//
// Demonstrates the use of the library to print text.
//
// User can enter text on the serial monitor and this will display as a
// message on the display.

#include <MD_MAX72xx.h>
#include <SPI.h>

#define PRINT(s, v)     \
  {                     \
    Serial.print(F(s)); \
    Serial.print(v);    \
  }

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 4

#define CLK_PIN 2  // or SCK
#define DATA_PIN 4 // or MOSI
#define CS_PIN 3   // or SS

// SPI hardware interface
// MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Text parameters
#define CHAR_SPACING 1 // pixels between characters

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE 75
// char message[BUF_SIZE] = "V O V A";
char message[BUF_SIZE] = "VOVA";
bool newMessageAvailable = true;

void readSerial(void)
{
  static uint8_t putIndex = 0;

  while (Serial.available())
  {
    message[putIndex] = (char)Serial.read();
    if ((message[putIndex] == '\n') || (putIndex >= BUF_SIZE - 3)) // end of message character or full buffer
    {
      // put in a message separator and end the string
      message[putIndex] = '\0';
      // restart the index for next filling spree and flag we have a message waiting
      putIndex = 0;
      newMessageAvailable = true;
    }
    else
      // Just save the next char in next location
      message[putIndex++];
  }
}

uint16_t mapCol(uint16_t col)
{
  return col - col % COL_SIZE + (COL_SIZE - col % COL_SIZE - 1);
}

void printText(uint8_t modStart, uint8_t modEnd, char *pMsg, uint16_t colOffset = 0)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
  uint8_t state = 0;
  uint8_t curLen;
  uint16_t showLen;
  uint8_t cBuf[8];
  int16_t col = ((modEnd + 1) * COL_SIZE) - 1 - colOffset;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do // finite state machine to print the characters in the space available
  {
    switch (state)
    {
    case 0: // Load the next character from the font table
      // if we reached end of message, reset the message pointer
      if (*pMsg == '\0' || *pMsg == '\n' || *pMsg == 0xD)
      {
        showLen = col - (modEnd * COL_SIZE); // padding characters
        state = 2;
        break;
      }

      // retrieve the next character form the font file
      PRINT("\nNext char: ", *pMsg);
      Serial.print(", HEX: ");
      Serial.print(*pMsg, HEX);
      showLen = mx.getChar(*pMsg++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state++;
      // !! deliberately fall through to next state to start displaying

    case 1: // display the next part of the character
      mx.setColumn(mapCol(col--), cBuf[curLen++]);

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

    case 3: // display inter-character spacing or end of message padding (blank columns)
      mx.setColumn(mapCol(col--), 0);
      curLen++;
      if (curLen == showLen)
        state = 0;
      break;

    default:
      col = -1; // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void setup()
{
  mx.begin();

  Serial.begin(57600);
  Serial.print("\n[MD_MAX72XX Message Display]\nType a message for the display\nEnd message line with a newline");
}

void loop()
{
  readSerial();
  if (newMessageAvailable)
  {
    PRINT("\nProcessing new message: ", message);
    printText(0, MAX_DEVICES - 1, message);
    newMessageAvailable = false;
  }
}
