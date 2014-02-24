#include "MD_EyePair.h"

// Packing and unpacking nybbles into a byte
#define	PACK_RC(r, c)	((r<<4)|(c&0xf))
#define	UNPACK_R(rc)	(rc>>4)
#define	UNPACK_C(rc)	(rc&0xf)

#define	SMALL_EYEBALL	0

// Class static variables

#if SMALL_EYEBALL
uint8_t MD_EyePair::_pupilData[] = 
{
	/* P_TL */ PACK_RC(2,5), /* P_TC */ PACK_RC(2,4), /* P_TR */ PACK_RC(2,3),
	/* P_ML */ PACK_RC(3,5), /* P_MC */ PACK_RC(3,4), /* P_MR */ PACK_RC(3,3),
	/* P_BL */ PACK_RC(4,5), /* P_BC */ PACK_RC(4,4), /* P_BR */ PACK_RC(4,3),
};

// Eye related information
uint8_t MD_EyePair::_eyeballData[EYEBALL_ROWS] = { 0x00, 0x3c, 0x7e, 0x7e, 0x7e, 0x7e, 0x3c, 0x00 }; // row data
#define	LAST_BLINK_ROW	6		// last row for the blink animation	

#else

uint8_t MD_EyePair::_pupilData[] = 
{
	/* P_TL */ PACK_RC(3,5), /* P_TC */ PACK_RC(3,4), /* P_TR */ PACK_RC(3,3),
	/* P_ML */ PACK_RC(4,5), /* P_MC */ PACK_RC(4,4), /* P_MR */ PACK_RC(4,3),
	/* P_BL */ PACK_RC(5,5), /* P_BC */ PACK_RC(5,4), /* P_BR */ PACK_RC(5,3),
};
uint8_t MD_EyePair::_eyeballData[EYEBALL_ROWS] = { 0x3c, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x3c };	// row data
#define	LAST_BLINK_ROW	7		// last row for the blink animation	

#endif

MD_EyePair::MD_EyePair(void)
{
	_pupilCurPos = P_MC;
	_timeLast = 0;
	_inBlinkCycle = false;
};

void MD_EyePair::drawEyeball()
// Draw the iris on the display(s)
{
	_M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

	_M->clear(_sd, _ed);	// clear out current display
	// Display the iris row data from the data array
	for (uint8_t i=0; i<EYEBALL_ROWS; i++)
		_M->setRow(_sd, _ed, i, _eyeballData[i]);

	_M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

bool MD_EyePair::blinkEyeball(bool bFirst)
// Blink the iris. If this is the first call in the cycle, bFirst will be set true.
// Return true if the blink is still active, false otherwise.
{
	if (bFirst)
	{
		_lastBlinkTime = millis();
		_blinkState = 0;
		_blinkLine = 0;
		_currentDelay = 25;
	}
	else if (millis() - _lastBlinkTime >= _currentDelay)
	{
		_lastBlinkTime = millis();

		_M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
		switch(_blinkState)
		{
		case 0:	// initialisation - save the current eye pattern assuming both eyes are the same
			for (uint8_t i=0; i<EYEBALL_ROWS; i++)
				_savedEyeball[i] = _M->getRow(_sd, i);
			_blinkState = 1;
			// fall through

		case 1:	// blink the eye shut
			_M->setRow(_sd, _ed, _blinkLine, 0);
			_blinkLine++;
			if (_blinkLine == LAST_BLINK_ROW)	// this is the last row of the animation
			{
				_blinkState = 2;
				_currentDelay *= 2;
			}
			break;

		case 2:	// set up for eye opening
			_currentDelay /= 2;
			_blinkState = 3;
			// fall through
			
		case 3:
			_blinkLine--;
			_M->setRow(_sd, _ed, _blinkLine, _savedEyeball[_blinkLine]);

			if (_blinkLine == 0)	
			{
				_blinkState = 99;
			}
			break;
		}
		_M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
	}

	return(_blinkState != 99);
}

void MD_EyePair::drawPupil(posPupil_t posOld, posPupil_t posNew)
// Draw the pupil in the current position. Needs to erase the 
// old position first, then put in the new position
{
	_M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

	// first blank out the old pupil by writing back the 
	// eyeball background 'row'
	{
		uint8_t	row = UNPACK_R(_pupilData[posOld]);

		_M->setRow(_sd, _ed, row, _eyeballData[row]); 
		_M->setRow(_sd, _ed, row+1, _eyeballData[row+1]); 
	}

	// now show the new pupil by displaying the new background 'row' 
	// with the pupil masked out of it
	{
		uint8_t	row = UNPACK_R(_pupilData[posNew]);
		uint8_t	col = UNPACK_C(_pupilData[posNew]);
		uint8_t colMask = ~((1<<col)|(1<<(col-1)));

		_M->setRow(_sd, _ed, row, (_eyeballData[row]&colMask)); 
		_M->setRow(_sd, _ed, row+1, (_eyeballData[row+1]&colMask)); 
	}
	_M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

bool MD_EyePair::posIsAdjacent(posPupil_t posCur, posPupil_t posNew)
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
	case P_MC:	return(true);	// all are adjacent to center
	case P_MR:	return(posNew != P_TL && posNew != P_ML && posNew != P_BL);
	case P_BL:	return(posNew == P_ML || posNew == P_MC || posNew == P_BC);
	case P_BC:	return(posNew != P_TL && posNew != P_TC && posNew == P_TR);
	case P_BR:	return(posNew == P_BC || posNew == P_MC || posNew == P_MR);
	}

	return(false);
}

void MD_EyePair::begin(uint8_t startDev, MD_MAX72XX *M, uint16_t maxDelay)
// initialise the eyes
{
	_sd = startDev;
	_ed = startDev + 1;
	_M = M;
	_timeDelay = _maxDelay = maxDelay;

	drawEyeball();
	drawPupil(_pupilCurPos, _pupilCurPos);
};

void MD_EyePair::animate(void)
// Animate the eye(s).
// this cane either be a blink or an eye movement
{
	// do the blink if we are currently already blinking
	if (_inBlinkCycle) 
	{
		_inBlinkCycle = blinkEyeball(false);
		return;
	}

	// Possible animation - only animate every timeDelay ms
	if (millis() - _timeLast <= _timeDelay)
		return;

	// set up timers for next time
	_timeLast = millis();
	_timeDelay = TrueRandom.random(_maxDelay);

	// Do the animation most of the time, so bias the 
	// random number check to achieve this
	if (TrueRandom.random(1000) <= 900)
	{
		posPupil_t	pupilNewPos = (posPupil_t)(TrueRandom.random(9));

		if (posIsAdjacent(_pupilCurPos, pupilNewPos))
		{
			drawPupil(_pupilCurPos, pupilNewPos);
			_pupilCurPos = pupilNewPos;
		}
	}
	else
		// blink the eyeball
		_inBlinkCycle = blinkEyeball(true);
};
