/*
MD_MAX72xx - Library for using a MAX7219/7221 LED matrix controller

See header file for comments

This file contains methods that act on the matrix as a pixel field,
generally only acting on the visible device range of the buffered
device field (ie, the physical pixel matrix).

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
#include <Arduino.h>
#include "MD_MAX72xx.h"
#include "MD_MAX72xx_lib.h"

/**
 * \file
 * \brief Implements pixel related methods
 */

void MD_MAX72XX::clear(uint8_t startDev, uint8_t endDev)
{
  if (endDev < startDev) return;

  for (uint8_t buf = startDev; buf <= endDev; buf++)
  {
    memset(_matrix[buf].dig, 0, sizeof(_matrix[buf].dig));
    _matrix[buf].changed = ALL_CHANGED;
  }

  if (_updateEnabled) flushBufferAll();
}

bool MD_MAX72XX::getBuffer(uint16_t col, uint8_t size, uint8_t *pd)
{
  if ((col >= getColumnCount()) || (pd == NULL))
    return(false);

  for (uint8_t i=0; i<size; i++)
    *pd++ = getColumn(col--);

  return(true);
}

bool MD_MAX72XX::setBuffer(uint16_t col, uint8_t size, uint8_t *pd)
{
  bool b = _updateEnabled;

  if ((col >= getColumnCount()) || (pd == NULL))
    return(false);

  _updateEnabled = false;
  for (uint8_t i=0; i<size; i++)
    setColumn(col--, *pd++);
  _updateEnabled = b;

  if (_updateEnabled) flushBufferAll();

  return(true);
}

bool MD_MAX72XX::drawHLine(uint8_t r, uint16_t c1, uint16_t c2, bool state)
// draw a horizontal line at row r between columns c1 and c2 inclusive
{
  if (r >= ROW_SIZE || c1 >= getColumnCount() || c2 >= getColumnCount())
    return(false);

  if (c1 > c2)      // swap c1/c2
  {
    uint16_t  t = c1;
    c1 = c2;
    c2 = t;
  }

  for (uint16_t i = c1; i <= c2; i++)
    setPoint(r, i, state);
}

bool MD_MAX72XX::drawVLine(uint16_t c, uint8_t r1, uint8_t r2, bool state)
// draw a vertical line at column c between rows r1 and r2 inclusive
{
  if (r1 >= ROW_SIZE || r2 >= ROW_SIZE || c >= getColumnCount())
    return(false);

  if (r1 > r2)      // swap r1/r2
  {
    uint8_t  t = r1;
    r1 = r2;    
    r2 = t;
  }

  for (uint8_t i = r1; i <= r2; i++)
    setPoint(i, c, state);
}

bool MD_MAX72XX::drawRectangle(uint8_t r1, uint16_t c1, uint8_t r2, uint16_t c2, bool state)
// draw a rectangle given the 2 diagonal vertices
{
  if (r1 >= ROW_SIZE || r2 >= ROW_SIZE || c1 >= getColumnCount() || c2 >= getColumnCount())
    return(false);

  drawHLine(r1, c1, c2, state);
  drawHLine(r2, c1, c2, state);
  drawVLine(c1, r1, r2, state);
  drawVLine(c2, r1, r2, state);
}

bool MD_MAX72XX::drawLine(uint8_t r1, uint16_t c1, uint8_t r2, uint16_t c2, bool state)
// draw an arbitrary line between two points using Bresentham's line algorithm
{
  if (r1 >= ROW_SIZE || r2 >= ROW_SIZE || c1 >= getColumnCount() || c2 >= getColumnCount())
    return(false);

  if (c1 > c2)
  {
    uint16_t  t;
    t = c1;   c1 = c2;    c2 = t;   // swap c1/c2
    t = r1;   r1 = r2;    r2 = t;   // swap r1/r2
  }

  // Bresentham's line algorithm
  int16_t dc = abs(c2-c1);
  int16_t sc = c1<c2 ? 1 : -1;
  int16_t dr = abs(r2-r1);
  int16_t sr = r1<r2 ? 1 : -1;
  int16_t err = (dc>dr ? dc : -dr)/2;
  int16_t e2;

  for(;;)
  {
    setPoint(r1, c1, state);
    if (c1 == c2 && r1 == r2) break;
    e2 = err;
    if (e2 >-dc) { err -= dr; c1 += sc; }
    if (e2 < dr) { err += dc; r1 += sr; }
  }

  if (_updateEnabled) flushBufferAll();

  return(true);
}

// used in getPoint and setPoint!
#if HW_DIG_ROWS
#define R r
#define C c
#else
#define R c
#define C r
#endif

bool MD_MAX72XX::getPoint(uint8_t r, uint16_t c)
{
  uint8_t	buf = c/COL_SIZE;

  c %= COL_SIZE;
  PRINT("\ngetPoint: (", buf);
  PRINT(", ", r);
  PRINT(", ", c);
  PRINTS(")");

  if ((buf > LAST_BUFFER) || (r >= ROW_SIZE) || (c >= COL_SIZE))
    return(false);

  return(bitRead(_matrix[buf].dig[HW_ROW(R)], HW_COL(C)) == 1);
}

bool MD_MAX72XX::setPoint(uint8_t r, uint16_t c, bool state)
{
  uint8_t	buf = c/COL_SIZE;
  c %= COL_SIZE;

  PRINT("\nsetPoint: (", buf);
  PRINT(", ", r);
  PRINT(", ", c);
  PRINT(") = ", state?1:0);

  if ((buf > LAST_BUFFER) || (r >= ROW_SIZE) || (c >= COL_SIZE))
    return(false);

  if (state)
    bitSet(_matrix[buf].dig[HW_ROW(R)], HW_COL(C));
  else
    bitClear(_matrix[buf].dig[HW_ROW(R)], HW_COL(C));

  bitSet(_matrix[buf].changed, HW_ROW(R));

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

#undef  R
#undef  C

bool MD_MAX72XX::setRow(uint8_t startDev, uint8_t endDev, uint8_t r, uint8_t value)
{
  bool b = _updateEnabled;

  PRINT("\nsetRow: ", r);

  if ((r >= ROW_SIZE) || (endDev < startDev))
    return(false);

  _updateEnabled = false;
  for (uint8_t i = startDev; i <= endDev; i++)
    setRow(i, r, value);
  _updateEnabled = b;

  if (_updateEnabled) flushBufferAll();

  return(true);
}

bool MD_MAX72XX::transform(uint8_t startDev, uint8_t endDev, transformType_t ttype)
{
 // uint8_t t[ROW_SIZE];
  uint8_t colData;
  bool b = _updateEnabled;

  if (endDev < startDev) return(false);

  _updateEnabled = false;

  switch (ttype)
  {
    case TSL: // Transform Shift Left one pixel element (with overflow)
    colData = 0;
    // if we can call the user function later then we don't need to do anything here
    // however, wraparound mode means we know the data so no need to request from the
    // callback at all - just save it for later
    if (_wrapAround)
      colData = getColumn(((endDev+1)*COL_SIZE)-1);
    else if (_cbShiftDataOut != NULL)
      (*_cbShiftDataOut)(endDev, ttype, getColumn(((endDev+1)*COL_SIZE)-1));

    // shift all the buffers along
    for (int8_t buf = endDev; buf >= startDev; --buf)
    {
      transformBuffer(buf, ttype);
      // handle the boundary condition
      setColumn(buf, 0, getColumn(buf-1, COL_SIZE-1));
    }

    // if we have a callback function, now is the time to get the data if we are
    // not in wraparound mode
    if (_cbShiftDataIn != NULL && !_wrapAround)
      colData = (*_cbShiftDataIn)(startDev, ttype);

    setColumn((startDev*COL_SIZE), colData);
    break;

    case TSR: // Transform Shift Right one pixel element (with overflow)
    // if we can call the user function later then we don't need to do anything here
    // however, wraparound mode means we know the data so no need to request from the
    // callback at all - just save it for later.
    colData = 0;
    if (_wrapAround)
      colData = getColumn(startDev*COL_SIZE);
    else if (_cbShiftDataOut != NULL)
      (*_cbShiftDataOut)(startDev, ttype, getColumn((startDev*COL_SIZE)));

    // shift all the buffers along
    for (uint8_t buf=startDev; buf<=endDev; buf++)
    {
      transformBuffer(buf, ttype);

      // handle the boundary condition
      setColumn(buf, COL_SIZE-1, getColumn(buf+1, 0));
    }

    // if we have a callback function, now is the time to get the data if we are
    // not in wraparound mode
    if (_cbShiftDataIn != NULL && !_wrapAround)
      colData = (*_cbShiftDataIn)(endDev, ttype);

    setColumn(((endDev+1)*COL_SIZE)-1, colData);
    break;

    case TFLR: // Transform Flip Left to Right (use the whole field)
    // first reverse the device buffers end for end
    for (uint8_t buf = 0; buf < (endDev - startDev)/2; buf++)
    {
      deviceInfo_t	t;

      t = _matrix[startDev + buf];
      _matrix[startDev + buf] = _matrix[endDev - buf];
      _matrix[endDev - buf] = t;
    }

    // now reverse the columns in each device
    for (uint8_t buf = startDev; buf <= endDev; buf++)
      transformBuffer(buf, ttype);
    break;

    // These next transformations work the same just by doing the individual devices
    case TSU:   // Transform Shift Up one pixel element
    case TSD:   // Transform Shift Down one pixel element
    case TFUD:  // Transform Flip Up to Down
    case TRC:   // Transform Rotate Clockwise
    case TINV:  // Transform INVert
    for (uint8_t buf = startDev; buf <= endDev; buf++)
      transformBuffer(buf, ttype);
    break;

    default:
      return(false);
  }

  _updateEnabled = b;

  if (_updateEnabled) flushBufferAll();

  return(true);
}