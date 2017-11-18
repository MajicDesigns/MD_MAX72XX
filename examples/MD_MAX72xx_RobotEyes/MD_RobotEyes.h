// Implements a class to draw and animate a pair of 'emotive' eyes for a robot
//
#pragma once

#include <MD_MAX72xx.h>

// Misc defines
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))  ///< number of elements in an array
#define EYE_COL_SIZE 8                          ///< number of columns in one eye

// Module offsets from first module specified
#define LEFT_MODULE_OFFSET   1    ///< offset from the base LED module for the left eye
#define RIGHT_MODULE_OFFSET  0    ///< offset from the base LED module for the right eye

// Array references for eyeData array below
#define LEFT_EYE_INDEX   1        ///< array reference in the eye data for the left eye
#define RIGHT_EYE_INDEX  0        ///< array reference in the eye data for the right eye

// Basic unit of time a frame is displayed
#define FRAME_TIME 100            ///< minimum animation time

/**
* Robot Eyes Class.
* This class manages the displayed of animated eyes using LED matrices using the functions
* provided by the MD_MAX72xx library.
*/
class MD_RobotEyes
{
public:
  /**
  * Emotions enumerated type.
  *
  * This enumerated type defines the emotion animations
  * availabel in the class for the eyes display
  */
  // 
  typedef enum
  {
    E_NONE,     ///< placeholder for no emotions, not user selectable
    E_NEUTRAL,  ///< eyes in neutral position (no animation)
    E_BLINK,    ///< both eyes blink
    E_WINK,     ///< one eye blink
    E_LOOK_L,   ///< both eyes look left
    E_LOOK_R,   ///< both eyes look right
    E_LOOK_U,   ///< both eyes look up
    E_LOOK_D,   ///< both eyes look down
    E_ANGRY,    ///< eyes look angry (symmetrical)
    E_SAD,      ///< eyes look sad (symmetrical)
    E_EVIL,     ///< eyes look evil (symmetrical)
    E_EVIL2,    ///< eyes look evil (asymmetrical)
    E_SQUINT,   ///< both eye squint
    E_DEAD,     ///< eyes indicate dead (different)
    E_SCAN_UD,  ///< both eyes scanning Up/Down
    E_SCAN_LR,  ///< both eyes scanning Left/Right
  } emotion_t;

  /**
  * Class Constructor.
  *
  * Instantiate a new instance of the class.
  */
  MD_RobotEyes(void);

  /**
  * Class Destructor.
  *
  * Released any allocated memory and does the necessary to clean 
  * up once the object is no longer required.
  */
  ~MD_RobotEyes(void) { };

  /**
  * Initialize the object.
  *
  * Initialise the object data. This needs to be called during setup() to initialise new
  * data for the class that cannot be done during the object creation.
  *
  * Outside of the class, the MD_MAX72xx library should be initialised and the pointer
  * to the MD_MAX72xx object passed to the parameter. Also, as the eyes could be in the 
  * middle of a string of LED modules, the first 'eye' module can be specified.
  *
  * /param M            pointer to the MD_MAX72xx library object.
  * /param moduleStart  the first 'eye' LED module. Defaults to 0 if not specified.
  */
  void begin(MD_MAX72XX *M, uint8_t moduleStart = 0);

  /**
  * Set the animation type and parameters.
  *
  * Set the next animations to the specified. Additionally, set whether the animation should
  * auto reverse the action (eg, blink down then back up again) and whether the animation 
  * should be run in reverse.
  * 
  * Animations are generally symmetric, so only half the anmation needs to be specified.
  * If an animated expression needs to be held, the animation should be run without auto 
  * reverse, which holds the animation at the end point, and then later run the animation 
  * in reverse from the last position to return to the idle state.
  *
  * \param e  the type of emotion to be displayed, one of the emotion_T enumerated values.
  * \param r  if true, run auto reverse.
  * \param b  if true, start the animation from the end of the sequence.
  */
  inline void setAnimation(emotion_t e, bool r, bool b = false) { _nextEmotion = e; _autoReverse = r; _animReverse = b; };

  /**
  * Set the blink time.
  *
  * When no animation is running and AutoBlink is set, the eyes will occasionally blink.
  * Set the minimum time period between blinks. A blink will occur a random time after this.
  *
  * \param t  the minimum time between blinkins actions in milliseconds.
  */
  inline void setBlinkTime(uint16_t t) { _timeBlinkMinimum = t; };
  
  /**
  * Set or reset auto blink mode.
  *
  * When no animation is running and AutoBlink is set, the eyes will loccasionally blink.
  *
  * \param b  set auto blink if true, reset auto blink if false.
  */
  inline void setAutoBlink(bool b) { _autoBlink = b; };

  /**
  * Display a text message.
  *
  * At the end of the cuurent animation, the text will be scrolled across the 'eyes'
  * and then the eyes are returned to the neutral expression
  *
  * \param p  a pointer to a char aarray containing a nul terminated string. 
              The string must remain in scope while the message is being dispayed.
  */
  inline bool setText(char *pText) { if (_pText != nullptr) return(false); else _pText = pText; return(true); };

  /**
  * Animate the display.
  *
  * This method needs to be invoked as often as possible to ensure smooth animation. 
  *
  * The calling program should monitor the return value for 'true' in order to know when 
  * the animation has concluded. A 'true' return value means that the animation is complete.
  *
  * \return bool	true if the animation has completed, false otherwise.
  */
  bool runAnimation(void);

protected:
  // Animations FSM state
  typedef enum  
  {
    S_IDLE,
    S_RESTART, 
    S_ANIMATE,
    S_PAUSE,
    S_TEXT,
  } animState_t;

  // Define an animation frame
  typedef struct animFrame_t
  {
    uint8_t eyeData[2];  // [LEFT_MODULE_OFFSET] and [RIGHT_MODULE_OFFSET] eye character from font data
    uint16_t timeFrame;  // time for this frame in milliseconds
  };

  // Define an entry in the animation sequence lookup table
  typedef struct
  {
    emotion_t   e;
    animFrame_t *seq;
    uint8_t     size;
  } animTable_t;

  // Display parameters
  MD_MAX72XX  *_M;
  uint16_t _sd;   // start module for the display
 
  // Animation parameters
  uint32_t  _timeStartPause;
  uint32_t  _timeLastAnimation;
  uint16_t  _timeBlinkMinimum;
  animState_t _animState;
  bool      _autoBlink;
  uint16_t  _scrollDelay;

  // Animation control data
  animTable_t _animEntry;     // record with animation sequence parameters
  int8_t      _animIndex;     // current index in the animation sequence
  bool        _animReverse;   // true = reverse sequence, false = normal sequence
  bool        _autoReverse;   // true = always play the reverse, false = slected direction only
  emotion_t   _nextEmotion;   // the next emotion to display
  char *      _pText;         // pointer to text data in user code. Not null means there is text to print

  // Methods
  void loadEye(uint8_t module, uint8_t ch);
  void drawEyes(uint8_t L, uint8_t R);
  uint8_t loadSequence(emotion_t e);  // return the size of the sequence
  void loadFrame(animFrame_t* pBuf);
  void showText(bool bInit = false);

  void dumpSequence(const animFrame_t* pBuf, uint8_t numElements);  // debugging routine only

  // Static data tables
  static const animFrame_t seqBlink[], seqWink[];
  static const animFrame_t seqLeft[], seqRight[], seqUp[], seqDown[];
  static const animFrame_t seqAngry[], seqSad[], seqEvil[], seqEvil2[];
  static const animFrame_t seqSquint[], seqDead[];
  static const animFrame_t seqScanUpDown[], seqScanLeftRight[];

  // Lookup table to find animation
  static const animTable_t lookupTable[];
};
