// Program to exercise the MD_MAX72XX library
//
// Test the library transformation functions

#include <MD_MAX72xx.h>
#include <SPI.h>

// Use a button to transfer between transformations or just do it on a timer basis
#define	USE_SWITCH_INPUT	1

#define	SWITCH_PIN	8	// switch pin if enabled - active LOW

// We always wait a bit between updates of the display
#define  DELAYTIME  500  // in milliseconds

// Number of times to repeat the transformation animations
#define	REPEATS_PRESET	16

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define	MAX_DEVICES		4
#define	WRAPAROUND_MODE	MD_MAX72XX::ON

#define	CLK_PIN		13  // or SCK
#define	DATA_PIN	11  // or MOSI
#define	CS_PIN		10  // or SS

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);


bool changeState(void)
{
  bool b = false;

#if USE_SWITCH_INPUT

  static int8_t	lastStatus = HIGH;
  int8_t	status = digitalRead(SWITCH_PIN);

  b = (lastStatus == HIGH) && (status == LOW);
  lastStatus = status;
#else
  static uint32_t	lastTime = 0;
  static uint8_t	repeatCount = 0;

  if (repeatCount == 0)
    repeatCount = REPEATS_PRESET;

  if (millis()-lastTime >= DELAYTIME)
  {
    lastTime = millis();
    b = (--repeatCount == 0);
  }
#endif

  return(b);
}

void transformDemo(MD_MAX72XX::transformType_t tt, bool bNew)
{
  static uint32_t lastTime = 0;

  if (bNew)
  {
    mx.clear();

    for (uint8_t i=0; i<MAX_DEVICES; i++)
    mx.setChar(((i+1)*COL_SIZE)-1, 'o'+i);
    lastTime = millis();
  }

  if (millis() - lastTime >= DELAYTIME)
  {
    mx.transform(0, MAX_DEVICES-1, tt);
    lastTime = millis();
  }
}

void setup()
{
  mx.begin();
  // use wraparound mode
  mx.control(MD_MAX72XX::WRAPAROUND, WRAPAROUND_MODE);

#if USE_SWITCH_INPUT
  pinMode(SWITCH_PIN, INPUT_PULLUP);
#endif

  Serial.begin(57600);
  Serial.println("[Transform Test]");
}

void loop()
{
  static int8_t tState = -1;
  static bool bNew = true;

  if (bNew)
  {
    tState = (tState+1) % 8;
    Serial.print("State: "); Serial.println(tState);
  }

  switch (tState)
  {
    case 0: transformDemo(MD_MAX72XX::TSL,	bNew);	break;
    case 1:	transformDemo(MD_MAX72XX::TSR,	bNew);	break;
    case 2:	transformDemo(MD_MAX72XX::TSU,	bNew);	break;
    case 3:	transformDemo(MD_MAX72XX::TSD,	bNew);	break;
    case 4:	transformDemo(MD_MAX72XX::TFUD,	bNew);	break;
    case 5:	transformDemo(MD_MAX72XX::TFLR,	bNew);	break;
    case 6:	transformDemo(MD_MAX72XX::TRC,	bNew);	break;
    case 7:	transformDemo(MD_MAX72XX::TINV,	bNew);	break;
    default:	tState = 0;	// just in case
  }

  bNew = changeState();
}

