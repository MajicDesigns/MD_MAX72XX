/*
MD_MAX72xx - Library for using a MAX7219/7221 LED matrix controller
  
See header file for comments

This file contains library related definitions and is not visible
to user code.
  
Copyright (C) 2012-14 Marco Colli. All rights reserved.

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
 */
#ifndef MDMAX72xxLIB_H
#define	MDMAX72xxLIB_H

/**
 * \file
 * \brief Includes library definitions
 */

#define	MAX_DEBUG	0		///< Enable or disable (default) debugging output from the MD_MAX72xx library

#if MAX_DEBUG
#define	PRINT(s, v)		{ Serial.print(F(s)); Serial.print(v); }		  ///< Print a string followed by a value (decimal)
#define	PRINTX(s, v)	{ Serial.print(F(s)); Serial.print(v, HEX); }	///< Print a string followed by a value (hex)
#define	PRINTB(s, v)	{ Serial.print(F(s)); Serial.print(v, BIN); }	///< Print a string followed by a value (binary)
#define	PRINTS(s)		  { Serial.print(F(s)); }							          ///< Print a string
#else
#define	PRINT(s, v)		///< Print a string followed by a value (decimal)
#define	PRINTX(s, v)	///< Print a string followed by a value (hex)
#define	PRINTB(s, v)	///< Print a string followed by a value (binary)
#define	PRINTS(s)		  ///< Print a string
#endif

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

#define ALL_CHANGED   0xff			///< Mask for all rows changed in a buffer structure
#define ALL_CLEAR     0x00			///< Mask for all rows clear in a buffer structure

#define	FONT_INDEX_SIZE	256			///< Number of characters in a font table (ASCII maximum)

// Shortcuts
#define	SPI_DATA_SIZE	(sizeof(uint8_t)*_maxDevices*2)	///< Size of the SPI data buffers
#define	SPI_OFFSET(i,x)	(((i)*2)+(x))			///< SPI data offset for buffer i, digit x
#define	FIRST_BUFFER	0						        ///< First buffer number
#define	LAST_BUFFER		(_maxDevices-1)			///< Last buffer number

// variables shared in the library
extern uint8_t _sysfont_var[];		///< System variable pitch font table

/**
\page pageNewHardware New Hardware Types
A word on coordinate systems
----------------------------

Two Cartesian coordinate systems are used in the library
- one defines the pixels seen (_display coordinates_), and 
- an underlying _hardware coordinate_ system based on digits and segments 
  mapping to the MAX72xx hardware control registers.

Display coordinates always have their origin in the top right corner of a display. 
- Column numbers increase to the left (as do module numbers)  
- Row numbers increase down (0..7)
All user functions are consistent and use display coordinates. 

Display memory buffers are stored in hardware coordinates that depend on 
the hardware configuration (i.e. the module type). It is the job of the low level 
library functions to map display to hardware coordinates. Digit 0 is the lowest 
row/column number and Segment G is the lowest column/row number.

All the code to do this is in the is in the buffer and pixel modules.
All other library modules are use the primitives made available in these modules.

What needs to change?
---------------------

As there is no standard way of wiring a LED matrix to the MAX72xx, each hardware type 
definition activates a series of coordinate mapping transformations. Possible changes 
are limited to combinations (8 in total) of
- swapping rows and column coordinates (digits and segments in MAX72xx),  
- a reversal row indices, and
- a reversal of column indices. 

The hardware types defined in MD_MAX72xx.h activate different library code by defining 
appropriate values for the defines listed below, in the MD_MAX72xx_lib.h file.

HW_DIG_ROWS - MAX72xx digits are mapped to rows in on the matrix. 
              If digits are not rows then they are columns!
HW_REV_COLS - Normal column coordinates orientation is 0 col is on the right side of the display.
              Set to 1 to reverse this (0 on left).
HW_REV_ROWS - Normal row coordinates orientation is 0 row is at top of the display. 
              Set to 1 to reverse this (0 at bottom).
              
Determining the type of mapping
-------------------------------
The library example code includes a utility called MD_MAX72xx_HW_Mapper. 
This is test software to map display hardware rows and columns. It uses a 
generic SPI interface and only one MAX72xx/8x8 LED module required. It is 
independent of the libraries as the code is used to directly map the display 
orientation by setting pixels on the display and printing to the serial monitor 
which MAX72xx hardware component (segment and digit) is being exercised. 

By observing the LED display and the serial monitor you can build a map like the 
one below. It is worth noting the direction in which the rows and columns are 
scanned by the utility, as this is the easiest way to work out the row/column 
reversal values.
    
The result of these observations is a grid definition that looks somewhat like:

          DIG0 D1 D2 D3 D4 D5 D6 D7
    Seg G
    Seg F
    Seg E
    Seg D
    Seg C
    Seg B
    Seg A
    Seg DP

From this mapping it is clear
- MAX72xx digits map to the columns, HW_DIG_ROWS is 0.
- DIG0 is on the left (columns were also scanned left to right), so HW_REV_COLS should be set 
  to 1 to reverse it to the standard 0 on the right.
- Seg G is at the top (rows were also top to bottom), so HW_REV_ROWS should be set to 0, 
  as it is already standard with 0 on top.
  
Note that in some situations using the module 'upside down' will result in a better configuration 
than would otherwise be the case. An example of this is the generic module mapping. Also remember 
that the modules are daisy chained from right to left.

Having determined the values for the defines, the new mapping can be configured, or matched to 
an existing hardware type.

*/

// *******************************************************************************************
// ** Combinations not listed here have probably not been tested and may not work correctly **
// *******************************************************************************************
#if USE_PAROLA_HW		// tested MC 8 March 2014
#define	HW_DIG_ROWS	1 ///< MAX72xx digits are mapped to rows in on the matrix
#define	HW_REV_COLS	1 ///< Normal orientation is col 0 on the right. Set to 1 to reverse this
#define	HW_REV_ROWS	0 ///< Normal orientation is row 0 at the top. Set to 1 to reverse this
#endif

#if USE_GENERIC_HW		// tested MC 9 March 2014
#define	HW_DIG_ROWS	0 ///< MAX72xx digits are mapped to rows in on the matrix
#define	HW_REV_COLS	1 ///< Normal orientation is col 0 on the right. Set to 1 to reverse this
#define	HW_REV_ROWS	0 ///< Normal orientation is row 0 at the top. Set to 1 to reverse this
#endif

#if USE_ICSTATION_HW	// tested MC 9 March 2014
#define	HW_DIG_ROWS	1 ///< MAX72xx digits are mapped to rows in on the matrix
#define	HW_REV_COLS	1 ///< Normal orientation is col 0 on the right. Set to 1 to reverse this
#define	HW_REV_ROWS	1 ///< Normal orientation is row 0 at the top. Set to 1 to reverse this
#endif

// Macros to map ROW and COLUMN coordinates
#if HW_REV_ROWS
#define	HW_ROW(r)	(7-r)	///< Pixel to hardware coordinate row mapping
#else
#define	HW_ROW(r)	(r)		///< Pixel to hardware coordinate row mapping
#endif

#if HW_REV_COLS
#define	HW_COL(c)	(7-c)	///< Pixel to hardware coordinate column mapping
#else
#define	HW_COL(c)	(c)		///< Pixel to hardware coordinate column mapping
#endif

#endif
