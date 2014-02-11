/*
MD_MAX72xx - Library for using a MAX7219/7221 LED matrix controller
  
See header file for comments
This file contains methods that act on display buffers.
  
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
 */
#include <Arduino.h>
#include "MD_MAX72xx.h"
#include "MD_MAX72xx_lib.h"

/**
 * \file
 * \brief Implements buffer related methods
 */

bool MD_MAX72XX::clear(uint8_t buf) 
{
  if (buf > END_BUFFER)
    return(false);
    
  memset(_matrix[buf].row, 0, sizeof(_matrix[buf].row));
  _matrix[buf].changed = ALL_CHANGED;

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

#if USE_PAROLA_HW
uint8_t MD_MAX72XX::getColumn(uint8_t buf, uint8_t c)
#else
uint8_t MD_MAX72XX::getRow(uint8_t buf, uint8_t c)
#endif
{
  uint8_t mask = COL_ZERO_BIT;
  uint8_t value = 0;
  
#if USE_PAROLA_HW
  PRINT("\ngetCol: (", buf);
#else
  PRINT("\ngetRow: (", buf);
#endif
  PRINT(", ", c);
  PRINTS(") ");

#if USE_PAROLA_HW
  if ((buf > END_BUFFER) || (c >= COL_SIZE))
#else
  if ((buf > END_BUFFER) || (c >= ROW_SIZE))
#endif
  return(0);

  mask >>= c;
  PRINTX("mask 0x", mask);

  for (uint8_t i=0; i<ROW_SIZE; i++)
  {
      if (_matrix[buf].row[i] & mask)
        bitSet(value, 7-i);
  }

  PRINTX(" = 0x", value);
 
  return(bitReverse(value));
}

#if USE_PAROLA_HW
bool MD_MAX72XX::setColumn(uint8_t buf, uint8_t c, uint8_t value)
#else
bool MD_MAX72XX::setRow(uint8_t buf, uint8_t c, uint8_t value)
#endif
{
  uint8_t mask = COL_ZERO_BIT;

  value = bitReverse(value);

#if USE_PAROLA_HW
  PRINT("\nsetCol: (", buf);
#else
  PRINT("\nsetRow: (", buf);
#endif
  PRINT(", ", c);
  PRINTX(") 0x", value);

#if USE_PAROLA_HW
  if ((buf > END_BUFFER) || (c >= COL_SIZE))
#else
  if ((buf > END_BUFFER) || (c >= ROW_SIZE))
#endif
    return(false);
  
  for (uint8_t i=0; i<ROW_SIZE; i++)
  {
      if (value & mask)
        bitSet(_matrix[buf].row[i], 7-c);
      else
        bitClear(_matrix[buf].row[i], 7-c);
        
      mask >>= 1;
  }
  _matrix[buf].changed = ALL_CHANGED;
  
  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

#if USE_PAROLA_HW
uint8_t MD_MAX72XX::getRow(uint8_t buf, uint8_t r)
#else
uint8_t MD_MAX72XX::getColumn(uint8_t buf, uint8_t r)
#endif
{
#if USE_PAROLA_HW
  PRINT("\ngetRow: (", buf);
#else
  PRINT("\ngetCol: (", buf);
#endif
  PRINT(", ", r);
  PRINTS(") ");

#if USE_PAROLA_HW
  if ((buf > END_BUFFER) || (r >= ROW_SIZE))
#else
  if ((buf > END_BUFFER) || (r >= COL_SIZE))
#endif
	  return(0);

  PRINTX("0x", bitReverse(_matrix[buf].row[r]));

  return(bitReverse(_matrix[buf].row[r]));

}

#if USE_PAROLA_HW
bool MD_MAX72XX::setRow(uint8_t buf, uint8_t r, uint8_t value)
#else
bool MD_MAX72XX::setColumn(uint8_t buf, uint8_t r, uint8_t value)
#endif
{
#if USE_PAROLA_HW
  PRINT("\nsetRow: (", buf);
#else
  PRINT("\nsetCol: (", buf);
#endif
  PRINT(", ", r);
  PRINTX(") 0x", value);

#if USE_PAROLA_HW
  if ((buf > END_BUFFER) || (r >= ROW_SIZE))
#else
  if ((buf > END_BUFFER) || (r >= COL_SIZE))
#endif
    return(false);

  _matrix[buf].row[r] = bitReverse(value);
  bitSet(_matrix[buf].changed, r);

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

bool MD_MAX72XX::transform(uint8_t buf, transformType_t ttype)
{
  if (buf > END_BUFFER)
    return(false);

  if (!transformBuffer(buf, ttype))
    return(false);

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

bool MD_MAX72XX::transformBuffer(uint8_t buf, transformType_t ttype)
{
  uint8_t t[ROW_SIZE];
      
  switch (ttype)
  {

	//--------------
    case TSL: // Transform Shift Left one pixel element
#if USE_PAROLA_HW
      for (uint8_t i=0; i<ROW_SIZE; i++)
        _matrix[buf].row[i] >>= 1;
#else  // Same code a PAROLA_HW TSD
      for (uint8_t i=ROW_SIZE; i>0; --i)
        _matrix[buf].row[i] = _matrix[buf].row[i-1];
#endif
    break;
    
	//--------------
	case TSR:	// Transform Shift Right one pixel element
      for (uint8_t i=0; i<ROW_SIZE-1; i++)
#if USE_PAROLA_HW
        _matrix[buf].row[i] <<= 1;
#else  // Same code as PAROLA_HW TSU
        _matrix[buf].row[i] = _matrix[buf].row[i+1];
#endif
    break;
    
	//--------------
    case TSU: // Transform Shift Up one pixel element
	  if (_wrapAround)	// save the first row or a zero row
		  t[0] = getRow(buf, 0);
	  else
		  t[0] = 0;

#if USE_PAROLA_HW
      for (uint8_t i=0; i<ROW_SIZE-1; i++)
        _matrix[buf].row[i] = _matrix[buf].row[i+1];
#else  // Same code as PAROLA_HW TSR
      for (int8_t i=ROW_SIZE-1; i>=0; i--)
        _matrix[buf].row[i] <<= 1;
#endif
	  setRow(buf, ROW_SIZE-1, t[0]);
    break;
    
	//--------------
    case TSD: // Transform Shift Down one pixel element
	  if (_wrapAround)	// save the last row or a zero row
		  t[0] = getRow(buf, ROW_SIZE-1);
	  else
		  t[0] = 0;

#if USE_PAROLA_HW
      for (uint8_t i=ROW_SIZE; i>0; --i)
        _matrix[buf].row[i] = _matrix[buf].row[i-1];
#else
	  // Same code a PAROLA_HW TSL
      for (uint8_t i=0; i<ROW_SIZE; i++)
        _matrix[buf].row[i] >>= 1;
#endif
      setRow(buf, 0, t[0]);
    break;
    
	//--------------
#if USE_PAROLA_HW
	case TFLR: // Transform Flip Left to Right
#else
	case TFUD: // Transform Flip Up to Down
#endif
      for (uint8_t i=0; i<ROW_SIZE; i++)
        _matrix[buf].row[i] = bitReverse(_matrix[buf].row[i]);
    break;
    
	//--------------
#if USE_PAROLA_HW
    case TFUD: // Transform Flip Up to Down
#else
	case TFLR: // Transform Flip Left to Right
#endif
      for (uint8_t i=0; i<ROW_SIZE/2; i++)
      {
        uint8_t	t = _matrix[buf].row[i];
        _matrix[buf].row[i] = _matrix[buf].row[ROW_SIZE-i-1];
        _matrix[buf].row[ROW_SIZE-i-1] = t;
      }
    break;
    
	//--------------
    case TRC: // Transform Rotate Clockwise
		for (uint8_t i=0; i<ROW_SIZE; i++)
			t[i] = getColumn(buf, COL_SIZE-1-i); 

		for (uint8_t i=0; i<ROW_SIZE; i++)
			setRow(buf, i, t[i]);
    break;
    
	//--------------
	case TINV: // Transform INVert
      for (uint8_t i=0; i<ROW_SIZE; i++)
        _matrix[buf].row[i] = ~_matrix[buf].row[i];
    break;

    default:
      return(false);
  }
  
  _matrix[buf].changed = ALL_CHANGED;
  
  return(true);
}

uint8_t MD_MAX72XX::bitReverse(uint8_t b)
// Reverse the order of bits within a byte.
// Returns: The reversed byte value.
{
  b = ((b & 0xf0) >>  4) | ((b & 0x0f) << 4);
  b = ((b & 0xcc) >>  2) | ((b & 0x33) << 2);
  b = ((b & 0xaa) >>  1) | ((b & 0x55) << 1);

  return(b);
}

