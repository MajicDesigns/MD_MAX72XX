// Program to exercise the MD_MAX72XX library
//
// Test the library transformation functions with range subsets

#include <MD_MAX72xx.h>

// We always wait a bit between updates of the display
#define  DELAYTIME  300  // in milliseconds

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may 
// need to be adapted
#define	MAX_DEVICES		8	// 2, 4, 6, or 8 work best - see Z array

#define	CLK_PIN		13  // or SCK
#define	DATA_PIN	11  // or MOSI
#define	CS_PIN		10  // or SS

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Global variables
uint32_t	lastTime = 0;

typedef struct
{
  uint8_t	startDev;	// start of zone
  uint8_t	endDev;		// end of zone
  uint8_t	ch;		// character to show
  MD_MAX72XX::transformType_t	tt;
} zoneDef_t;

zoneDef_t Z[] = 
{
#if MAX_DEVICES == 2
  {0, 0, 26, MD_MAX72XX::TSR  },
  {1, 1, 27, MD_MAX72XX::TSL  },
#endif // MAX_DEVICES 2
#if MAX_DEVICES == 4
  {0, 0, 26, MD_MAX72XX::TSR  },
  {1, 1, 25, MD_MAX72XX::TSD  },
  {2, 2, 24, MD_MAX72XX::TSU  },
  {3, 3, 27, MD_MAX72XX::TSL  },
#endif // MAX_DEVICES 4
#if MAX_DEVICES == 6
  {0, 1, 26, MD_MAX72XX::TSR  },
  {2, 2, 24, MD_MAX72XX::TSU  },
  {3, 3, 25, MD_MAX72XX::TSD  },
  {4, 5, 27, MD_MAX72XX::TSL  },
#endif // MAX_DEVICES 6
#if MAX_DEVICES == 8
  {0, 1, 26, MD_MAX72XX::TSR  },
  {2, 2, 24, MD_MAX72XX::TSU  },
  {3, 3, 25, MD_MAX72XX::TSD  },
  {4, 4, 24, MD_MAX72XX::TSU  },
  {5, 5, 25, MD_MAX72XX::TSD  },
  {6, 7, 27, MD_MAX72XX::TSL  },
#endif // MAX_DEVICES 8
};

#define	ARRAY_SIZE(A)	(sizeof(A)/sizeof(A[0]))

void runTransformation(void) 
{
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  for (uint8_t i = 0; i < ARRAY_SIZE(Z); i++)
    mx.transform(Z[i].startDev, Z[i].endDev, Z[i].tt);

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void setup()
{
  Serial.begin(57600);
  Serial.println("[Zone Transform Test]");

  mx.begin();
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::ON);

  // set up the display characters
  for (uint8_t i = 0; i < ARRAY_SIZE(Z); i ++)
  {
    mx.clear(Z[i].startDev, Z[i].endDev);
    for (uint8_t j = Z[i].startDev; j <= Z[i].endDev; j++)
      mx.setChar(((j+1)*COL_SIZE)-2, Z[i].ch);
  }
  lastTime = millis();

  // Enable the display
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void loop() 
{
  if (millis() - lastTime >= DELAYTIME)
  {
    runTransformation();
    lastTime = millis();
  }
}

