// Use the MD_MAX72XX library to simuate an hourglass
//
// Uses the graphics functions to animate an hourglass on 
// two matrix modules.The matrices are placed diagonally touching
// at one point.
// 

#include <MD_MAX72xx.h>
#include <SPI.h>

#define DEBUG 0   // Enable or disable (default) debugging output

#if DEBUG
#define PRINT(s, v)   do { Serial.print(F(s)); Serial.print(v); } while(false)      // Print a string followed by a value (decimal)
#define PRINTX(s, v)  do { Serial.print(F(s)); Serial.print(v, HEX); } while(false) // Print a string followed by a value (hex)
#define PRINTS(s)     do { Serial.print(F(s)); } while(false)                       // Print a string
#define PRINTP(s, p)  do { Serial.print(F(s)); Serial.print(F(" [")); Serial.print(p.r); Serial.print(F(",")); Serial.print(p.c), Serial.print(F("]")); } while(false)
#else
#define PRINT(s, v)   // Print a string followed by a value (decimal)
#define PRINTX(s, v)  // Print a string followed by a value (hex)
#define PRINTS(s)     // Print a string
#define PRINTP(s, p)  // Print a point value
#endif

// --------------------
// MD_MAX72xx hardware definitions and object
// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
//
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 2
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);                      // SPI hardware interface
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES); // Arbitrary pins

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

// ========== Data Types ===========
//
typedef enum { ATT_HIHI = 0, ATT_HI = 1, ATT_LO = 2, ATT_LOLO = 3 } attractorId_t;
typedef enum { FLOW_HI2LO, FLOW_LO2HI } flowDirection_t;    // flow direction for particles (HI->LO or LO->HI)

// Define a generic point
typedef struct
{
  int8_t r, c;       // the row and column coordinates for this point
} coord_t;

// Define tracking data for a particle
typedef struct
{
  attractorId_t att;  // the id for the attractor relevelt to this element
  coord_t p;          // position calculated for this particle
} particle_t;

// --------------------
// Hourglass switch parameters
//
const uint8_t FLOW_SWITCH = 9;  // the hourglass has been rotated

// ========== General Variables ===========
//
particle_t particle[] =
{
// Initialize the attractor and coordinate for the particles
{ATT_HI,{7,7}}, {ATT_HI,{7,6}}, {ATT_HI,{7,5}}, {ATT_HI,{7,4}}, {ATT_HI,{7,3}}, {ATT_HI,{7,2}}, {ATT_HI,{7,1}}, {ATT_HI,{7,0}},
{ATT_HI,{6,7}}, {ATT_HI,{6,6}}, {ATT_HI,{6,5}}, {ATT_HI,{6,4}}, {ATT_HI,{6,3}}, {ATT_HI,{6,2}}, {ATT_HI,{6,1}}, {ATT_HI,{6,0}},
{ATT_HI,{5,7}}, {ATT_HI,{5,6}}, {ATT_HI,{5,5}}, {ATT_HI,{5,4}}, {ATT_HI,{5,3}}, {ATT_HI,{5,2}}, {ATT_HI,{5,1}},
{ATT_HI,{4,7}}, {ATT_HI,{4,6}}, {ATT_HI,{4,5}}, {ATT_HI,{4,4}}, {ATT_HI,{4,3}}, {ATT_HI,{4,2}},
{ATT_HI,{3,7}}, {ATT_HI,{3,6}}, {ATT_HI,{3,5}}, {ATT_HI,{3,4}}, {ATT_HI,{3,3}},
{ATT_HI,{2,7}}, {ATT_HI,{2,6}}, {ATT_HI,{2,5}}, {ATT_HI,{2,4}},
{ATT_HI,{1,7}}, {ATT_HI,{1,6}}, {ATT_HI,{1,5}},
{ATT_HI,{0,7}}, {ATT_HI,{0,6}}
};

flowDirection_t flowCur = FLOW_HI2LO;     // current flow direction
flowDirection_t flowPrev = FLOW_HI2LO;    // previous flow direction

// --------------------
// Constant parameters
//
const uint32_t TOTAL_TIMER = 30000; // in milliseconds
const uint32_t STEP_TIME = (TOTAL_TIMER / ARRAY_SIZE(particle));

const coord_t ATTRACTOR[4] =  // coordinates of attractors in the 'vertical' diagonal of the display
{
  {0, 0},     // ATT_HIHI
  {7, 7},     // ATT_HI
  {0, 8},     // ATT_LO
  {7, 15}     // ATT_LOLO
};

// ========== Control routines ===========
//

void updateDisplay(void)
{
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  mx.clear();
  for (uint8_t i = 0; i < ARRAY_SIZE(particle); i++)
    mx.setPoint(particle[i].p.r, particle[i].p.c, true);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

int16_t d2(const coord_t p, const coord_t q)
// work out the distance^2 between 2 points
{
  int16_t dr, dc;

  dr = (p.r - q.r);
  dr *= dr;
  dc = (p.c - q.c);
  dc *= dc;

  return (dr + dc);
}

int16_t findParticle(const coord_t p)
// Return the element index for the point p in the element array.
// If not found return -1.
{
  for (uint16_t i = 0; i < ARRAY_SIZE(particle); i++)
    if (particle[i].p.r == p.r && particle[i].p.c == p.c)
      return(i);

  return(-1);
}

void moveParticle(particle_t& pa)
{
  coord_t q;

  // set up the minimum to be the current point
  int16_t dMin = d2(pa.p, ATTRACTOR[pa.att]);
  int8_t drMin = 0, dcMin = 0;

  for (int8_t dr = -1; dr <= 1; dr++)
  {
    for (int8_t dc = -1; dc <= 1; dc++)
    {
      // new coordinates to test
      q.r = pa.p.r + dr;
      q.c = pa.p.c + dc;

      // avoid out of bounds conditions
      bool inbound = (q.r >= 0 && q.r < ROW_SIZE)     // fits vertically
                  && (((pa.att == ATT_HI || pa.att == ATT_HIHI) && (q.c >= 0 && q.c < COL_SIZE)) // hourglass top limits
                     || ((pa.att == ATT_LO || pa.att == ATT_LOLO) && (q.c >= COL_SIZE && q.c < mx.getColumnCount()))); // hourglass bottom limits
      if (inbound)
      {
        //work out the distance to the attractor
        int32_t d = d2(q, ATTRACTOR[pa.att]);

        // check for this being a new minimum and empty space
        // if distance is the same, then use a random value to flip a coin on choice
        if ((d < dMin || (d == dMin && random(100) < 50)) && findParticle(q) == -1)
        {
          drMin = dr;
          dcMin = dc;
          dMin = d;
        }
      }
    }
  }

  // save the minimum (which could be the same point)
  pa.p.r += drMin;
  pa.p.c += dcMin;
}

void moveAll(void)
{
  for (uint16_t i = 0; i < ARRAY_SIZE(particle); i++)
    moveParticle(particle[i]);
}

void setFlowDirection(void)
// detect the current flow direction and change over all the particle flows
// if it has changed.
{
  flowCur = (digitalRead(FLOW_SWITCH) == HIGH) ? FLOW_HI2LO : FLOW_LO2HI;

  // detect edge transition
  if (flowCur != flowPrev)
  {
    PRINTS("\n->FLOW change");
    flowPrev = flowCur;
    for (uint8_t i = 0; i < ARRAY_SIZE(particle); i++)
      switch (particle[i].att)
      {
      case ATT_HIHI:
      case ATT_HI:
        particle[i].att = (flowCur == FLOW_HI2LO) ? ATT_HI : ATT_HIHI; 
        break;
      case ATT_LO:
      case ATT_LOLO: 
        particle[i].att = (flowCur == FLOW_HI2LO) ? ATT_LOLO : ATT_LO; 
        break;
      }
  }
}

void checkTransition(void)
// Detect and handle transitions at the necking points (corners)
// depending on the flow direction.
// * FLOW_HI2LO transitions between ATT_HI to ATT_LO
// * FLOW_LO2HI transitions between ATT_LO to ATT_HI
{
  // is a particle ready to transition? 
  int16_t idx = findParticle(ATTRACTOR[(flowCur == FLOW_HI2LO) ? ATT_HI : ATT_LO]);  // top of the neck
  if (idx != -1)
  {
    PRINTP("\n->TRANSITION ready", particle[idx].p);
    // is there a space for it to move into?
    if (findParticle(flowCur == FLOW_HI2LO ? ATTRACTOR[ATT_LO] : ATTRACTOR[ATT_HI]) == -1) // bottom of the neck
    {
      // move the particle across the neck and set the new attractor
      particle[idx].p.r = (flowCur == FLOW_HI2LO) ? ATTRACTOR[ATT_LO].r : ATTRACTOR[ATT_HI].r;
      particle[idx].p.c = (flowCur == FLOW_HI2LO) ? ATTRACTOR[ATT_LO].c : ATTRACTOR[ATT_HI].c;
      particle[idx].att = (flowCur == FLOW_HI2LO) ? ATT_LOLO : ATT_HIHI;
    }
  }
}

void setup(void)
{
#if DEBUG
  Serial.begin(57600);
#endif
  PRINTS("\n[MD_MAX72XX Hourglass]");

  // matrix initialization
  mx.begin();
  updateDisplay();

  // set up the UI hardware
  pinMode(FLOW_SWITCH, INPUT_PULLUP);
}

void loop(void)
{
  static uint32_t timeLast = 0;

  // process user interface direction input
  setFlowDirection();

  // when time expires, work out the next animation frame
  if (millis() - timeLast >= STEP_TIME)
  {
    timeLast += STEP_TIME;
    moveAll();            // move all particles towards the attractor
    checkTransition();    // check movement from one segment of hourglass to the other
    updateDisplay();      // update the display with new frame
  }
}

