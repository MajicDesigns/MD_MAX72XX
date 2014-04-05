// Test software to map display hardware rows and columns
// Generic SPI interface and only MAX72xx/8x8 LED module required
//
// Does not use any libraries as the code is used to directly map the display orientation
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


void setup(void)
{
  Serial.begin(57600);
  Serial.println("[MD_MAX72xx Hardware mapping]\n");
	
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

void mapSegment(char *label, uint8_t data)
{
  Serial.print("seg ");
  Serial.println(label);
  spiTransmit(OP_DIGIT0, data);
  delay(2000);
}

void mapDigit(uint8_t opCode)
{
  Serial.print("D");
  Serial.println(opCode - OP_DIGIT0);
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

  Serial.println("Segment Mapping");
  mapSegment("G", 1); 
  mapSegment("F", 2); 
  mapSegment("E", 4); 
  mapSegment("D", 8); 
  mapSegment("C", 16); 
  mapSegment("B", 32); 
  mapSegment("A", 64); 
  mapSegment("DP", 128); 
	
  clear();
	
  Serial.println("Mapping for Digits");
  mapDigit(OP_DIGIT0); 
  mapDigit(OP_DIGIT1); 
  mapDigit(OP_DIGIT2); 
  mapDigit(OP_DIGIT3); 
  mapDigit(OP_DIGIT4); 
  mapDigit(OP_DIGIT5); 
  mapDigit(OP_DIGIT6); 
  mapDigit(OP_DIGIT7); 
}
