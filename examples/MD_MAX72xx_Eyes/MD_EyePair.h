// Implements a class to draw and animate a pair of eyes
#ifndef MDEYEPAIR_H
#define MDEYEPAIR_H

#include <MD_MAX72xx.h>

// Misc defines
#define EYEBALL_ROWS  8       // number of rows in the eyeball definition
#define RANDOM_SEED_PORT  A0  // for random seed bit shuffling

class MD_EyePair
{
public:
  MD_EyePair(void);
  ~MD_EyePair(void) { };

  void begin(uint8_t startdev, MD_MAX72XX *M, uint16_t maxDelay);
  void animate(void);

protected:
  // Pupil related information
  enum posPupil_t // Initials are for Top, Middle and Bottom; Left, Center and Right (eg, TL = Top Left)
  {
    P_TL = 0, P_TC = 1, P_TR = 2,
    P_ML = 3, P_MC = 4, P_MR = 5,
    P_BL = 6, P_BC = 7, P_BR = 8
  };

  // Class static data
  static uint8_t  _pupilData[];
  static uint8_t  _eyeballData[];

  // display parameters
  MD_MAX72XX  *_M;
  uint8_t   _sd;  // start device
  uint8_t   _ed;  // end device

  // blinking parameters
  uint32_t  _lastBlinkTime;
  uint16_t  _currentDelay;
  uint8_t   _blinkState;
  uint8_t   _savedEyeball[EYEBALL_ROWS];
  uint8_t   _blinkLine;

  // animation parameters
  posPupil_t  _pupilCurPos;   // the current position for the pupil
  uint32_t  _timeLast;
  uint16_t  _timeDelay;
  uint16_t  _maxDelay;
  bool    _inBlinkCycle;

  // methods
  void drawEyeball(void);
  bool blinkEyeball(bool bFirst);
  void drawPupil(posPupil_t posOld, posPupil_t posNew);
  bool posIsAdjacent(posPupil_t posCur, posPupil_t posNew);
  
  // random seed creation
  uint16_t MD_EyePair::bitOut(uint8_t port);
  uint32_t MD_EyePair::seedOut(uint16_t noOfBits, uint8_t port);
};

#endif