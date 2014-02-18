// Program to exercise the MD_MAX72XX library
//
// Uses the graphics functions to animates many independent
// eyes on MAX_DEVICES modules.
//
// Uses the TrueRandom library to generate random numbers for the 
// animation, available at http://code.google.com/p/tinkerit
//
#include <MD_MAX72xx.h>
#include <TrueRandom.h>
#include "MD_MAX72XX_Eyes.h"

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
// NOTE: These pin numbers will probably not work with your hardware and may 
// need to be adapted
#define	MAX_DEVICES	8

#define	CLK_PIN		13  // or SCK
#define	DATA_PIN	11  // or MOSI
#define	CS_PIN		10  // or SS

// SPI hardware interface
MD_MAX72XX eye = MD_MAX72XX(CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX eye = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Miscellaneous defines
#define	DELAYTIME  500  // in milliseconds
#define	ARRAY_SIZE(z)	(sizeof(z)/sizeof(z[0]))

// State data for each eye
typedef struct
{
	posPupil	pupilCurPos;	// the current position for the pupil
	uint32_t	timeLast;		// time of last animation
	uint16_t 	timeDelay;		// delay between animations
	bool		inBlinkCycle;	// currently blinking?

	uint32_t	lastBlinkTime;	// last time for blink animation
	uint16_t	currentDelay;	// current blink animation delay
	uint8_t		blinkState;		// current state in the blink
	uint8_t		savedEyeball[ARRAY_SIZE(eyeballData)];
	uint8_t		blinkLine;		// where the blink is at in the animation
} eyeData_t;

eyeData_t	eyeState[MAX_DEVICES];


void drawEyeball(uint8_t n)
// Draw the n-th iris on the display.
{
	eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

	eye.clear(n);	// clear out current display

	// Display the iris row data from the data array
	for (uint8_t i=0; i<ARRAY_SIZE(eyeballData); i++)
		eye.setRow(n, i, eyeballData[i]);

	eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

bool blinkEyeball(uint8_t n, bool bFirst)
// Blink the n-th iris. If this is the first call in the cycle, bFirst will be set true.
// Return true if the blink is still active, false otherwise.
{
	if (bFirst)
	{
		PRINTS("\nBlink Start");
		eyeState[n].lastBlinkTime = millis();
		eyeState[n].blinkState = 0;
		eyeState[n].blinkLine = 0;
		eyeState[n].currentDelay = 25;
	}
	else if (millis() - eyeState[n].lastBlinkTime >= eyeState[n].currentDelay)
	{
		eyeState[n].lastBlinkTime = millis();

		PRINT("\nBlink S ", eyeState[n].blinkState);
		PRINT(", BL ", eyeState[n].blinkLine);

		eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
		switch(eyeState[n].blinkState)
		{
		case 0:	// initialisation - save the current eye pattern
			for (uint8_t i=0; i<ARRAY_SIZE(eyeState[n].savedEyeball); i++)
				eyeState[n].savedEyeball[i] = eye.getRow(n, i);
			eyeState[n].blinkState = 1;
			// fall through

		case 1:	// blink the eye shut
			eye.setRow(n, eyeState[n].blinkLine, 0);
			eyeState[n].blinkLine++;
			if (eyeState[n].blinkLine == LAST_BLINK_ROW)	// this is the last row of the animation
			{
				eyeState[n].blinkState = 2;
				eyeState[n].currentDelay *= 2;
			}
			break;

		case 2:	// set up for eye opening
			eyeState[n].currentDelay /= 2;
			eyeState[n].blinkState = 3;
			// fall through
			
		case 3:
			eyeState[n].blinkLine--;
			eye.setRow(n, eyeState[n].blinkLine, eyeState[n].savedEyeball[eyeState[n].blinkLine]);

			if (eyeState[n].blinkLine == 0)	
			{
				PRINTS("\nBlink end");
				eyeState[n].blinkState = 99;
			}
			break;
		}
		eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
	}

	return(eyeState[n].blinkState != 99);
}

uint8_t findPupilData(posPupil p)
// Look through the pupil data array and return the index for 
// the entry we are requesting. If not found, return 0xff.
{
	for (uint8_t i=0; i<ARRAY_SIZE(pupilData); i++)
		if (pupilData[i].pos == p)
			return(i);

	return(0xff);
}

void drawPupil(uint8_t n, posPupil posOld, posPupil posNew)
// Draw the n-th pupil in the current position. Needs to erase the 
// old position first, then put in the new position
{
	uint8_t	p;

	eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

	// first blank out the old pupil by writing back the 
	// eyeball background 'row'
	if ((p = findPupilData(posOld)) != 0xff)
	{
		uint8_t	row = UNPACK_R(pupilData[p].rc);

		eye.setRow(n, row, eyeballData[row]); 
		eye.setRow(n, row+1, eyeballData[row+1]); 
	}

	// now show the new pupil by displaying the new background 'row' 
	// with the pupil masked out of it
	if ((p=findPupilData(posNew)) != 0xff)
	{
		uint8_t	row = UNPACK_R(pupilData[p].rc);
		uint8_t	col = UNPACK_C(pupilData[p].rc);
		uint8_t colMask = ~((1<<col)|(1<<(col-1)));

		eye.setRow(n, row, (eyeballData[row]&colMask)); 
		eye.setRow(n, row+1, (eyeballData[row+1]&colMask)); 
	}
	eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

bool posIsAdjacent(posPupil posCur, posPupil posNew)
// If the new pos is an adjacent position to the old, return true
// the arrangement is P_TL  P_TC  P_TR 
//                    P_ML  P_MC  P_MR
//                    P_BL  P_BC  P_BR 
{
	switch (posCur)
	{
	case P_TL:	return(posNew == P_TC || posNew == P_MC || posNew == P_ML);
	case P_TC:	return(posNew != P_BL && posNew != P_BC && posNew == P_BR);
	case P_TR:	return(posNew == P_TC || posNew == P_MC || posNew == P_MR);
	case P_ML:	return(posNew != P_TR && posNew != P_MR && posNew != P_BR);
	case P_MC:	return(true);	// all are adjacent
	case P_MR:	return(posNew != P_TL && posNew != P_ML && posNew != P_BL);
	case P_BL:	return(posNew == P_ML || posNew == P_MC || posNew == P_BC);
	case P_BC:	return(posNew != P_TL && posNew != P_TC && posNew == P_TR);
	case P_BR:	return(posNew == P_BC || posNew == P_MC || posNew == P_MR);
	}

	return(false);
}

void eyeAnimation(uint8_t n)
// Animate the n-th eye.
// this can either be a blink or an eye movement
{
	// do the blink if we are currently already blinking
	if (eyeState[n].inBlinkCycle) 
	{
		eyeState[n].inBlinkCycle = blinkEyeball(n, false);
		return;
	}

	// Possible animation - only animate every timeDelay ms
	if (millis() - eyeState[n].timeLast <= eyeState[n].timeDelay)
		return;

	eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

	// set up timers for next time
	eyeState[n].timeLast = millis();
	eyeState[n].timeDelay = TrueRandom.random(DELAYTIME);

	// Do the animation most of the time, so bias the 
	// random number check to achieve this
	if (TrueRandom.random(1000) <= 900)
	{
		posPupil	pupilNewPos = (posPupil)(TrueRandom.random(9));

		if (posIsAdjacent(eyeState[n].pupilCurPos, pupilNewPos))
		{
			drawPupil(n, eyeState[n].pupilCurPos, pupilNewPos);
			eyeState[n].pupilCurPos = pupilNewPos;
		}
	}
	else
		// blink the eyeball
		eyeState[n].inBlinkCycle = blinkEyeball(n, true);

	eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
};

void setup()
{
  eye.begin();
  // The MAX72XX is in power-saving mode on startup,
  // we have to do a wakeup call
  eye.control(MD_MAX72XX::SHUTDOWN, MD_MAX72XX::OFF);
  // Set the brightness to a medium values
  eye.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY/2);
  // Enable and clear the display
  eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  eye.clear();

#if  DEBUG
  Serial.begin(57600);
#endif
  PRINTS("\n[MD_MAX72XX Eye Demo]");

  // initialise the eye view
	for (uint8_t i=0; i<MAX_DEVICES; i++)
	{
	  eyeState[i].timeDelay = TrueRandom.random(DELAYTIME);;
      eyeState[i].pupilCurPos = P_MC;

	  drawEyeball(i);
      drawPupil(i, eyeState[i].pupilCurPos, eyeState[i].pupilCurPos);
	}
}

void loop() 
{
  for (uint8_t i=0; i<MAX_DEVICES; i++)
	  eyeAnimation(i);
}

