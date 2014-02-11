// Program to exercise the MD_MAX72XX library
//
// Uses the graphics functions to animates eyes on 2 matrix modules.
// Can be just one eye and one module if preferred!
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
#define	MAX_DEVICES	2

#define	CLK_PIN		13  // or SCK
#define	DATA_PIN	11  // or MOSI
#define	CS_PIN		10  // or SS

// SPI hardware interface
MD_MAX72XX eye = MD_MAX72XX(CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX eye = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

posPupil	pupilCurPos = P_MC;		// the current position for the pupil

// Miscellaneous defines
#define	DELAYTIME  500  // in milliseconds
#define	ARRAY_SIZE(z)	(sizeof(z)/sizeof(z[0]))

void drawEyeball()
// Draw the iris on the display(s).
{
	eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

	eye.clear();	// clear out current display

	// Display the iris row data from the data array
	for (uint8_t x=0; x<MAX_DEVICES; x++)
	{
		for (uint8_t i=0; i<ARRAY_SIZE(eyeballData); i++)
		{
			eye.setRow(x, i, eyeballData[i]);
		}
	}
	eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

bool blinkEyeball(bool bFirst)
// Blink the iris. If this is the first call in the cycle, bFirst will be set true.
// Return true if the blink is still active, false otherwise.
{
	static uint32_t	lastBlinkTime;
	static uint16_t	currentDelay;
	static uint8_t	blinkState;
	static uint8_t	savedEyeball[ARRAY_SIZE(eyeballData)];
	static uint8_t	blinkLine;

	if (bFirst)
	{
		PRINTS("\nBlink Start");
		lastBlinkTime = millis();
		blinkState = 0;
		blinkLine = 0;
		currentDelay = 25;
	}
	else if (millis() - lastBlinkTime >= currentDelay)
	{
		lastBlinkTime = millis();

		PRINT("\nBlink S ", blinkState);
		PRINT(", BL ", blinkLine);

		eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
		switch(blinkState)
		{
		case 0:	// initialisation - save the current eye pattern assuming all eyes are the same
			for (uint8_t i=0; i<ARRAY_SIZE(savedEyeball); i++)
				savedEyeball[i] = eye.getRow(0, i);
			blinkState = 1;
			// fall through

		case 1:	// blink the eye shut
			for (uint8_t x=0; x<MAX_DEVICES; x++)
				eye.setRow(x, blinkLine, 0);
			blinkLine++;
			if (blinkLine == LAST_BLINK_ROW)	// this is the last row of the animation
			{
				blinkState = 2;
				currentDelay *= 2;
			}
			break;

		case 2:	// set up for eye opening
			currentDelay /= 2;
			blinkState = 3;
			// fall through
			
		case 3:
			blinkLine--;
			for (uint8_t x=0; x<MAX_DEVICES; x++)
				eye.setRow(x, blinkLine, savedEyeball[blinkLine]);

			if (blinkLine == 0)	
			{
				PRINTS("\nBlink end");
				blinkState = 99;
			}
			break;
		}
		eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
	}

	return(blinkState != 99);
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

void drawPupil(posPupil posOld, posPupil posNew)
// Draw the pupil in the current position. Needs to erase the 
// old position first, then put in the new position
{
	uint8_t	p;

	eye.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

	// first blank out the old pupil by writing back the 
	// eyeball background 'row'
	if ((p = findPupilData(posOld)) != 0xff)
	{
		uint8_t	row = UNPACK_R(pupilData[p].rc);

		for (uint8_t x=0; x<MAX_DEVICES; x++)
		{
			eye.setRow(x, row, eyeballData[row]); 
			eye.setRow(x, row+1, eyeballData[row+1]); 
		}
	}

	// now show the new pupil by displaying the new background 'row' 
	// with the pupil masked out of it
	if ((p=findPupilData(posNew)) != 0xff)
	{
		uint8_t	row = UNPACK_R(pupilData[p].rc);
		uint8_t	col = UNPACK_C(pupilData[p].rc);
		uint8_t colMask = ~((1<<col)|(1<<(col-1)));

		for (uint8_t x=0; x<MAX_DEVICES; x++)
		{
			eye.setRow(x, row, (eyeballData[row]&colMask)); 
			eye.setRow(x, row+1, (eyeballData[row+1]&colMask)); 
		}
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

void eyeAnimation(void)
// Animate the eye(s).
// this cane either be a blink or an eye movement
{
	static uint32_t	timeLast = 0;
	static uint16_t timeDelay = DELAYTIME;
	static bool		inBlinkCycle = false;

	// do the blink if we are currently already blinking
	if (inBlinkCycle) 
	{
		inBlinkCycle = blinkEyeball(false);
		return;
	}

	// Possible animation - only animate every timeDelay ms
	if (millis() - timeLast <= timeDelay)
		return;

	// set up timers for next time
	timeLast = millis();
	timeDelay = TrueRandom.random(DELAYTIME);

	// Do the animation most of the time, so bias the 
	// random number check to achieve this
	if (TrueRandom.random(1000) <= 900)
	{
		posPupil	pupilNewPos = (posPupil)(TrueRandom.random(9));

		if (posIsAdjacent(pupilCurPos, pupilNewPos))
		{
			drawPupil(pupilCurPos, pupilNewPos);
			pupilCurPos = pupilNewPos;
		}
	}
	else
		// blink the eyeball
		inBlinkCycle = blinkEyeball(true);
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
  drawEyeball();
  drawPupil(pupilCurPos, pupilCurPos);
}

void loop() 
{
	eyeAnimation();
}

