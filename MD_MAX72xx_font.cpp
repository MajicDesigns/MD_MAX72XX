/*
MD_MAX72xx - Library for using a MAX7219/7221 LED matrix controller
  
See header file for comments

This file contains methods that work with the fonts and characters defined in the library
  
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
#include <avr/io.h> 
#include <avr/pgmspace.h>  
#include "MD_MAX72xx.h"
#include "MD_MAX72xx_lib.h"

/**
 * \file
 * \brief Implements font definition and methods
 */

#if USE_LOCAL_FONT
// Local font handling functions if the option is enabled

void MD_MAX72XX::buildFontIndex(void)
{
  uint16_t	offset = 0;

  if (_fontIndex == NULL)
    return;

  PRINTS("\nBuilding font index");
  for (int16_t i=0; i<FONT_INDEX_SIZE; i++)
  {
	_fontIndex[i] = offset;
	PRINT("\nASCII '", i);
	PRINT("' offset ", _fontIndex[i]);
	offset += pgm_read_byte(_fontData+offset);
	offset++;
  }
}

uint16_t MD_MAX72XX::getFontCharOffset(uint8_t c)
{
  PRINT("\nfontOffset ASCII ", c);

  if (_fontIndex != NULL)
  {
    PRINTS("' from Table");
    return(_fontIndex[c]);
  }
  else
  {
	uint16_t	offset = 0;

	for (int8_t i=0; i<c; i++)
	{
	  offset += pgm_read_byte(_fontData+offset);
	  offset++;	// skip size byte we used above
	}
    PRINT("' searched offset ", offset);

	return(offset);
  }
}

bool MD_MAX72XX::setFont(fontType_t f)
{
	_fontData = (f == NULL ? _sysfont_var : f);

  buildFontIndex();

  return(true);
}

uint8_t MD_MAX72XX::getChar(uint8_t c, uint8_t size, uint8_t *buf)
{
  PRINT("\ngetChar: '", (char)c);
  PRINT("' bufsize ", size);

  if (buf == NULL) 
    return(0);

  uint16_t offset = getFontCharOffset(c);
  size = min(size, pgm_read_byte(_fontData+offset));

  offset++;	// skip the size byte  

  for (uint8_t i=0; i<size; i++) 
    *buf++ = pgm_read_byte(_fontData+offset+i);

  return(size);
}

uint8_t MD_MAX72XX::setChar(uint16_t col, uint8_t c)
{
  PRINT("\nsetChar: '", c);
  PRINT("' column ", col);
  boolean	b = _updateEnabled;

  uint16_t offset = getFontCharOffset(c);
  uint8_t size = pgm_read_byte(_fontData+offset);

  offset++;	// skip the size byte  

  _updateEnabled = false;
  for (int8_t i=0; i<size; i++) 
  {
    uint8_t colData = pgm_read_byte(_fontData+offset+i);
	setColumn(col--, colData);
  }
  _updateEnabled = b;

  if (_updateEnabled) flushBufferAll();

  return(size);
}

// Standard font - variable spacing
uint8_t _sysfont_var[] PROGMEM = 
{
	0,							// 0 - 'Empty Cell'
	5, 62, 91, 79, 91, 62,		// 1 - 'Sad Smiley'
	5, 62, 107, 79, 107, 62,	// 2 - 'Happy Smiley'
	5, 28, 62, 124, 62, 28,		// 3 - 'Heart'
	5, 24, 60, 126, 60, 24,		// 4 - 'Diamond'
	5, 28, 87, 125, 87, 28,		// 5 - 'Clubs'
	5, 28, 94, 127, 94, 28,		// 6 - 'Spades'
	4, 0, 24, 60, 24,			// 7 - 'Bullet Point'
	0,		// 8
	0,		// 9
	0,		// 10
	5, 48, 72, 58, 6, 14,		// 11 - 'Male'
	5, 38, 41, 121, 41, 38,		// 12 - 'Female'
	5, 64, 127, 5, 5, 7,		// 13 - 'Music Note 1'
	5, 64, 127, 5, 37, 63,		// 14 - 'Music Note 2'
	5, 90, 60, 231, 60, 90,		// 15 - 'Snowflake'
	5, 127, 62, 28, 28, 8,		// 16 - 'Right Pointer'
	5, 8, 28, 28, 62, 127,		// 17 - 'Left Pointer'
	5, 20, 34, 127, 34, 20,		// 18 - 'UpDown Arrows'
	0,		// 19
	0,		// 20
	0,		// 21
	0,		// 22
	0,		// 23
	5, 8, 4, 126, 4, 8,			// 24 - 'Up Arrow'
	5, 16, 32, 126, 32, 16,		// 25 - 'Down Arrow'
	5, 8, 8, 42, 28, 8,			// 26 - 'Right Arrow'
	5, 8, 28, 42, 8, 8,			// 27 - 'Left Arrow'
	0,		// 28
	0,		// 29
	5, 48, 56, 62, 56, 48,		// 30 - 'Up Pointer'
	5, 6, 14, 62, 14, 6,		// 31 - 'Down Pointer'
	2, 0, 0,					// 32 - 'Space'
	1, 95,						// 33 - '!'
	3, 7, 0, 7,					// 34 - '"'
	5, 20, 127, 20, 127, 20,	// 35 - '#'
	5, 36, 42, 127, 42, 18,		// 36 - '$'
	5, 35, 19, 8, 100, 98,		// 37 - '%'
	5, 54, 73, 85, 34, 80,		// 38 - '&'
	3, 8, 7, 3,					// 39 - '''
	3, 28, 34, 65,				// 40 - '('
	3, 65, 34, 28,				// 41 - ')'
	5, 42, 28, 127, 28, 42,		// 42 - '*'
	5, 8, 8, 62, 8, 8,			// 43 - '+'
	3, 128, 112, 48,			// 44 - ','
	5, 8, 8, 8, 8, 8,			// 45 - '-'
	2, 96, 96,					// 46 - '.'
	5, 32, 16, 8, 4, 2,			// 47 - '/'
	5, 62, 81, 73, 69, 62,		// 48 - '0'
	3, 66, 127, 64,				// 49 - '1'
	5, 114, 73, 73, 73, 70,		// 50 - '2'
	5, 33, 65, 73, 77, 51,		// 51 - '3'
	5, 24, 20, 18, 127, 16,		// 52 - '4'
	5, 39, 69, 69, 69, 57,		// 53 - '5'
	5, 60, 74, 73, 73, 49,		// 54 - '6'
	5, 65, 33, 17, 9, 7,		// 55 - '7'
	5, 54, 73, 73, 73, 54,		// 56 - '8'
	5, 70, 73, 73, 41, 30,		// 57 - '9'
	1, 20,						// 58 - ':'
	2, 128, 104,				// 59 - ';'
	4, 8, 20, 34, 65,			// 60 - '<'
	5, 20, 20, 20, 20, 20,		// 61 - '='
	4, 65, 34, 20, 8,			// 62 - '>'
	5, 2, 1, 89, 9, 6,			// 63 - '?'
	5, 62, 65, 93, 89, 78,		// 64 - '@'
	5, 124, 18, 17, 18, 124,	// 65 - 'A'
	5, 127, 73, 73, 73, 54,		// 66 - 'B'
	5, 62, 65, 65, 65, 34,		// 67 - 'C'
	5, 127, 65, 65, 65, 62,		// 68 - 'D'
	5, 127, 73, 73, 73, 65,		// 69 - 'E'
	5, 127, 9, 9, 9, 1,			// 70 - 'F'
	5, 62, 65, 65, 81, 115,		// 71 - 'G'
	5, 127, 8, 8, 8, 127,		// 72 - 'H'
	3, 65, 127, 65,				// 73 - 'I'
	5, 32, 64, 65, 63, 1,		// 74 - 'J'
	5, 127, 8, 20, 34, 65,		// 75 - 'K'
	5, 127, 64, 64, 64, 64,		// 76 - 'L'
	5, 127, 2, 28, 2, 127,		// 77 - 'M'
	5, 127, 4, 8, 16, 127,		// 78 - 'N'
	5, 62, 65, 65, 65, 62,		// 79 - 'O'
	5, 127, 9, 9, 9, 6,			// 80 - 'P'
	5, 62, 65, 81, 33, 94,		// 81 - 'Q'
	5, 127, 9, 25, 41, 70,		// 82 - 'R'
	5, 38, 73, 73, 73, 50,		// 83 - 'S'
	5, 3, 1, 127, 1, 3,			// 84 - 'T'
	5, 63, 64, 64, 64, 63,		// 85 - 'U'
	5, 31, 32, 64, 32, 31,		// 86 - 'V'
	5, 63, 64, 56, 64, 63,		// 87 - 'W'
	5, 99, 20, 8, 20, 99,		// 88 - 'X'
	5, 3, 4, 120, 4, 3,			// 89 - 'Y'
	5, 97, 89, 73, 77, 67,		// 90 - 'Z'
	3, 127, 65, 65,				// 91 - '['
	5, 2, 4, 8, 16, 32,			// 92 - '\'
	3, 65, 65, 127,				// 93 - ']'
	5, 4, 2, 1, 2, 4,			// 94 - '^'
	5, 64, 64, 64, 64, 64,		// 95 - '_'
	3, 3, 7, 8,					// 96 - '`'
	5, 32, 84, 84, 120, 64,		// 97 - 'a'
	5, 127, 40, 68, 68, 56,		// 98 - 'b'
	5, 56, 68, 68, 68, 40,		// 99 - 'c'
	5, 56, 68, 68, 40, 127,		// 100 - 'd'
	5, 56, 84, 84, 84, 24,		// 101 - 'e'
	4, 8, 126, 9, 2,			// 102 - 'f'
	5, 24, 164, 164, 156, 120,	// 103 - 'g'
	5, 127, 8, 4, 4, 120,		// 104 - 'h'
	3, 68, 125, 64,				// 105 - 'i'
	4, 64, 128, 128, 122,		// 106 - 'j'
	4, 127, 16, 40, 68,			// 107 - 'k'
	3, 65, 127, 64,				// 108 - 'l'
	5, 124, 4, 120, 4, 120,		// 109 - 'm'
	5, 124, 8, 4, 4, 120,		// 110 - 'n'
	5, 56, 68, 68, 68, 56,		// 111 - 'o'
	5, 252, 24, 36, 36, 24,		// 112 - 'p'
	5, 24, 36, 36, 24, 252,		// 113 - 'q'
	5, 124, 8, 4, 4, 8,			// 114 - 'r'
	5, 72, 84, 84, 84, 36,		// 115 - 's'
	4, 4, 63, 68, 36,			// 116 - 't'
	5, 60, 64, 64, 32, 124,		// 117 - 'u'
	5, 28, 32, 64, 32, 28,		// 118 - 'v'
	5, 60, 64, 48, 64, 60,		// 119 - 'w'
	5, 68, 40, 16, 40, 68,		// 120 - 'x'
	5, 76, 144, 144, 144, 124,	// 121 - 'y'
	5, 68, 100, 84, 76, 68,		// 122 - 'z'
	3, 8, 54, 65,				// 123 - '{'
	1, 119,						// 124 - '|'
	3, 65, 54, 8,				// 125 - '}'
	5, 2, 1, 2, 4, 2,			// 126 - '~'
	5, 60, 38, 35, 38, 60,		// 127 - 'Hollow Up Arrow'
	5, 30, 161, 161, 97, 18,	// 128 - 'C sedilla'
	5, 56, 66, 64, 34, 120,		// 129 - 'u umlaut'
	5, 56, 84, 84, 85, 89,		// 130 - 'e acute'
	5, 33, 85, 85, 121, 65,		// 131 - 'a accent'
	5, 33, 84, 84, 120, 65,		// 132 - 'a umlaut'
	5, 33, 85, 84, 120, 64,		// 133 - 'a grave'
	5, 32, 84, 85, 121, 64,		// 134 - 'a acute'
	5, 24, 60, 164, 228, 36,	// 135 - 'c sedilla'
	5, 57, 85, 85, 85, 89,		// 136 - 'e accent'
	5, 56, 85, 84, 85, 88,		// 137 - 'e umlaut'
	5, 57, 85, 84, 84, 88,		// 138 - 'e grave'
	3, 69, 124, 65,				// 139 - 'i umlaut'
	4, 2, 69, 125, 66,			// 140 - 'i hat'
	4, 1, 69, 124, 64,			// 141 - 'i grave'
	5, 240, 41, 36, 41, 240,	// 142 - 'A umlaut'
	5, 240, 40, 37, 40, 240,	// 143 - 'A dot'
	4, 124, 84, 85, 69,			// 144 - 'E grave'
	7, 32, 84, 84, 124, 84, 84, 8,	// 145 - 'ae'
	6, 124, 10, 9, 127, 73, 73,	// 146 - 'AE'
	5, 50, 73, 73, 73, 50,		// 147 - 'o hat'
	5, 48, 74, 72, 74, 48,		// 148 - 'o umlaut'
	5, 50, 74, 72, 72, 48,		// 149 - 'o grave'
	5, 58, 65, 65, 33, 122,		// 150 - 'u hat'
	5, 58, 66, 64, 32, 120,		// 151 - 'u grave'
	4, 157, 160, 160, 125,		// 152 - 'y umlaut'
	5, 56, 69, 68, 69, 56,		// 153 - 'O umlaut'
	5, 60, 65, 64, 65, 60,		// 154 - 'U umlaut'
	5, 60, 36, 255, 36, 36,		// 155 - 'Cents'
	5, 72, 126, 73, 67, 102,	// 156 - 'Pounds'
	5, 43, 47, 252, 47, 43,		// 157 - 'Yen'
	0,		// 158
	0,		// 159
	5, 32, 84, 84, 121, 65,		// 160 - 'a acute'
	3, 68, 125, 65,				// 161 - 'i acute'
	5, 48, 72, 72, 74, 50,		// 162 - 'o acute'
	5, 56, 64, 64, 34, 122,		// 163 - 'u acute'
	4, 122, 10, 10, 114,		// 164 - 'n accent'
	5, 125, 13, 25, 49, 125,	// 165 - 'N accent'
	0,		// 166
	0,		// 167
	5, 48, 72, 77, 64, 32,		// 168 - 'Inverted ?'
	5, 56, 8, 8, 8, 8,			// 169
	5, 8, 8, 8, 8, 56,			// 170
	5, 47, 16, 200, 172, 186,	// 171 - '1/2'
	5, 47, 16, 40, 52, 250,		// 172 - '1/4'
	1, 123,						// 173 - '| split'
	5, 8, 20, 42, 20, 34,		// 174 - '<<'
	5, 34, 20, 42, 20, 8,		// 175 - '>>'
	5, 170, 0, 85, 0, 170,		// 176 - '30% shading'
	5, 170, 85, 170, 85, 170,	// 177 - '50% shading'
	0,		// 178
	0,		// 179
	0,		// 180
	0,		// 181
	0,		// 182
	0,		// 183
	0,		// 184
	0,		// 185
	0,		// 186
	0,		// 187
	0,		// 188
	0,		// 189
	0,		// 190
	0,		// 191
	0,		// 192
	0,		// 193
	0,		// 194
	0,		// 195
	0,		// 196
	0,		// 197
	0,		// 198
	0,		// 199
	0,		// 200
	0,		// 201
	0,		// 202
	0,		// 203
	0,		// 204
	0,		// 205
	0,		// 206
	0,		// 207
	0,		// 208
	0,		// 209
	0,		// 210
	0,		// 211
	0,		// 212
	0,		// 213
	0,		// 214
	0,		// 215
	0,		// 216
	0,		// 217
	5, 255, 255, 255, 255, 255,	// 218 - 'Full Block'
	5, 240, 240, 240, 240, 240,	// 219 - 'Half Block Bottom'
	3, 255, 255, 255,			// 220 - 'Half Block LHS'
	5, 0, 0, 0, 255, 255,		// 221 - 'Half Block RHS'
	5, 15, 15, 15, 15, 15,		// 222 - 'Half Block Top'
	5, 56, 68, 68, 56, 68,		// 223 - 'Alpha'
	5, 124, 42, 42, 62, 20,		// 224 - 'Beta'
	5, 126, 2, 2, 6, 6,			// 225 - 'Gamma'
	5, 2, 126, 2, 126, 2,		// 226 - 'Pi'
	5, 99, 85, 73, 65, 99,		// 227 - 'Sigma'
	5, 56, 68, 68, 60, 4,		// 228 - 'Theta'
	5, 64, 126, 32, 30, 32,		// 229 - 'mu'
	5, 6, 2, 126, 2, 2,			// 230 - 'Tau'
	5, 153, 165, 231, 165, 153,	// 231
	5, 28, 42, 73, 42, 28,		// 232
	5, 76, 114, 1, 114, 76,		// 233
	5, 48, 74, 77, 77, 48,		// 234
	5, 48, 72, 120, 72, 48,		// 235
	5, 188, 98, 90, 70, 61,		// 236 - 'Zero Slashed'
	4, 62, 73, 73, 73,			// 237
	5, 126, 1, 1, 1, 126,		// 238
	4, 42, 42, 42, 42,			// 239 - '3 Bar Equals'
	3, 36, 46, 36,				// 240 - '+/-'
	5, 64, 81, 74, 68, 64,		// 241 - '>='
	5, 64, 68, 74, 81, 64,		// 242 - '<='
	0,		// 243
	0,		// 244
	6, 8, 8, 107, 107, 8, 8,	// 245 - 'Divide'
	6, 36, 18, 18, 36, 36, 18,	// 246 - 'Wavy ='
	5, 6, 15, 9, 15, 6,			// 247 - 'Degree'
	2, 24, 24,					// 248 - 'Math Product'
	4, 0, 0, 16, 16,			// 249 - 'Short Dash'
	5, 48, 64, 255, 1, 1,		// 250 - 'Square Root'
	4, 31, 1, 1, 30,			// 251 - 'Superscript n'
	4, 25, 21, 21, 18,			// 252 - 'Superscript 2'
	5, 0, 60, 60, 60, 60,		// 253 - 'Centered Square'
	5, 255, 129, 129, 129, 255,	// 254 - 'Full Frame'
	5, 255, 255, 255, 255, 255,	// 255 - 'Full Block'
};
#endif //INCLUDE_LOCAL_FONT

