/**
\mainpage Arduino LED Matrix Library
The Max72xx LED Controller IC
-----------------------------
The MAX7219/MAX7221 are compact, serial input/output display drivers that 
interface microprocessors to 7-segment numeric LED displays of up to 8 digits,
bar-graph displays, or 64 individual LEDs. Included on-chip are a BCD code-B 
decoder, multiplex scan circuitry, segment and digit drivers, and an 8x8 static 
RAM that stores each digit.

A convenient 4-wire serial interface (SPI) connects to all common microprocessors.
The devices may be cascaded, with communications passed through the first device 
in the chain to all others.

Individual digits may be addressed and updated without rewriting the entire 
display. The MAX72XX also allow the user to select code-B decoding or no-decode
for each digit.

The devices include
- a 150uA low-power shutdown mode
- analog and digital brightness control
- a scan limit register that allows the user to display from 1 to 8 digits
- a test mode that forces all LEDs on

Topics
------
- \subpage pageHardware
- \subpage pageSoftware
- \subpage pageConnect

Revision History 
----------------
February 2014 - version 2.3
- Complete rework of the font system
-- New font builder tool available
-- Removed USE_FONT_ADJUST and related code as font builder tool now available
-- Fixed width font has been removed from the code
-- fontype_t definition changed to suit new requirements

November 2013 - version 2.2
- Replaced reference to SPI library with inline code to allow for different select lines
- Obsoleted INCLUDE_HARDWARE_SPI conditional compile switch
- Fixed legacy code function name error when USE_FONT_ADJUST switch turned on
- Implemented USE_PAROLA_HW to allow cheaply available matrix modules to be used in ganged mode
- Fixed reversal of bit field for set/get Row/Column functions -> flipped charset data
- Added Eyes example program
- Upgraded and reorganised documentation

June 2013 - version 2.1
- Include the selection of hardware SPI interface (10x speed improvement)
- Tidied up comments

April 2013 - version 2.0 
- Major update and rewrite of library code:
- Improved speed and efficiency of code
- Increased level of abstraction in the library for pixel methods
- Increased level of abstraction for character and font methods
- Increased number of functions and added variable sized font
- Changed defines to enumerated types within the scope of the class
- Updated functionality to simplify controlling multiple devices 
- Changed text and comments to be aligned to doxygen documentation generation

June 2012 - version 1.0
- Incorporated elements of Arduino LedControl (Eberhard Fahle) and MAX7219 libraries
- Easier functionality for pixel graphics treatment of 8x8 matrices

Copyright
---------
Copyright (C) 2012-13 Marco Colli. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\page pageSoftware Software
The Library
-----------
The library implements functions that allow the MAX72xx to be used
for LED matrices (64 individual LEDs), allowing the programmer to use the LED 
matrix as a pixel device, displaying graphics elements much like any other 
pixel addressable display.

In this scenario, it is convenient to abstract out the concept of the hardware device 
and create a uniform and consistent pixel address space, with the libraries determining 
device and device-element address. Similarly, control of the devices should be uniform 
and abstracted to a system level.

The library still retains flexibility for device level control, should the developer 
require, through the use of overloaded class methods.
___

Conditional Compilation Switches
--------------------------------
The library allows the run time code to be tailored through the use of compilation
switches. The compile options start with USE_ and are documented in the section 
related to the main header file MD_MAX72xx.h.
___

Font Storage Format
-------------------
One default font is defined as part of the library in PROGMEM memory. Alternative fonts
can be specified to the library. The font builder tool provides a convenient way to develop
alternative fonts.

The fonts are stored as a series of contiguous bytes in the following format:
- byte 1 - the number of bytes that form this character (could be zero)
- byte 2..n - each byte is a column of the character to be formed, starting with the 
leftmost column of the character. The least significant bit of the byte is the bottom 
pixel position of the character matrix (row 7).
 
To find a character in the font table, the library looks at the first byte (size), 
skips 'size'+1 bytes to the next character size byte and repeat until the last or 
target character is reached.

The compile-time switch INDEX_FONT enables indexing of the font table for faster access, at 
the expense of increased RAM usage. If indexing is enabled, a single lookup is required to 
access the character data, rather than the sequential search described above.

The support for fonts (methods and data) may be completely disabled  if not required through 
the compile-time switch INCLUDE_LOCAL_FONT. This will also disable user defined fonts.

\page pageHardware Hardware
Hardware Supported
------------------
This library supports the Parola hardware and the more commonly available LED modules available 
from many low-cost sources. The circuits for the these modules are essentially the same except 
but the LED matrix is oriented differently.

![Parola module] (Parola_Module.png "Parola Module") 
![Generic Module] (Generic_Module.png "Generic Module")

___

Circuit Schematic
-----------------
The schematic is the basic application circuit that is found on the MAX7219 datasheet, 
adapted to the LED matrix. Each Module consists of an 8x8 LED Common Anode matrix
controlled by a MAX7219 LED controller and a few passive components. These controllers 
can be daisy chained, making them ideal for the purpose.

![Parola Circuit Schematic] (Circuit_Schematic.jpg "Module Schematic")

The Parola and generic modules LED matrices are rotated with respect to each other.
This difference is accounted for in software when the type of module is selected 
using the USE_PAROLA_HW compile time switch.
___

Wiring your own matrix
----------------------

How the LED matrix is wired is important for the library. The matrix used for library 
development was a **common anode** type labelled 1088B. Connections should be made as 
described in the table below to be consistent with the assumptions in the software library.
- Columns are addressed through the digits section lines 
- Rows are are addressed through the LED segment selection lines

Matrix|MAX Signal|MAX7219 pin|
:----:|:--------:|----------:|
Col1  |Dig0      |2          | 
Col2  |Dig1      |11         |
Col3  |Dig2      |6          |
Col4  |Dig3      |7          |
Col5  |Dig4      |3          |
Col6  |Dig5      |10         |
Col7  |Dig6      |5          |
Col8  |Dig7      |8          |
Row1  |SegDP     |22         |
Row2  |SegA      |14         |
Row3  |SegB      |16         |
Row4  |SegC      |20         |
Row5  |SegD      |23         |
Row6  |SegE      |21         |
Row7  |SegF      |15         |
Row8  |SegG      |17         |

\page pageConnect System Connections
Module to Arduino Connection (SPI interface)
--------------------------------------------
The modules are connected through a 4-wire serial interface (SPI), and devices are cascaded, 
with communications passed through the first device in the chain to all others. The Arduino 
should be connected to the IN side (shown in the figure below) of the first module in the chain. 

The Arduino interface is implemented with either
+ The hardware SPI interface, or
+ 3 arbitrary digital outputs that are passed through to the class constructor. 

The AVR hardware SPI interface is fast but fixed to predetermined output pins. The more general 
software interface uses the Arduino shiftOut() library function, making it slower but allows the 
use of arbitrary digital pins to send the data to the device. Which mode is enabled depends 
on the class constructor used.

The Arduino interface is implemented with 3 digital outputs that are passed through to 
the class constructor. The digital outputs define the SPI interface as follows:
- DIN (MOSI) - the Data IN signal shifts data into the display module. Data is loaded into 
the device's internal 16-bit shift register on CLK's rising edge.
- CLK (SCK) - the CLocK signal that is used to time the data for the device. 
- LD (SS) - the interface is active when LoaD signal is LOW. Serial data are loaded into the 
device shift register while LOAD is LOW and latched in on the rising edge.

Note that the LD signal is used to select the entire device chain. This allows separate LD 
outputs to control multiple displays sharing the same DIN and CLK signals. The 
software needs to instantiate a separate object for each display.

The remaining interface pins are for +5V and GND. The power supply must be able to supply 
enough current for the number of connected modules. The central position of the Parola GND
connector provides some protection for accidentally reversing the connector.
___

Connecting Multiple Modules
---------------------------
Separate modules are connected by the plugging them together edge to edge, with the 
OUT side of one module plugged to the IN side of the next, as shown in the figures below.

Parola modules are connected by plugging them together.
![Connecting Parola modules] (Parola_conn.jpg "Parola Modules connected")

Generic modules may need to be rotated and connected using short patch cables in a 
spiral pattern.
![Connecting Generic modules] (Generic_conn.jpg "Generic Modules connected")
*/
#ifndef MD_MAX72xx_h
#define MD_MAX72xx_h

#include <Arduino.h>

/**
 * \file
 * \brief Main header file for the MD_MAX72xx library
 */

/**
 \def USE_PAROLA_HW
 Set to 1 (default) to use the Parola hardware modules. The library 
 may be used with similar generic modules (with top and bottom connectors),
 available from many sources, by setting this switch to 0.
 */
#define	USE_PAROLA_HW	1

/**
 \def USE_LOCAL_FONT
 Set to 1 (default) to enable local font in this library and enable 
 loadChar() and related methods. If the library is just used for 
 graphics some memory can be saved by not including the font data.
 */
#define	USE_LOCAL_FONT	1

/**
 \def USE_INDEX_FONT
 Set to 1 to enable font indexing to speed up font lookups - usually disabled.
 This will trade off increased stack RAM usage for lookup speed if enabled. 
 When disabled lookups will then become linear searches through PROGMEM.
 Uses FONT_INDEX_SIZE elements of uint16_t (512 bytes) if enabled.

 USE_LOCAL FONT must be enabled for this option to take effect.
 */
#define	USE_INDEX_FONT	0

// Display parameter constants
// Defined values that are used throughout the library to define physical limits
#define	ROW_SIZE	8		///< The size in pixels of a row in the device LED matrix array
#define COL_SIZE    8		///< The size in pixels of a column in the device LED matrix array
#define	MAX_INTENSITY	0xf	///< The maximum intensity value that can be set for a LED array
#define	MAX_SCANLIMIT	7	///< The maximum scan limit value that can be set for the devices

/**
 * Core object for the Parola library
 */
class MD_MAX72XX
{
public:
#if USE_LOCAL_FONT
	/**
	 * Font definition type.
	 *
	 * This type is used in the setFont() method to set the font to be used
	 */
	typedef uint8_t PROGMEM	*	fontType_t;
#endif

	/**
	 * Control Request enumerated type.
	 *
	 * This enumerated type is used with the control() method to identify 
	 * the control action request.
	 */
	enum controlRequest_t
	{
		SHUTDOWN = 0,	///< Shut down the MAX72XX. Requires ON/OFF value
		SCANLIMIT = 1,	///< Set the scan limit for the MAX72XX. Requires numeric value [0..MAX_SCANLIMIT]
		INTENSITY =	2,	///< Set the LED intensity for the MAX72XX. Requires numeric value [0..MAX_INTENSITY]
		TEST = 3,		///< Set the MAX72XX in test mode. Requires ON/OFF value.
		UPDATE = 10,	///< Enable or disable auto updates of the devices from the library. Requires ON/OFF value.
		WRAPAROUND = 11	///< Enable or disable wraparound when shifting (circular buffer). Requires ON/OFF value.
	};

	/**
	 * Control Value enumerated type.
	 *
	 * This enumerated type is used with the control() method as the 
	 * ON/OFF value for a control request. Other values may be used 
	 * if numeric data is required.
	 */
	enum controlValue_t
	{
		OFF = 0,	///< General OFF status request
		ON = 1		///< General ON status request
	};

	/**
	 * Transformation Types enumerated type.
	 *
	 * This enumerated type is used in the transform() methods to identify a 
	 * specific transformation of the display data in the device buffers.
	 */
	enum transformType_t
	{
		TSL,	///< Transform Shift Left one pixel element
		TSR,	///< Transform Shift Right one pixel element
		TSU,	///< Transform Shift Up one pixel element
		TSD,	///< Transform Shift Down one pixel element
		TFLR,	///< Transform Flip Left to Right
		TFUD,	///< Transform Flip Up to Down
		TRC,	///< Transform Rotate Clockwise 90 degrees
		TINV	///< Transform INVert (pixels inverted)
	};

  /** 
   * Class Constructor - arbitrary digital interface.
   *
   * Instantiate a new instance of the class. The parameters passed are used to 
   * connect the software to the hardware. Multiple instances may co-exist 
   * but they should not share the same hardware CS pin (SPI interface).
   * 
   * \param dataPin		output on the Arduino where data gets shifted out.
   * \param clkPin		output for the clock signal.
   * \param csPin		output for selecting the device.
   * \param numDevices	number of devices connected. Default is 1 if not supplied. 
   *                    Memory for device buffers is dynamically allocated based 
   *                    on this parameter.
   */
  MD_MAX72XX(uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices=1);

  /** 
   * Class Constructor - SPI hardware interface.
   *
   * Instantiate a new instance of the class. The parameters passed are used to 
   * connect the software to the hardware. Multiple instances may co-exist 
   * but they should not share the same hardware CS pin (SPI interface).
   * The dataPin and the clockPin are defined by the Arduino hardware definition
   * (SPI MOSI and SCK signals).
   * 
   * \param csPin		output for selecting the device.
   * \param numDevices	number of devices connected. Default is 1 if not supplied. 
   *                    Memory for device buffers is dynamically allocated based 
   *                    on this parameter.
   */
  MD_MAX72XX(uint8_t csPin, uint8_t numDevices=1);

  /** 
   * Initialise the object.
   *
   * Initialise the object data. This needs to be called during setup() to initialise new 
   * data for the class that cannot be done during the object creation.
   */
  void begin(void);

  /** 
   * Class Destructor.
   *
   * Released allocated memory and does the necessary to clean up once the object is
   * no longer required.
   */
  ~MD_MAX72XX();

  //--------------------------------------------------------------
  /** \name Methods for object and hardware control.
   * @{
   */
  /** 
   * Set the control status of the specified parameter for the specified device.
   * 
   * The device has a number of control parameters that can be set through this method. 
   * The type of control action required is passed through the mode parameter and 
   * should be one of the control actions defined by controlRequest_t. The value that 
   * needs to be supplied on the control action required is one of the defined 
   * actions in controlValue_t or a numeric parameter suitable for the control action.
   *
   * \param dev			address of the device to control [0..getDeviceCount()-1].
   * \param mode		one of the defined control requests.
   * \param value		parameter value or one of the control status defined.
   * \return false if parameter errors, true otherwise.
   */
  bool control(uint8_t dev, controlRequest_t mode, int value);

  /** 
   * Set the control status of the specified parameter for all devices.
   * 
   * Invokes the control function for each device in turn. See documentation for the control() method.
   *
   * \param mode		one of the defined control requests.
   * \param value		parameter value or one of the control status defined.
   * \return no return value.
   */
  void control(controlRequest_t mode, int value);

  /**
   * Gets the number of devices attached to this class instance.
   *
   * \return uint8_t representing the number of devices attached to this object.
   */
  uint8_t getDeviceCount(void) { return(_maxDevices); };

  /**
   * Gets the maximium number of columns for devices attached to this class instance.
   *
   * \return uint16_t representing the number of devices attached to this object.
   */
  uint16_t getColumnCount(void) { return(_maxDevices*COL_SIZE); };

  /** 
   * Set the Shift Data In callback function.
   *
   * The callback function is called from the library when a transform shift left 
   * or shift right operation is executed and the library needs to obtain data for 
   * the last element of the shift (ie, conceptually this is the new data that is 
   * shifted 'into' the display). The callback function is invoked when
   * - WRAPAROUND is not active, as the data is automatically supplied within the library.
   * - the call to transform() is global (ie, not for an individual buffer).
   *  
   * The callback function takes one parameter (one of the transformation 
   * types in transformType_t) that tells the callback function what shift is being 
   * performed and the return value is the data for the column to be shifted into 
   * the display.
   * 
   * \param cb	the address of the function to be called from the library.
   * \return No return data
   */
  void setShiftDataInCallback(uint8_t (*cb)(transformType_t t)) { _cbShiftDataIn = cb; };

  /** 
   * Set the Shift Data Out callback function.
   *
   * The callback function is called from the library when a transform shift left 
   * or shift right operation is executed and the library is about to discard the data for 
   * the first element of the shift (ie, conceptually this is the data that 'falls' off 
   * the front end of the scrolling display). The callback function is invoked when
   * - WRAPAROUND is not active, as the data is automatically supplied to the tail end.
   * - the call to transform() is global (ie, not for an individual buffer).
   *  
   * The callback function is with supplied 2 parameters, with no return value required:
   * - one of the transformation types transformType_t that tells the callback 
   * function the type of shifting being executed
   * - the data for the column being shifted out
   * 
   * \param cb	the address of the function to be called from the library.
   * \return No return data
   */
  void setShiftDataOutCallback(void (*cb)(transformType_t t, uint8_t colData)) { _cbShiftDataOut = cb; };
  
  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for graphics and bitmap related abstraction.
   * @{
   */
  /**
   * Clear all the display data on all the display devices.
   *
   * \return no return value.
   */
  void clear(void);

  /**
   * Draw a line between two points on the display
   *
   * Draw a line between the specified points. The LED will be turned on or 
   * off depending on the value supplied. The column number will be dereferenced 
   * into the device and column within the device, allowing the LEDs to be treated 
   * as a continuous pixel field.
   *
   * \param r1		starting row coordinate for the point [0..ROW_SIZE-1].
   * \param c1		starting column coordinate for the point [0..getColumnCount()-1].
   * \param r2		ending row coordinate for the point [0..ROW_SIZE-1].
   * \param c2		ending column coordinate for the point [0..getColumnCount())-1].
   * \param state	true - switch on; false - switch off.
   * \return false if parameter errors, true otherwise.
   */
  bool drawLine(uint8_t r1, uint16_t c1, uint8_t r2, uint16_t c2, bool state);

 /** 
   * Load a bitmap from the display buffers to a user buffer.
   *
   * Allows the calling program to read bitmaps (characters or graphic)
   * elements from the library display buffers. The data buffer 
   * pointer should be a block of uint8_t data of size elements that will 
   * contain the returned data.
   * 
   * \param col		address of the display column [0..getColumnCount()-1].
   * \param size	number of columns of data to return.
   * \param *pd		Pointer to a data buffer [0..size-1].
   * \return false if parameter errors, true otherwise. If true, data will be in the buffer at *pd.
   */
  bool getBuffer(uint16_t col, uint8_t size, uint8_t *pd);
 
  /**
   * Get the LEDS status for the specified column.
   *
   * This method operates on a specific buffer
   * 
   * This method operates on one column, getting the bit field value of 
   * the LEDs in the column. The column is referenced with the absolute column 
   * number (ie, the device number is inferred from the column).
   * 
   * \param c		column which is to be set [0..getColumnCount()-1].
   * \return uint8_t value with each bit set to 1 if the corresponding LED is lit. 0 is returned for parameter error.
   */
  uint8_t getColumn(uint8_t c) { return getColumn((c / COL_SIZE), c % COL_SIZE); };

  /** 
   * Get the status of a single LED, addressed as a pixel.
   *
   * The method will get the status of a specific LED element based on its 
   * coordinate position. The column number is dereferenced into the device 
   * and column within the device, allowing the LEDs to be treated as a 
   * continuous pixel field.
   * 
   * \param r		row coordinate for the point [0..ROW_SIZE-1].
   * \param c		column coordinate for the point [0..getColumnCount()-1].
   * \return true if LED is on, false if off or parameter errors.
   */
  bool getPoint(uint8_t r, uint16_t c);

 /** 
   * Load a bitfield from the user buffer to a display buffer.
   *
   * Allows the calling program to define bitmaps (characters or graphic)
   * elements and pass them to the library for display. The data buffer 
   * pointer should be a block of uint8_t data of size elements that define 
   * the bitmap.
   * 
   * \param col		address of the display column [0..getColumnCount()-1].
   * \param size	number of columns of data following.
   * \param *pd		Pointer to a data buffer [0..size-1].
   * \return false if parameter errors, true otherwise.
   */
  bool setBuffer(uint16_t col, uint8_t size, uint8_t *pd);

  /**
   * Set all LEDs in a specific column to a new state.
   *
   * This method operates on one column, setting the value of the LEDs in 
   * the column to the specified value bit field. The column is 
   * referenced with the absolute column number (ie, the device number is 
   * inferred from the column). The method is useful for drawing vertical 
   * lines and patterns when the display is being treated as a pixel field.
   * The least significant bit of the value is the lowest row number.
   * 
   * \param c		column which is to be set [0..getColumnCount()-1].
   * \param value	each bit set to 1 will light up the corresponding LED.
   * \return false if parameter errors, true otherwise.
   */
  bool setColumn(uint8_t c, uint8_t value) { return setColumn((c / COL_SIZE), c % COL_SIZE, value); };

  /** 
   * Set the status of a single LED, addressed as a pixel.
   *
   * The method will set the value of a specific LED element based on its 
   * coordinate position. The LED will be turned on or off depending on the 
   * value supplied. The column number is dereferenced into the device and 
   * column within the device, allowing the LEDs to be treated as a 
   * continuous pixel field.
   * 
   * \param r		row coordinate for the point [0..ROW_SIZE-1].
   * \param c		column coordinate for the point [0..getColumnCount()-1].
   * \param state	true - switch on; false - switch off.
   * \return false if parameter errors, true otherwise.
   */
  bool setPoint(uint8_t r, uint16_t c, bool state);

  /**
   * Set all LEDs in a row to a new state on all devices.
   *
   * This method operates on all devices, setting the value of the LEDs in 
   * the row to the specified value bit field. The method is useful for 
   * drawing patterns and lines horizontally across on the entire display.
   * The least significant bit of the value is the lowest column number.
   * 
   * \param r	   row which is to be set [0..ROW_SIZE-1].
   * \param value  each bit set to 1 will light up the corresponding LED on each device.
   * \return false if parameter errors, true otherwise.
   */
  bool setRow(uint8_t r, uint8_t value);

  /** 
   * Apply a transformation to the data in all the devices.
   *
   * The buffers for all devices can be transformed using one of the enumerated 
   * transformations in transformType_t. The transformation is carried across 
   * device boundaries (ie, there is overflow to an adjacent devices if appropriate).
   * 
   * \param ttype  one of the transformation types in transformType_t.
   * \return false if parameter errors, true otherwise.
   */
  bool transform(transformType_t ttype);

  /** 
   * Turn auto display updates on or off.
   *
   * Turn auto updates on and off, as required. When auto updates are turned OFF the 
   * display will not update after each operation. Display updates can be forced at any 
   * time using using a call to update() with no parameters.
   *
   * This function is a convenience wrapper for the more general control() function call.
   * 
   * \param mode	one of the types in controlValue_t (ON/OFF).
   * \return bool value returned by control().
   */
  bool update(controlValue_t mode) { control(UPDATE, mode); };

  /** 
   * Force an update of all devices 
   *
   * Used when auto updates have been turned off through the control 
   * method. This will force all buffered changes to be written to 
   * all the connected devices.
   * 
   * \return no return value.
   */
  void update(void) { flushBufferAll(); };

  /** 
   * Turn display wraparound on or off.
   *
   * When shifting left or right, up or down, the outermost edge is normally lost and a blank 
   * row or column inserted on the opposite side. If this options is enabled, the edge is wrapped 
   * around to the opposite side.
   *
   * This function is a convenience wrapper for the more general control() function call.
   * 
   * \param mode	one of the types in controlValue_t (ON/OFF).
   * \return bool value returned by control().
   */
  bool wraparound(controlValue_t mode) { control(WRAPAROUND, mode); };
  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for managing specific devices or display buffers.
   * @{
   */
  /**
   * Clear all display data in the specified buffer.
   *
   * \param buf		address of the buffer to clear [0..getDeviceCount()-1].
   * \return false if parameter errors, true otherwise.
   */
  bool clear(uint8_t buf);

   /** 
   * Get the state of the LEDs in a specific column.
   * 
   * This method operates on the specific buffer, returning the bit field value of 
   * the LEDs in the column.
   * 
   * \param buf		address of the display [0..getDeviceCount()-1].
   * \param c		column which is to be set [0..COL_SIZE-1].
   * \return uint8_t value with each bit set to 1 if the corresponding LED is lit. 0 is returned for parameter error.
   */
  uint8_t getColumn(uint8_t buf, uint8_t c);

/**
   * Get the state of the LEDs in a specified row.
   * 
   * This method operates on the specific buffer, returning the bit field value of 
   * the LEDs in the row.
   *
   * \param buf		address of the display [0..getDeviceCount()-1].
   * \param r		row which is to be set [0..ROW_SIZE-1].
   * \return uint8_t value with each bit set to 1 if the corresponding LED is lit. 0 is returned for parameter error.
   */
  uint8_t getRow(uint8_t buf, uint8_t r);

  /** 
   * Set all LEDs in a column to a new state.
   * 
   * This method operates on a specific buffer, setting the value of the LEDs in 
   * the column to the specified value bit field. The method is useful for 
   * drawing patterns and lines vertically on the display device.
   * The least significant bit of the value is the lowest column number.
   * 
   * \param buf		address of the display [0..getDeviceCount()-1].
   * \param c		column which is to be set [0..COL_SIZE-1].
   * \param value   each bit set to 1 will light up the	corresponding LED.
   * \return false if parameter errors, true otherwise.
   */
  bool setColumn(uint8_t buf, uint8_t c, uint8_t value);

  /**
   * Set all LEDs in a row to a new state.
   * 
   * This method operates on a specific device, setting the value of the LEDs in 
   * the row to the specified value bit field. The method is useful for 
   * drawing patterns and lines horizontally across the display device.
   * The least significant bit of the value is the lowest row number.
   *
   * \param buf		address of the display [0..getDeviceCount()-1].
   * \param r		row which is to be set [0..ROW_SIZE-1].
   * \param value   each bit set to 1 within this byte will light up the corresponding LED.
   * \return false if parameter errors, true otherwise.
   */
  bool setRow(uint8_t buf, uint8_t r, uint8_t value);

  /** 
   * Apply a transformation to the data in the specified device.
   *
   * The buffer for one device can be transformed using one of the enumerated 
   * transformations in transformType_t. The transformation is limited to the 
   * nominated device buffer only (ie, there is no overflow to an adjacent device).
   * 
   * \param buf	   address of the display [0..getBufferCount()-1].
   * \param ttype  one of the transformation types in transformType_t.
   * \return false if parameter errors, true otherwise.
   */
  bool transform(uint8_t buf, transformType_t ttype);

  /** 
   * Force an update of one buffer.
   *
   * Used when auto updates have been turned off through the control() 
   * method. This will force all buffered display changes to be written to 
   * the specified device at the same time.
   * Note that control() messages are not buffered but cause immediate action.
   * 
   * \param buf	address of the display [0..getBufferCount()-1].
   * \return false if parameter errors, true otherwise.
   */
  bool update(uint8_t buf) { flushBuffer(buf); };
  /** @} */

#if USE_LOCAL_FONT
  //--------------------------------------------------------------
  /** \name Methods for font and characters.
   * @{
   */
  /** 
   * Load a character from the font data into a user buffer.
   *
   * Copy the bitmap for a library font character (current font set by setFont()) and
   * return it in the data area passed by the user. If the user buffer is not large 
   * enough, only the first size elements are copied to the buffer.
   *
   * NOTE: This function is only available if the library defined value
   * INCLUDE_LOCAL_FONT is set to 1.
   * 
   * \param c		the character to retrieve.
   * \param size	the size of the user buffer in unit8_t units.
   * \param buf		address of the user buffer supplied.
   * \return width (in columns) of the character, 0 if parameter errors.
   */
  uint8_t getChar(uint8_t c, uint8_t size, uint8_t *buf);

  /** 
   * Load a character from the font data starting at a specific column.
   *
   * Load a character from the font table directly into the display at the column 
   * specified. The currently selected font table is used as the source.
   *
   * NOTE: This function is only available if the library defined value
   * INCLUDE_LOCAL_FONT is set to 1.
   * 
   * \param col		column of the display in the range accepted [0..getColumnCount()-1].
   * \param c		the character to display.
   * \return width (in columns) of the character, 0 if parameter errors.
   */
  uint8_t setChar(uint16_t col, uint8_t c);

  /** 
   * Set the current font table.
   *
   * Font data is stored in PROGMEM, in the format described elsewhere in the 
   * documentation. All characters retrieved or used after this call will use 
   * the nominated font (default or user defined). To specify a user defined 
   * character set, pass the PROGMEM address of the font table. Passing a NULL 
   * pointer resets the font table to the library default table.
   * This function also causes the font index table to be recreated if the 
   * library defined value INDEX_TABLE is set to 1.
   *
   * NOTE: This function is only available if the library defined value
   * INCLUDE_LOCAL_FONT is set to 1.
   * 
   * \param f	fontType_t pointer to the table of font data in PROGMEM or NULL.
   * \return false if parameter errors, true otherwise.
   */
  bool setFont(fontType_t f);
#endif // INCLUDE_LOCAL_FONT
  /** @} */  

#if USE_FONT_ADJUST
  void adjustFont();		// utility routine for font fle transformations
#endif // ENABLE_FONT_ADJUST

  
private:
  typedef struct 
  {
	uint8_t row[ROW_SIZE];	// data for each row of the display
	uint8_t changed;        // one bit for each row changed ('dirty bit')
  } deviceInfo_t;

  // SPI interface data
  uint8_t	_dataPin;		// DATA is shifted out of this pin ...
  uint8_t	_clkPin;		// ... signalled by a CLOCK on this pin ...
  uint8_t	_csPin;			// ... and LOADed when the chip select pin is driven HIGH to LOW
  bool		_hardwareSPI;	// true if SPI interface is the hardware interface
	
  // Device buffer data
  uint8_t	_maxDevices;	// maximum number of devices in use
  deviceInfo_t*	_matrix;	// the current status of the LED matrix
  uint8_t*	_spiData;		// data buffer for writing to SPI interface

  // User callback function for shifting operations
  uint8_t	(*_cbShiftDataIn)(transformType_t t);
  void		(*_cbShiftDataOut)(transformType_t t, uint8_t colData);
	
  // Control data for the library
  bool		_updateEnabled; // update the display when this is true, suspend otherwise
  bool		_wrapAround;	// when shifting, wrap left to right and vice versa (circular buffer)

#if USE_LOCAL_FONT
  // Font related data
  fontType_t	_fontData;				// pointer to the current font data being used
  uint16_t		*_fontIndex;			// font index for faster access to font table offsets

  uint16_t	getFontCharOffset(uint8_t c);	// find the character in the font data 
  void		buildFontIndex(void);			// build a font index
#endif
  // Private functions
  void spiTransmit(void);			// do the actual physical communications task
  void spiSend(uint8_t dev, uint8_t opcode, uint8_t data);	// Send a single command to the correct device
  void spiClearBuffer(void);		// clear the SPI send buffer

  void flushBuffer(uint8_t buf);	// determine what needs to be sent for one device and transmit
  void flushBufferAll();			// determine what needs to be sent for all devices and transmit

  uint8_t bitReverse(uint8_t b);	// reverse the order of bits in the byte
  bool transformBuffer(uint8_t buf, transformType_t ttype);	// internal transform function

};

#endif