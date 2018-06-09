// Use the MD_MAX72XX library to display a Pacman animation
// Just for fun!

#include <MD_MAX72xx.h>
#include <SPI.h>

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
#define MAX_DEVICES 12
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);                      // SPI hardware interface
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES); // Arbitrary pins

// --------------------
// Constant parameters
//
#define ANIMATION_DELAY 75	// milliseconds
#define MAX_FRAMES      4   // number of animation frames

// ========== General Variables ===========
//
const uint8_t pacman[MAX_FRAMES][18] =  // ghost pursued by a pacman
{
  { 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe, 0x00, 0x00, 0x00, 0x3c, 0x7e, 0x7e, 0xff, 0xe7, 0xc3, 0x81, 0x00 },
  { 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe, 0x00, 0x00, 0x00, 0x3c, 0x7e, 0xff, 0xff, 0xe7, 0xe7, 0x42, 0x00 },
  { 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe, 0x00, 0x00, 0x00, 0x3c, 0x7e, 0xff, 0xff, 0xff, 0xe7, 0x66, 0x24 },
  { 0xfe, 0x7b, 0xf3, 0x7f, 0xf3, 0x7b, 0xfe, 0x00, 0x00, 0x00, 0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c },
};
const uint8_t DATA_WIDTH = (sizeof(pacman[0])/sizeof(pacman[0][0]));

uint32_t prevTimeAnim = 0;  // remember the millis() value in animations
int16_t idx;                // display index (column)
uint8_t frame;              // current animation frame
uint8_t deltaFrame;         // the animation frame offset for the next frame

// ========== Control routines ===========
//
void resetMatrix(void)
{
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY/2);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  mx.clear();
}

void setup()
{
  mx.begin();
  resetMatrix();
  prevTimeAnim = millis();
  #if DEBUG
  Serial.begin(57600);
  #endif
  PRINTS("\n[MD_MAX72XX Pacman]");
}

void loop(void)
{
  static boolean bInit = true;  // initialise the animation

  // Is it time to animate?
  if (millis()-prevTimeAnim < ANIMATION_DELAY)
    return;
  prevTimeAnim = millis();      // starting point for next time

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  // Initialize
  if (bInit)
  {
    mx.clear();
    idx = -DATA_WIDTH;
    frame = 0;
    deltaFrame = 1;
    bInit = false;

    // Lay out the dots
    for (uint8_t i=0; i<MAX_DEVICES; i++)
    {
      mx.setPoint(3, (i*COL_SIZE) + 3, true);
      mx.setPoint(4, (i*COL_SIZE) + 3, true);
      mx.setPoint(3, (i*COL_SIZE) + 4, true);
      mx.setPoint(4, (i*COL_SIZE) + 4, true);
    }
  }

  // now run the animation
  PRINT("\nINV I:", idx);
  PRINT(" frame ", frame);

  // clear old graphic
  for (uint8_t i=0; i<DATA_WIDTH; i++)
    mx.setColumn(idx-DATA_WIDTH+i, 0);
  // move reference column and draw new graphic
  idx++;
  for (uint8_t i=0; i<DATA_WIDTH; i++)
    mx.setColumn(idx-DATA_WIDTH+i, pacman[frame][i]);

  // advance the animation frame
  frame += deltaFrame;
  if (frame == 0 || frame == MAX_FRAMES-1)
    deltaFrame = -deltaFrame;

  // check if we are completed and set initialise for next time around
  bInit = (idx == mx.getColumnCount()+DATA_WIDTH);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);

  return;
}
