// Test software to map display hardware rows and columns
// Generic SPI interface and only one MAX72xx/8x8 LED module required
//
// Does not use any libraries as the code is used to directly map the display orientation
// Observe the display and relate it to the MAX7219 hardware being exercised through the
// instructions and output on the serial monitor.
//
// NOTE: You need to change the hardware pins to match your specific setup

#define SERIAL_SPEED 57600

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

#define MAX_DIG 8
#define MAX_SEG 8

#define USER_DELAY  1000  // ms

void spiTransmit(uint8_t opCode, uint8_t data)
{
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
  Serial.print(F("\nINTRODUCTION\n------------"));
  Serial.print(F("\nHow the LED matrix is wired is important for the MD_MAX72xx library as different"));
  Serial.print(F("\nLED modules are wired differently. The library can accommodate these, but it"));
  Serial.print(F("\nneeds to know what transformations need to be carried out to map your board to the"));
  Serial.print(F("\nstandard coordinate system. This utility shows you how the matrix is wired so that"));
  Serial.print(F("\nyou can set the correct USE_*_HW #defines in the MD_MAX72xx.h library header file."));
  Serial.print(F("\n\nThe standard fucntions in the library expect that:"));
  Serial.print(F("\no COLUMNS are addressed through the SEGMENT selection lines, and"));
  Serial.print(F("\no ROWS are addressed through the DIGIT selection lines."));
  Serial.print(F("\n\nThe DISPLAY always has its origin in the top right corner of a display:"));
  Serial.print(F("\no LED matrix module numbers increase from right to left,"));
  Serial.print(F("\no Column numbers (ie, the hardware segment numbers) increase from right to left (0..7), and "));
  Serial.print(F("\no Row numbers (ie, the hardware digit numbers) increase down (0..7)."));
  Serial.print(F("\n\nThere are three hardware setting that describe your hardware configuration:"));
  Serial.print(F("\n- HW_DIG_ROWS - HardWare DIGits are ROWS. Set to 1 if the digits map to the rows"));
  Serial.print(F("\n                of the matrix, 0 otherwise"));
  Serial.print(F("\n- HW_REV_COLS - HardWare REVerse COLumnS. The normal column coordinates orientation"));
  Serial.print(F("\n                is 0 col on the right side of the display. Set to 1 to reverse this"));
  Serial.print(F("\n                (ie, hardware 0 is on the left)."));
  Serial.print(F("\n- HW_REV_ROWS - HardWare REVerse ROWS. The normal row coordinates orientation is 0"));
  Serial.print(F("\n                row at top of the display. Set to 1 to reverse this (ie, hardware 0"));
  Serial.print(F("\n                is at the bottom)."));
  Serial.print(F("\n\nThese individual setting are determined as a consequence of nominating the model type"));
  Serial.print(F("\nof the hardware you are using, you do not need to change these directly."));
  Serial.print(F("\n\nINSTRUCTIONS\n------------"));
  Serial.print(F("\n1. Wire up one matrix only, or cover up the other modules, to avoid confusion."));
  Serial.print(F("\n2. Enter the answers to the question in the edit field at the top of Serial Monitor."));
}

void setup(void)
{
  Serial.begin(SERIAL_SPEED);
  Serial.print(F("\n\n[MD_MAX72xx Hardware mapping utility]\n"));
  instructions();

  // Initialise comms hardware
  digitalWrite(CS_PIN, HIGH);
  pinMode(CS_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
}

void initialise(void)
// Initialise the display devices.
// On initial power-up, all control registers are reset, the
// display is blanked, and the MAX7219/MAX7221 enters shutdown
// mode.
{
  spiTransmit(OP_SHUTDOWN, 1);	// wake up
  spiTransmit(OP_SCANLIMIT, 7);	// all on
  spiTransmit(OP_INTENSITY, 7);	// mid intensity
  spiTransmit(OP_DISPLAYTEST, 0);	// no test
  spiTransmit(OP_DECODEMODE, 0);	// no decode
}

void mapSegment(char *label, uint8_t data)
{
  Serial.print(F("-"));
  Serial.print(label);
  spiTransmit(OP_DIGIT0, data);
  delay(USER_DELAY);
}

void mapDigit(uint8_t opCode)
{
  Serial.print(F("-"));
  Serial.print(opCode - OP_DIGIT0);
  spiTransmit(opCode, 0xff);
  delay(USER_DELAY);
  spiTransmit(opCode, 0x0);
}

void clear(void)
{
  for (uint8_t i=0; i<MAX_DIG; i++)
    spiTransmit(OP_DIGIT0 + i, 0);
}

char getResponse(char *validInput)
// blocking wait for user input from the serial monitor
{
  char  c = '\0';

  do
  {
    if (Serial.available())
    {
      uint8_t i;

      c = Serial.read();
      for (i=0; validInput[i] != '\0' && validInput[i] != c; i++)
        ; // set the index I to the matching character or nul if none - all work done in the loop
      c = validInput[i];  // could be nul character
    }
  } while (c == '\0');

  Serial.print(c);

  return(toupper(c));
}

void loop()
{
  boolean def_dig_rows, def_rev_cols, def_rev_rows;

  clear();

  Serial.print(F("\n\n======================================================"));
  Serial.print(F("\n\nSTEP 1 - DIGITS MAPPING (rows)\n------------------------------"));
  Serial.print(F("\nIn this step you will see a line moving across the LED matrix."));
  Serial.print(F("\nYou need to observe whether the bar is scanning ROWS or COLUMNS,"));
  Serial.print(F("\nand the direction it is moving."));
  Serial.print(F("\n>> Enter Y when you are ready to start: "));
  getResponse("Yy");

  initialise();

  Serial.print("\nDig");
  for (uint8_t i=0; i<MAX_DIG; i++)
    mapDigit(OP_DIGIT0+i);

  clear();

  Serial.print(F("\n>> Enter Y if you saw ROWS animated, N if you saw COLUMNS animated: "));
  def_dig_rows = (getResponse("YyNn") == 'Y');

  if (def_dig_rows)
    Serial.print(F("\n>> Enter Y if you saw the line moving BOTTOM to TOP, or enter N otherwise: "));
  else
    Serial.print(F("\n>> Enter Y if you saw the line moving LEFT to RIGHT, or enter N otherwise: "));
  def_rev_rows = (getResponse("YyNn") == 'Y');

  Serial.print(F("\n\nSTEP 2 - SEGMENT MAPPING (columns)\n----------------------------------"));
  Serial.print(F("\nIn this step you will see a dot moving along one edge of the LED matrix."));
  Serial.print(F("\nYou need to observe the direction it is moving."));
  Serial.print(F("\n>> Enter Y when you are ready to start: "));
  getResponse ("Yy");

  Serial.print(F("\nSeg"));
  mapSegment("G", 1);
  mapSegment("F", 2);
  mapSegment("E", 4);
  mapSegment("D", 8);
  mapSegment("C", 16);
  mapSegment("B", 32);
  mapSegment("A", 64);
  mapSegment("DP", 128);

  clear();

  if (def_dig_rows)
    Serial.print(F("\n>> Enter Y if you saw the LED moving LEFT to RIGHT, or enter N otherwise: "));
  else
    Serial.print(F("\n>> Enter Y if you saw the LED moving BOTTOM to TOP, or enter N otherwise: "));
  def_rev_cols = (getResponse("YyNn") == 'Y');

  Serial.print(F("\n\nSTEP 3 - RESULTS (#defines)\n---------------------------"));
  Serial.print(F("\nYour responses produce these configuration parameters\n"));
  Serial.print(F("\n#define\tHW_DIG_ROWS\t")); Serial.print(def_dig_rows ? 1 : 0 );
  Serial.print(F("\n#define\tHW_REV_COLS\t")); Serial.print(def_rev_cols ? 1 : 0 );
  Serial.print(F("\n#define\tHW_REV_ROWS\t")); Serial.print(def_rev_rows ? 1 : 0 );

  Serial.print(F("\n\nYour hardware matches the setting for "));
  if (def_dig_rows && def_rev_cols && !def_rev_rows)
    Serial.print(F("Parola modules. Please set USE_PAROLA_HW."));
  else if (!def_dig_rows && def_rev_cols && !def_rev_rows)
    Serial.print(F("Generic modules. Please set USE_GENERIC_HW."));
  else if (def_dig_rows && def_rev_cols && def_rev_rows)
    Serial.print(F("IC Station modules. Please set USE_ICSTATION_HW."));
  else if (def_dig_rows && !def_rev_cols && !def_rev_rows)
    Serial.print(F("FC-16 modules. Please set USE_FC16_HW."));
  else
  {
    Serial.print(F("none of the preconfigured module types."));
    Serial.print(F("\nYou should try rotating the matrix by 180 degrees and re-running this utility."));
    Serial.print(F("\n\nIf that still fails to provide a solution - congratulations! You have discovered"));
    Serial.print(F("\na new type of hardware module! Please contact the author of the libraries so that"));
    Serial.print(F("\nthese can be included in the next official release. In the meantime, you could"));
    Serial.print(F("\nselect USE_OTHER_HW and add your specific settings in the USE_OTHER_HW section"));
    Serial.print(F("\nin MD_MAX72xx_lib.h"));
  }
}
