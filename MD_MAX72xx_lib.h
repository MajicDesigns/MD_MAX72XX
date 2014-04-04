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
#define	PRINT(s, v)		{ Serial.print(F(s)); Serial.print(v); }		///< Print a string followed by a value (decimal)
#define	PRINTX(s, v)	{ Serial.print(F(s)); Serial.print(v, HEX); }	///< Print a string followed by a value (hex)
#define	PRINTB(s, v)	{ Serial.print(F(s)); Serial.print(v, BIN); }	///< Print a string followed by a value (binary)
#define	PRINTS(s)		{ Serial.print(F(s)); }							///< Print a string
#else
#define	PRINT(s, v)		///< Print a string followed by a value (decimal)
#define	PRINTX(s, v)	///< Print a string followed by a value (hex)
#define	PRINTB(s, v)	///< Print a string followed by a value (binary)
#define	PRINTS(s)		///< Print a string
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
#define	SPI_OFFSET(i,x)	(((i)*2)+(x))					///< SPI data offset for buffer i, digit x
#define	FIRST_BUFFER	0						///< First buffer number
#define	LAST_BUFFER		(_maxDevices-1)			///< Last buffer number

// variables shared in the library
extern uint8_t _sysfont_var[];		///< System variable pitch font table

// Coordinate system adjustment for different hardware configurations
// 
// Each hardware type translates into a transformation of coordinates and potential 
// reversal of row and column indices. This creates 2 coordinates systems - a cartesian 
// coordinate system based on the pixels seen and an underlying hardware coordinate 
// system based on digits and segments.
// Pixel coordinate space has it's origin in the top right hand corner of a display. 
// - Columns numbers increase to the left (as do module numbers)  
// - Row numbers increase down (0..7)
//
// All user functions are called using pixel coordinates space and all hardware buffers 
// are stored in 'hardware ready' coordinates that depend on the hardware configuration. 
// It is the job of the low level library functions to map one to the other. 
//
// All the dependent code is in the buffer and pixel modules. All other modules 
// are written in terms of the primitives given in these affected modules.
//
// Library code sections will be activated depending on the defines below, to perform 
// the appropriate coordinate remapping and transformations.
// HW_DIG_ROWS - Max72xx digits are mapped to rows in on the matrix. 
//               If digits are not rows then they are columns!
// HW_REV_COLS - Max72xx column coordinates orientation is 0 col is on right
// HW_REV_COLS - Max72xx row coordinates orientation is 0 row is at top  
//
// Note: Combinations not listed here have probably not been tested and may not work correctly.
//
#if USE_PAROLA_HW		// tested MC 8 March 2014
#define	HW_DIG_ROWS	1
#define	HW_REV_ROWS	0
#define	HW_REV_COLS	1
#endif

#if USE_GENERIC_HW		// tested MC 9 March 2014
#define	HW_DIG_ROWS	0
#define	HW_REV_ROWS	0
#define	HW_REV_COLS	1
#endif

#if USE_ICSTATION_HW	// tested MC 9 March 2014
#define	HW_DIG_ROWS	1
#define	HW_REV_ROWS	1
#define	HW_REV_COLS	1
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
