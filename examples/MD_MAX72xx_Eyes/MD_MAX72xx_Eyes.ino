// Program to exercise the MD_MAX72XX library
//
// Uses the graphics functions to animate a pair of eyes on 2 matrix modules.
// Eyes are coordinated to work together.
// Eyes are created to fill all available modules.
//
// Uses the TrueRandom library to generate random numbers for the 
// animation, available at http://code.google.com/p/tinkerit
//
#include <MD_MAX72xx.h>
#include <TrueRandom.h>
#include "MD_EyePair.h"

// Define the number of devices we have in the chain and the hardware interface
#define	MAX_DEVICES	10

// NOTE: These pin numbers will probably not work with your hardware and may 
// need to be adapted
#define	CLK_PIN		13  // or SCK
#define	DATA_PIN	11  // or MOSI
#define	CS_PIN		10  // or SS

// SPI hardware interface
MD_MAX72XX M = MD_MAX72XX(CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX eye = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Define the eyes!
#define	MAX_EYE_PAIR	(MAX_DEVICES/2)

MD_EyePair E[MAX_EYE_PAIR];

// Miscellaneous defines
#define	DELAYTIME  500  // in milliseconds

void setup()
{
  M.begin();

  // initialise the eye view
  for (uint8_t i=0; i<MAX_EYE_PAIR; i++)
    E[i].begin(i*2, &M, DELAYTIME);
}

void loop() 
{
  for (uint8_t i=0; i<MAX_EYE_PAIR; i++)
    E[i].animate();
}

