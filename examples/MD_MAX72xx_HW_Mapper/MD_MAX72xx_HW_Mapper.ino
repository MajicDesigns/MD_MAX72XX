// Test software to map display hardware rows and columns
// Generic SPI interface and only one MAX72xx/8x8 LED module required
//
// Does not use any libraries as the code is used to directly map the display orientation
// Observe the display and relate it to the MAX7219 hardware being exercised through the 
// output on the serial monitor.
//

// Hardware definition
#define	CLK_PIN		13  // or SCK
#define	DATA_PIN	11  // or MOSI
#define	CS_PIN		10  // or SS

// Opcodes for the MAX7221 and MAX7219
// All OP_DIGITn are offsets from OP_DIGIT0
#define	OP_NOOP   		0	///< MAX72xx opcode for NO OP
#define OP_DIGIT0 		1	///< MAX72xx opcode for DIGIT0
#define OP_DIGIT1 		2	///< MAX72xx opcode for DIGIT1
#define OP_DIGIT2 		3	///< MAX72xx opcode for DIGIT2
#define OP_DIGIT3 		4	///< MAX72xx opcode for DIGIT3
#define OP_DIGIT4 		5	///< MAX72xx opcode for DIGIT4
#define OP_DIGIT5 		6	///< MAX72xx opcode for DIGIT5
#define OP_DIGIT6 		7	///< MAX72xx opcode for DIGIT6
#define OP_DIGIT7 		8	///< MAX72xx opcode for DIGIT7
#define OP_DECODEMODE  	9	///< MAX72xx opcode for DECODE MODE
#define OP_INTENSITY   10	///< MAX72xx opcode for SET INTENSITY
#define OP_SCANLIMIT   11	///< MAX72xx opcode for SCAN LIMIT
#define OP_SHUTDOWN    12	///< MAX72xx opcode for SHUT DOWN
#define OP_DISPLAYTEST 15	///< MAX72xx opcode for DISPLAY TEST

void spiTransmit(uint8_t opCode, uint8_t data) 
{
  /*
  Serial.print("Opcode: ");  Serial.print(opCode);
  Serial.print(" Data: ");   Serial.println(data);
  */

  // enable the devices to receive data
  digitalWrite(CS_PIN, LOW);

  // shift out the data 
  shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, opCode);
  shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, data);
		
  // latch the data onto the display
  digitalWrite(CS_PIN, HIGH);
}    

void instructions(void)
{
  Serial.print(F("\n** INTRODUCTION **"));
  Serial.print(F("\nHow the LED matrix is wired is important for the MD_MAX72xx library, and different"));
  Serial.print(F("\nboard modules are wired in different ways. The library can accommodate these, but"));
  Serial.print(F("\nit needs to know what transformations need to be carried out to map your board to the"));
  Serial.print(F("\nstandard coordinate system. This utility shows you how the matrix is wired so that"));
  Serial.print(F("\nyou can set the appropriate #defines in the library header file."));
  Serial.print(F("\n\nThe library expects that in the HARDWARE")); 
  Serial.print(F("\n- COLUMNS are addressed through the SEGMENT selection lines"));
  Serial.print(F("\n- ROWS are addressed through the DIGIT selection lines"));
  Serial.print(F("\nThe DISPLAY always has its origin in the top right corner of a display:")); 
  Serial.print(F("\n- LED matrix module numbers increase to the right"));
  Serial.print(F("\n- Column numbers (ie, the hardware segment numbers) increase from right to left (0..7)"));
  Serial.print(F("\n- Row numbers (ie, the hardware digit numbers) increase down (0..7)"));
  Serial.print(F("\n\nThere are three hardware setting that describe your hardware configuration:"));
  Serial.print(F("\n- HW_DIG_ROWS - HardWare DIGits are ROWS. Set to 1 if the digits map to the rows"));
  Serial.print(F("\n                of the matrix, 0 otherwise"));
  Serial.print(F("\n- HW_REV_COLS - HardWare REVerse COLumnS. The normal column coordinates orientation"));
  Serial.print(F("\n                is 0 col on the right side of the display. Set to 1 to reverse this"));
  Serial.print(F("\n                (ie, hardware 0 is on the left)."));
  Serial.print(F("\n- HW_REV_ROWS - HardWare REVerse ROWS. The normal row coordinates orientation is 0"));
  Serial.print(F("\n                row at top of the display. Set to 1 to reverse this (ie, hardware 0"));
  Serial.print(F("\n                is at the bottom)."));
  Serial.print(F("\n\n** INSTRUCTIONS **"));
  Serial.print(F("\n1. Wire up one matrix only."));
  Serial.print(F("\n2. The first row/column that illuminates during the test should be the zero column/row."));
  Serial.print(F("\n   If possible the board should be oriented so that this is in the top right hand corner."));
}

void setup(void)
{
  Serial.begin(57600);
  Serial.print(F("\n[MD_MAX72xx Hardware mapping utility]\n"));
  instructions();
	
  // Initialise comms hardware
  digitalWrite(CS_PIN, HIGH);
  pinMode(CS_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
	
  // Initialise the display devices. 
  // On initial power-up, all control registers are reset, the
  // display is blanked, and the MAX7219/MAX7221 enters shutdown 
  // mode.
  spiTransmit(OP_SHUTDOWN, 1);	// wake up
  spiTransmit(OP_SCANLIMIT, 7);	// all on
  spiTransmit(OP_INTENSITY, 7);	// mid intensity
  spiTransmit(OP_DISPLAYTEST, 0);	// no test
  spiTransmit(OP_DECODEMODE, 0);	// no decode
}

void mapSegment(char *label, uint8_t data)
{
  Serial.print("\nseg ");
  Serial.print(label);
  spiTransmit(OP_DIGIT0, data);
  delay(2000);
}

void mapDigit(uint8_t opCode)
{
  Serial.print("\nD");
  Serial.print(opCode - OP_DIGIT0);
  spiTransmit(opCode, 0xff);
  delay(2000);
  spiTransmit(opCode, 0x0);
}

void clear(void)
{
  for (uint8_t i=0; i<8; i++)
    spiTransmit(OP_DIGIT0 + i, 0);
}

void loop()
{
  clear();

  Serial.print(F("\n\n** DIGITS MAPPING (rows) **")); 
  mapDigit(OP_DIGIT0); 
  mapDigit(OP_DIGIT1); 
  mapDigit(OP_DIGIT2); 
  mapDigit(OP_DIGIT3); 
  mapDigit(OP_DIGIT4); 
  mapDigit(OP_DIGIT5); 
  mapDigit(OP_DIGIT6); 
  mapDigit(OP_DIGIT7); 
  Serial.print(F("\n->If you see ROWS animated then set HW_DIG_ROWS to 1."));
  Serial.print(F("\n->If you saw BARS scanning right to left (or bottom to top if"));
  Serial.print(F("\n  HW_DIG_ROWS) then set HW_REV_ROWS to 1."));

  Serial.print(F("\n\n** SEGMENT MAPPING (columns) **"));
  mapSegment("G", 1); 
  mapSegment("F", 2); 
  mapSegment("E", 4); 
  mapSegment("D", 8); 
  mapSegment("C", 16); 
  mapSegment("B", 32); 
  mapSegment("A", 64); 
  mapSegment("DP", 128); 
  Serial.print(F("\n-> If you saw LEDS scanning bottom to top (or right"));
  Serial.print(F("\n   to left if HW_DIG_ROWS) then set HW_REV_COLS to 1."));
	
  clear();
	
}
