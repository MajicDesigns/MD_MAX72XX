/*
MD_MAX72xx - Library for using a MAX7219/7221 LED matrix controller

See header file for comments

This file contains library related definitions and is not visible
to user code.

****************************************************************
* PLEASE MAKE ALL ADJUSTMENTS FOR THE HARDWARE USING THE MAIN  *
* LIBRARY HEADER FILE. THIS FILE SHOULD NOT BE CHANGED.        *
****************************************************************

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

#define MAX_DEBUG 0   ///< Enable or disable (default) debugging output from the MD_MAX72xx library

#if MAX_DEBUG
#define PRINT(s, v)   { Serial.print(F(s)); Serial.print(v); }      ///< Print a string followed by a value (decimal)
#define PRINTX(s, v)  { Serial.print(F(s)); Serial.print(v, HEX); } ///< Print a string followed by a value (hex)
#define PRINTB(s, v)  { Serial.print(F(s)); Serial.print(v, BIN); } ///< Print a string followed by a value (binary)
#define PRINTS(s)     { Serial.print(F(s)); }                       ///< Print a string
#else
#define PRINT(s, v)   ///< Print a string followed by a value (decimal)
#define PRINTX(s, v)  ///< Print a string followed by a value (hex)
#define PRINTB(s, v)  ///< Print a string followed by a value (binary)
#define PRINTS(s)     ///< Print a string
#endif

// Opcodes for the MAX7221 and MAX7219
// All OP_DIGITn are offsets from OP_DIGIT0
#define OP_NOOP       0 ///< MAX72xx opcode for NO OP
#define OP_DIGIT0     1 ///< MAX72xx opcode for DIGIT0
#define OP_DIGIT1     2 ///< MAX72xx opcode for DIGIT1
#define OP_DIGIT2     3 ///< MAX72xx opcode for DIGIT2
#define OP_DIGIT3     4 ///< MAX72xx opcode for DIGIT3
#define OP_DIGIT4     5 ///< MAX72xx opcode for DIGIT4
#define OP_DIGIT5     6 ///< MAX72xx opcode for DIGIT5
#define OP_DIGIT6     7 ///< MAX72xx opcode for DIGIT6
#define OP_DIGIT7     8 ///< MAX72xx opcode for DIGIT7
#define OP_DECODEMODE  9  ///< MAX72xx opcode for DECODE MODE
#define OP_INTENSITY   10 ///< MAX72xx opcode for SET INTENSITY
#define OP_SCANLIMIT   11 ///< MAX72xx opcode for SCAN LIMIT
#define OP_SHUTDOWN    12 ///< MAX72xx opcode for SHUT DOWN
#define OP_DISPLAYTEST 15 ///< MAX72xx opcode for DISPLAY TEST

#define ALL_CHANGED   0xff    ///< Mask for all rows changed in a buffer structure
#define ALL_CLEAR     0x00    ///< Mask for all rows clear in a buffer structure

#define ASCII_INDEX_SIZE  256 ///< Number of characters in a font table (ASCII maximum)

// Shortcuts
#define SPI_DATA_SIZE (sizeof(uint8_t)*_maxDevices*2)   ///< Size of the SPI data buffers
#define SPI_OFFSET(i,x) (((LAST_BUFFER-(i))*2)+(x))     ///< SPI data offset for buffer i, digit x

#define FIRST_BUFFER 0                 ///< First buffer number
#define LAST_BUFFER  (_maxDevices-1)   ///< Last buffer number

// variables shared in the library
extern const uint8_t PROGMEM _sysfont_var[];  ///< System variable pitch font table

/**
\page pageHardware Hardware
Supported Hardware
------------------
This library supports the Parola hardware and the more commonly available LED modules available
from many other sources. The circuits for these modules are essentially identical except
in the way that the LED matrix is wired to the MAX7219 IC. This difference is accounted for in
software when the type of module is selected using the appropriate USE_*_HW compile time switch.

Hardware supported
------------------
- \subpage pageParola
- \subpage pageGeneric
- \subpage pageICStation
- \subpage pageFC16
- \subpage pageNewHardware

Connecting Multiple Modules
---------------------------
Separate modules are connected by the plugging them together edge to edge, with the
OUT side of one module plugged to the IN side of the next. More details can be found
at the end of each module's hardware section.
___

\page pageParola Parola Custom Module
The Parola Module
-----------------
These custom modules allow a 'lego-like' approach to LED matrix display, using standard
8x8 on LED matrices. The software supports this flexibility through a scalable approach 
that only requires the definition of the number of modules to adapt existing software to
a new configuration.

![Completed Parola module] (Parola_Module.png "Parola LED Matrix Modules")

Circuit Schematic
-----------------
The schematic is the basic application circuit that is found on the MAX7219 datasheet,
adapted to the LED matrix. Each Module consists of an 8x8 LED matrix controlled by a
MAX7219 LED controller and a few passive components. These controllers can be daisy
chained, making them ideal for the purpose.

![Parola Circuit Schematic] (Circuit_Schematic.jpg "Parola Schematic")

The PCB design was executed using the auto routing facility in Eagle CAD, and the PCB was
manufactured by SeeedStudio. The Eagle CAD files for the layout and the Gerber files
suitable for SeeedStudio are found on the [Parola website] (https://github.com/MajicDesigns/MD_Parola).
The final design includes edge connections that allow many modules to be connected
together into an extended display, one LED module high.

![PCB layout ready for manufacture] (PCB_Layout.jpg "PCB Design")

Wiring your own Parola standard matrix
--------------------------------------
How the LED matrix is wired is important for the library. The matrix used for library
development was labeled 1088B and is sometime referred to as a **common anode** matrix.
Connections should be made as described in the table below to be consistent with the
assumptions in the software library.
- Columns are addressed through the segment selection lines
- Rows are addressed through the digit selection lines

MAX Signal|MAX7219 Pin|MAX Signal|MAX7219 Pin|
:--------:|----------:|:--------:|----------:|
Dig0 (D0) |2          |SegDP     |22         |
Dig1 (D1) |11         |SegA      |14         |
Dig2 (D2) |6          |SegB      |16         |
Dig3 (D3) |7          |SegC      |20         |
Dig4 (D4) |3          |SegD      |23         |
Dig5 (D5) |10         |SegE      |21         |
Dig6 (D6) |5          |SegF      |15         |
Dig7 (D7) |8          |SegG      |17         |

Segment data is packed on a per-digit basis, with segment G as the least significant bit (bit 0)
through to A as bit 6 and DP as bit 7.
____

Module Orientation
------------------

      G  F  E  D  C  B  A  DP
    +------------------------+
    | 7  6  5  4  3  2  1  0 | DIG0
    |                      1 | DIG1
    |                      2 | DIG2
    |                      3 | DIG3
    | O                    4 | DIG4
    | O  O                 5 | DIG5
    | O  O  O              6 | DIG6
    | O  O  O  O           7 | DIG7
    +------------------------+
      Vcc ----      ---- Vcc
     DOUT <---      ---< DIN
      GND ----      ---- GND
    CS/LD <---      ---< CS/LD
      CLK <---      ---< CLK

____

Module Interconnections
-----------------------
Parola modules are connected by plugging them together.
![Connecting Parola modules] (Modules_conn.jpg "Parola Modules connected")
____

\page pageGeneric Generic Module
Generic MAX7219 Module
------------------------
These modules are commonly available from many suppliers (eg, eBay) at reasonable cost.
They are characterized by IN and OUT connectors at the short ends of the rectangular PCB.

![Generic Module] (Generic_Module.png "Generic Module")
____

Module Orientation
------------------

          C  C  D  G  V
          L  S  I  N  c
          K     N  D  c
          |  |  |  |  |
          V  V  V  |  |
      D7 D6 D5 D4 D3 D2 D1 D0
    +------------------------+
    | 7  6  5  4  3  2  1  0 |- DP
    |                      1 |- A
    |                      2 |- B
    |                      3 |- C
    | O                    4 |- D
    | O  O                 5 |- E
    | O  O  O              6 |- F
    | O  O  O  O           7 |- G
    +-----+--+--+--+--+------+
          |  |  |  |  |
          V  V  V  |  |

          C  C  D  G  V
          L  S  O  N  c
          K     U  D  c
                T

____

Module Interconnections
-----------------------
Generic modules need to be oriented with the MAX7219 IC at the top and connected using
short patch cables in a spiral pattern. The display is oriented with the IC at the top.
![Connecting Generic modules] (Generic_conn.jpg "Generic Modules connected")
____

\page pageICStation ICStation Module
ICStation DIY Kit Module
------------------------
These modules are available as kits from ICStation (http://www.icstation.com/product_info.php?products_id=2609#.UxqVJyxWGHs).

![ICStation Module] (ICStation_Module.jpg "ICStation Module")
____

Module Orientation
------------------

               G  F  E  D  C  B  A  DP
             +------------------------+
             | 7  6  5  4  3  2  1  0 | D7
     CLK <---|                      1 | D6 <--- CLK
      CS <---|                      2 | D5 <--- CS
    DOUT <---|                      3 | D4 <--- DIN
     GND ----| O                    4 | D3 ---- GND
     VCC ----| O  O                 5 | D2 ---- VCC
             | O  O  O              6 | D1
             | O  O  O  O           7 | D0
             +------------------------+
____

Module Interconnections
-----------------------
ICStation Modules are connected using the links supplied with the hardware. The display is
oriented with the DIN side on the right.

![Connecting ICStation modules] (ICStation_conn.jpg "ICStation Modules connected")

____
\page pageFC16 FC-16 Module
FC-16 DIY Kit Module
----------------------
These modules are available as kits from some internet suppliers such as G&C Supermarket on eBay
(http://stores.ebay.com.au/gcsupermarkethkcoltd/). They are identifiable by the FC-16 designation
silk screened on the PCB. Most of the available sets of 4 modules connected as one unit are 
FC-16 type.

![FC-16 Module] (FC-16_Module.jpg "FC-16 Module")
____

Module Orientation
------------------

               DP A  B  C  D  E  F  G
             +------------------------+
             | 7  6  5  4  3  2  1  0 | D0
     CLK <---|                      1 | D1 <--- CLK
      CS <---|                      2 | D2 <--- CS
    DOUT <---|                      3 | D3 <--- DIN
     GND ----| O                    4 | D4 ---- GND
     VCC ----| O  O                 5 | D5 ---- VCC
             | O  O  O              6 | D6
             | O  O  O  O           7 | D7
             +------------------------+
____

Module Interconnections
-----------------------
FC-16 Modules are connected using the links supplied with the hardware. The display is
oriented with the DIN side on the right. PCB text may appear upside down.

![Connecting FC-16 modules] (FC-16_conn.jpg "FC-16 Modules connected")

____
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

As there is no standard way of wiring a LED matrix to the MAX72xx IC, each hardware type
definition activates a series of coordinate mapping transformations. Possible changes
are limited to combinations (8 in total) of
- swapping rows and column coordinates (digits and segments in MAX72xx),
- a reversal of row indices, and
- a reversal of column indices.

The hardware types defined in MD_MAX72xx.h activate different library code by defining
appropriate values for the defines listed below, in the MD_MAX72xx_lib.h file.

- HW_DIG_ROWS - MAX72xx digits are mapped to rows in on the matrix. If digits are
not rows then they are columns!

- HW_REV_COLS - Normal column coordinates orientation is 0 col on the right side
of the display. Set to 1 to reverse this (0 on the left).

- HW_REV_ROWS - Normal row coordinates orientation is 0 row at top of the display.
Set to 1 to reverse this (0 at the bottom).

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
___

\page pageFontUtility Create and Modify Fonts

Font Storage Format
-------------------
One default font is defined as part of the library in PROGMEM memory. Alternative fonts
can be specified to the library. The font builder utilities provide a convenient way to
modify existing or develop alternative fonts.

Fonts are stored as a series of contiguous bytes in the following format:
- byte 1 - the number of bytes that form this character (could be zero)
- byte 2..n - each byte is a column of the character to be formed, starting with the
leftmost column of the character. The least significant bit of the byte is the bottom
pixel position of the character matrix (row 7).

To find a character in the font table, the library looks at the first byte (size),
skips 'size'+1 bytes to the next character size byte and repeat until the last or
target character is reached.

The compile-time switch USE_INDEX_FONT enables indexing of the font table for faster access, at
the expense of increased RAM usage. If indexing is enabled, a single lookup is required to
access the character data, rather than the sequential search described above.

The support for fonts (methods and data) may be completely disabled  if not required through
the compile-time switch USE_LOCAL_FONT. This will also disable user defined fonts.

____

The txt2font Utility
--------------------
The txt2font utility is a command line application that converts a text definition of the font
into a data file in the right format for MD_MAX72xx to use.

This utility is as an Win32 executable. Users with other Operating Systems will need to compile
a version to work with their OS, using the source code supplied.

The application is invoked from the command line and only the root name of the file is given as a command
line parameter (eg "txt2font fred"). The application will look for and input file with a '.txt' extension
(fred.txt) and produce an output file with a '.h' extension (fred.h).

The txt2font file format is line based. Lines starting with a '.' are directives for the application, all
other lines are data for the current character definition. An example of the beginning of a font
definition file is shown below.

      .NAME sys_var_single
      .HEIGHT 1
      .WIDTH 0
      .CHAR 0
      .NOTE  'Empty Cell'
      .CHAR 1
      .NOTE  'Sad Smiley'
       @@@
      @@@@@
      @ @ @
      @@@@@
      @@ @@
      @   @
       @@@
      .CHAR 2
      .NOTE  'Happy Smiley'
       ***

The directives have the following meaning:
- .NAME defines the name for the font and is used in naming the font table variable.
The name can appear anywhere in the file. If omitted, a default name is used.
- .HEIGHT defines the height for the font. Single height fonts are '1' and double height fonts are '2'.
If double height fonts are specified then the range of ASCII character values is restricted to 0..127 as
the top and bottom halves of the font are stored offset by 128 positions. If omitted, the application
assumes single height font.
- .WIDTH specifies the width of the font for all the characters defined between this WIDTH and the
next WIDTH definition. 0 means variable width; any other number defines the fixed width. WIDTH may be changed
within the file - for example to define a fixed size space (no pixels!) character in a variable width font.
- .CHAR ends the definition of the current character and starts the definition for the specified ASCII value.
Valid parameters are [0..255] for single height, and [0..127] for double height. If a character code is
omitted in the font definition file it is assumed to be empty.
- .NOTE is an option note that will be added as a comment for the entry in the font data table.

Any lines not starting with a '.' are data lines for the current character. The font characters are drawn
using a non-space character (eg, '*' or '@') wherever a LED needs to be 'on'. The application scans from
the top down, so any lines missing at the bottom of the character definition are assumed to be blank.
However, blank lines at the top need to be shown. Any extra rows are ignored will cause program errors.

A number of font definition files are supplied as examples.
___

The FontBuilder Excel/VBA application
-------------------------------------
FontBuilder is a Microsoft Excel spreadsheet with VBA macros to manage a GUI interface for defining
and managing font characters. FontBuilder supports both single and double height fonts. The first tab
in the FontBuilder spreadsheet has instructions for use.

As FontBuilder requires using Microsoft Office products, it does not work environments where
these are not available.

*/

// *******************************************************************************************
// ** Combinations not listed here have probably not been tested and may not work correctly **
// **                                                                                       **
// ** NOTE: in nearly all cases, the user should not be changing these combinations but     **
// **       modifying the hardware type definitions in the MD_MAX72xx.h library header file.**
// *******************************************************************************************
#if USE_PAROLA_HW   // tested MC 8 March 2014
//#pragma message "PAROLA HW selected"
#define HW_DIG_ROWS 1 ///< MAX72xx digits are mapped to rows in on the matrix
#define HW_REV_COLS 1 ///< Normal orientation is col 0 on the right. Set to 1 if reversed
#define HW_REV_ROWS 0 ///< Normal orientation is row 0 at the top. Set to 1 if reversed
#endif

#if USE_GENERIC_HW  // tested MC 9 March 2014
//#pragma message "GENERIC HW selected"
#define HW_DIG_ROWS 0 ///< MAX72xx digits are mapped to rows in on the matrix
#define HW_REV_COLS 1 ///< Normal orientation is col 0 on the right. Set to 1 if reversed
#define HW_REV_ROWS 0 ///< Normal orientation is row 0 at the top. Set to 1 if reversed
#endif

#if USE_ICSTATION_HW  // tested MC 9 March 2014
//#pragma message "ICSTATION HW selected"
#define HW_DIG_ROWS 1 ///< MAX72xx digits are mapped to rows in on the matrix
#define HW_REV_COLS 1 ///< Normal orientation is col 0 on the right. Set to 1 if reversed
#define HW_REV_ROWS 1 ///< Normal orientation is row 0 at the top. Set to 1 if reversed
#endif

#if USE_FC16_HW       // tested MC 23 Feb 2015
//#pragma message "FC16 HW selected"
#define HW_DIG_ROWS 1 ///< MAX72xx digits are mapped to rows in on the matrix
#define HW_REV_COLS 0 ///< Normal orientation is col 0 on the right. Set to 1 if reversed
#define HW_REV_ROWS 0 ///< Normal orientation is row 0 at the top. Set to 1 if reversed
#endif

#if USE_OTHER_HW      // user defined custom hardware configuration
//#pragma message "OTHER HW selected"
#define HW_DIG_ROWS 0 ///< MAX72xx digits are mapped to rows in on the matrix
#define HW_REV_COLS 0 ///< Normal orientation is col 0 on the right. Set to 1 if reversed
#define HW_REV_ROWS 0 ///< Normal orientation is row 0 at the top. Set to 1 if reversed
#endif

#ifndef HW_DIG_ROWS
#error "INVALID or missing hardware selected"
#endif

// Macros to map ROW and COLUMN coordinates
#if HW_REV_ROWS
#define HW_ROW(r) (7-r) ///< Pixel to hardware coordinate row mapping
#else
#define HW_ROW(r) (r)   ///< Pixel to hardware coordinate row mapping
#endif

#if HW_REV_COLS
#define HW_COL(c) (7-c) ///< Pixel to hardware coordinate column mapping
#else
#define HW_COL(c) (c)   ///< Pixel to hardware coordinate column mapping
#endif

#endif
