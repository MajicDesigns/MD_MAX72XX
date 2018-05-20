#include "MD_RobotEyes.h"
#include "MD_RobotEyes_Data.h"

// Debugging macros
#define DEBUG 0

#if DEBUG
#define PRINTS(s)    { Serial.print(F(s)); }
#define PRINT(s, v)  { Serial.print(F(s)); Serial.print(v); }
#define PRINTX(s, v) { Serial.print(F(s)); Serial.print(F("0x")); Serial.print(v, HEX); }
#else
#define PRINTS(s)
#define PRINT(s, v)
#define PRINTX(s, v)
#endif

MD_RobotEyes::MD_RobotEyes(void) :
_nextEmotion(E_NEUTRAL), _animState(S_IDLE),
_autoBlink(true), _timeBlinkMinimum(5000)
{
};

void MD_RobotEyes::loadEye(uint8_t module, uint8_t ch)
{
  uint8_t buf[EYE_COL_SIZE];
  uint8_t size;

  size = _M->getChar(ch, EYE_COL_SIZE, buf);

  for (uint8_t i = 0; i < EYE_COL_SIZE; i++)
  {
    _M->setColumn(module, i, buf[i]);
  }
}

void MD_RobotEyes::drawEyes(uint8_t L, uint8_t R)
// Draw the left and right eyes
{
  MD_MAX72XX::fontType_t *savedFont = _M->getFont();

  _M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  _M->setFont(_RobotEyes_Font);

  _M->clear(_sd, _sd+1);  // clear out display modules

  // Load the data and show it
  loadEye(_sd+LEFT_MODULE_OFFSET, L);
  loadEye(_sd+RIGHT_MODULE_OFFSET, R);

  _M->setFont(savedFont);
  _M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

#if DEBUG
void MD_RobotEyes::dumpSequence(const animFrame_t* pBuf, uint8_t numElements)
// Debugging routine to display an animation table in PROGMEM
{
  for (uint8_t i = 0; i < numElements; i++)
  {
    animFrame_t f;

    memcpy_P(&f, &pBuf[i], sizeof(animFrame_t));
    PRINT("\n[", i);
    PRINT("]: L:", f.eyeData[LEFT_EYE_INDEX]);
    PRINT(" R:", f.eyeData[RIGHT_EYE_INDEX]);
    PRINT(" T:", f.timeFrame);
  }
}
#endif

uint8_t MD_RobotEyes::loadSequence(emotion_t e)
// Load the next emotion from the static data. 
// Set global variables to the required values
{
  // run through the lookup table to find the sequence data
  for (uint8_t i = 0; i < ARRAY_SIZE(lookupTable); i++)
  {
    memcpy_P(&_animEntry, &lookupTable[i], sizeof(animTable_t));
    if (_animEntry.e == e)
    {
#if DEBUG
      dumpSequence(_animEntry.seq, _animEntry.size);
#endif
      break;
    }
  }

  // set up the current index depending on direction of animation
  if (_animReverse) _animIndex = _animEntry.size - 1; else _animIndex = 0;

  return(_animEntry.size);
}

void MD_RobotEyes::loadFrame(animFrame_t* pBuf)
// Load the idx'th frame from the frame sequence PROGMEM to normal memory pBuf
{
  memcpy_P(pBuf, &_animEntry.seq[_animIndex], sizeof(animFrame_t));
}

void MD_RobotEyes::showText(bool bInit)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
  static enum { S_LOAD, S_SHOW, S_SPACE } state;
  static uint8_t	curLen, showLen;
  static uint8_t	cBuf[EYE_COL_SIZE];

  if (bInit)
  {
    PRINT("\nText: ", _pText);
    _timeLastAnimation = millis();
    _M->clear(_sd, _sd + 1);
    state = S_LOAD;
  }

  // Is it time to scroll the text?
  if (millis() - _timeLastAnimation < FRAME_TIME/2)
    return;

  _M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  // Now scroll the text
  _M->transform(_sd, _sd+1, MD_MAX72XX::TSL);	// scroll along by one place
  _timeLastAnimation = millis();	  // starting time for next scroll

  // Now work out what's next using finite state machine to control what we do
  switch (state)
  {
  case S_LOAD:	// Load the next character from the font table
    // if we reached end of message or empty string, reset the message pointer
    if (*_pText == '\0')
    {
      _pText = nullptr;
      break;
    }

    // otherwise load the character
    showLen = _M->getChar(*_pText++, ARRAY_SIZE(cBuf), cBuf);
    curLen = 0;
    state = S_SHOW;
    // fall through to the next state

  case S_SHOW:	// display the next part of the character
    _M->setColumn(_sd, 0, cBuf[curLen++]);
    if (curLen == showLen)
    {
      showLen = (*_pText == '\0' ? 2*EYE_COL_SIZE : 1); // either 1 space or pad to the end of the display if finished
      curLen = 0;
      state = S_SPACE;
    }
    break;

  case S_SPACE:	// display inter-character spacing (blank columns)
    _M->setColumn(_sd, 0, 0);
    curLen++;
    if (curLen >= showLen)
      state = S_LOAD;
    break;

  default:
    state = S_LOAD;
  }

  _M->control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void MD_RobotEyes::begin(MD_MAX72XX *M, uint8_t moduleStart)
// initialize other stuff after libraries have started
{
#if DEBUG
  Serial.begin(57600);
#endif
  PRINTS("\n[MD_RobotEyes Debug]");

  _M = M;
  _sd = moduleStart;

  setAnimation(E_NEUTRAL, false);
};

bool MD_RobotEyes::runAnimation(void)
// Animate the eyes
// Return true if there is no animation happening
{
  static animFrame_t thisFrame;

  switch (_animState)
  {
  case S_IDLE:    // no animation running - wait for a new one or blink if time to do so
    if (_pText != nullptr)    // there is some text to show
    {
      PRINTS("\nIDLE: showing text");
      showText(true);
      _animState = S_TEXT;
      break;
    }
    // otherwise fall through and try for an animation

  case S_RESTART:   // back to start of current animation
    if (_nextEmotion != E_NONE) // check if we have an animation in the queue
    {
      PRINTS("\nRESRT: showing animation");
      _timeLastAnimation = millis();

      // set up the next animation
      loadSequence(_nextEmotion);
      _nextEmotion = E_NONE;
      _animState = S_ANIMATE;
    }
    else if (_autoBlink) // check if we should be blinking
    {
      if (((millis() - _timeLastAnimation) >= _timeBlinkMinimum) && (random(1000) > 700))
      {
        PRINTS("\nRESRT: forcing blink");
        setAnimation(E_BLINK, true);
        _animState = S_RESTART;
      }
    }
    break;

  case S_ANIMATE:  // process the next frame for this sequence
    PRINT("\nPROCESS: Frame:", _animIndex);
    loadFrame(&thisFrame);
    drawEyes(thisFrame.eyeData[LEFT_EYE_INDEX], thisFrame.eyeData[RIGHT_EYE_INDEX]);
    if (_animReverse) _animIndex--; else _animIndex++;

    _timeStartPause = millis();
    _animState = S_PAUSE;
    break;

  case S_PAUSE: // pause this frame for the required time
    {
      if ((millis() - _timeStartPause) < thisFrame.timeFrame)
        break;

      // check if this is the end of animation
      if ((!_animReverse && _animIndex >= _animEntry.size) ||
           (_animReverse && _animIndex < 0))
      {
        PRINTS("\nPAUSE: Animation end")
        if (_autoReverse) // set up the same emotion but in reverse
        {
          PRINTS(" & auto reverse");
          _nextEmotion = _animEntry.e;
          _animReverse = true;  // set this flag for the restart state
          _autoReverse = false; // clear the flag for this animation sequence
          _animState = S_RESTART;
        }
        else
          _animState = S_IDLE;
      }
      else
        _animState = S_ANIMATE;
    }
    break;

  case S_TEXT:  // currently displaying text
    {
      showText();
      if (_pText == nullptr)
        _animState = S_IDLE;
    }
    break;

  default:  // something is wrong - reset the FSM
      _animState = S_IDLE;
      break;
  }

  return(_animState == S_IDLE);
};


