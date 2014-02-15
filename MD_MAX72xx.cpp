/*
MD_MAX72xx - Library for using a MAX7219/7221 LED matrix controller
  
See header file for comments
This file contains class and hardware related methods.
  
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
 * \brief Implements class definition and general methods
 */

MD_MAX72XX::MD_MAX72XX(uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices):
_dataPin(dataPin), _clkPin(clkPin), _csPin(csPin), _maxDevices(numDevices),
_updateEnabled(true)
{
	_hardwareSPI = false;
#if USE_LOCAL_FONT && USE_FONT_INDEX
	_fontIndex = NULL;
#endif
}

MD_MAX72XX::MD_MAX72XX(uint8_t csPin, uint8_t numDevices):
_dataPin(0), _clkPin(0), _csPin(csPin), _maxDevices(numDevices),
_updateEnabled(true)
{
	_hardwareSPI = true;
#if USE_LOCAL_FONT && USE_FONT_INDEX
	_fontIndex = NULL;
#endif
}

void MD_MAX72XX::begin(void)
{
  // initialise the AVR hardware
  if (_hardwareSPI)
  {
	// Set direction register for SCK and MOSI pin.
	// MISO pin automatically overrides to INPUT.
	// SS pin is still used and needs to be made HIGH
	pinMode(SS, OUTPUT);
	digitalWrite(SS, HIGH);
	pinMode(MOSI, OUTPUT);
	pinMode(SCK, OUTPUT);

	// Warning: if the SS ever becomes a LOW INPUT then SPI
	// automatically switches to Slave, so the data direction of
	// the SS pin MUST be kept as OUTPUT.
	SPCR |= _BV(MSTR);
	SPCR |= _BV(SPE);

	// Set SPI to MSB first
    SPCR &= ~(_BV(DORD));
  }
  else
  {
    pinMode(_dataPin, OUTPUT);
  	pinMode(_clkPin, OUTPUT);
  }

  // initialise our preferred CS pin (could be same as SS)
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);

  // object memory and internals
  setShiftDataInCallback(NULL);
  setShiftDataOutCallback(NULL);

  _matrix = (deviceInfo_t *)malloc(sizeof(deviceInfo_t) * ((_maxDevices) * 2));
  _spiData = (uint8_t *)malloc(SPI_DATA_SIZE);

#if USE_LOCAL_FONT
#if USE_INDEX_FONT
  _fontIndex = (uint16_t *)malloc(sizeof(uint16_t) * FONT_INDEX_SIZE);
#endif
  setFont(NULL);
#endif // INCLUDE_LOCAL_FONT

  // clear internal memory map for this device
  for (uint8_t d=START_BUFFER; d<=END_BUFFER; d++)
  {
    _matrix[d].changed = ALL_CLEAR;
    for (uint8_t i=0; i<ROW_SIZE; i++)
    {
      _matrix[d].row[i] = 0;
    }
  }
			
  // Initialise the display devices. 
  // On initial power-up, all control registers are reset, the
  // display is blanked, and the MAX7219/MAX7221 enter shutdown 
  // mode. We need to program the devices prior to display use, 
  // otherwise, it will initially be set to scan one digit, will 
  // not decode data in the data registers, and the intensity 
  // register will be set to its minimum value.
  control(TEST, OFF);				// no test
  control(SHUTDOWN, OFF);			// devices are turned on
  control(SCANLIMIT, ROW_SIZE-1);	// scanlimit is set to max on startup
  control(INTENSITY, MAX_INTENSITY/2);	// set intensity to a reasonable value
  clear();
  control(SHUTDOWN, ON);			// devices are turned off
}

MD_MAX72XX::~MD_MAX72XX(void)
{
	if (_hardwareSPI) SPCR &= ~_BV(SPE);	// reset SPI mode

	free(_matrix);
	free(_spiData);
#if USE_LOCAL_FONT && USE_FONT_INDEX
	if (_fontIndex != NULL) free(_fontIndex);
#endif
}

void MD_MAX72XX::control(controlRequest_t mode, int value) 
{
  if (mode < UPDATE)	// device based control
  {
    for (uint8_t i=START_BUFFER; i<=END_BUFFER; i++) 
	  control(i, mode, value); 
  }
  else					// global control function, doesn't relate to specific device
  {
    control(0, mode, value);
  }
}
  
bool MD_MAX72XX::control(uint8_t buf, controlRequest_t mode, int value)
// dev is zero based and needs adjustment if used
{
  uint8_t opcode = OP_NOOP;
  uint8_t param = 0;

  if (buf > END_BUFFER) return(false);
  
  // work out data to write
  switch (mode)
  {
    case SHUTDOWN:
      opcode = OP_SHUTDOWN;
      param = (value == OFF ? 1 : 0);
    break;
    
    case SCANLIMIT:
      opcode = OP_SCANLIMIT;
      param = (value > MAX_SCANLIMIT ? MAX_SCANLIMIT : value);
    break;
    
    case INTENSITY:
      opcode = OP_INTENSITY;
      param = (value > MAX_INTENSITY ? MAX_INTENSITY : value);
    break;
    
    case TEST:
      opcode = OP_DISPLAYTEST;
      param = (value == OFF ? 0 : 1);
    break;
    
    case UPDATE:
      _updateEnabled = (value == ON);
	  if (_updateEnabled) flushBufferAll();
      break;

    case WRAPAROUND:
      _wrapAround = (value == ON);
	  break;

    default:
      return(false);
  }

  // actually do the write to the device
  if (opcode != OP_NOOP) 
    spiSend(buf, opcode, param);

  return(true);
}

void MD_MAX72XX::spiClearBuffer(void)
// Clear out the spi data array
{
  memset(_spiData, SPI_DATA_SIZE, OP_NOOP);
}

void MD_MAX72XX::flushBufferAll()
// Only one data byte is sent to a device, so if there are many changes, it is more
// efficient to send a data byte all devices at the same time, substantially cutting 
// the number of communication messages required.
{
  for (uint8_t i=0; i<ROW_SIZE; i++)	// all data rows
  {
    bool bChange = false;	// set to true if we detected a change

	spiClearBuffer();

    for (uint8_t dev=START_BUFFER; dev<=END_BUFFER; dev++)	// all devices
    {
      if (bitRead(_matrix[dev].changed, i))
	  {
	    // put our device data into the buffer
		_spiData[SPI_OFFSET(dev, 1)] = OP_DIGIT0+i;
		_spiData[SPI_OFFSET(dev, 0)] = _matrix[dev].row[i];
		bChange = true;
	  }
    }

	if (bChange) spiTransmit();
  }

  // mark everything as cleared
  for (uint8_t dev=START_BUFFER; dev<=END_BUFFER; dev++)
	_matrix[dev].changed = ALL_CLEAR;
}

void MD_MAX72XX::flushBuffer(uint8_t buf)
// Use this function when the changes are limited to one device only.
// Address passed is a buffer address
{
  PRINT("\nflushBuf: ", buf);
  PRINTS(" r");

  if (buf > END_BUFFER) 
    return;

  for (uint8_t i=0; i<ROW_SIZE; i++)
  {
    if (bitRead(_matrix[buf].changed, i))
	{
	  PRINT("", i);
      spiClearBuffer();

      // put our device data into the buffer
      // device address is one less than the buffer address
      _spiData[SPI_OFFSET(buf,1)] = OP_DIGIT0+i;
      _spiData[SPI_OFFSET(buf,0)] = _matrix[buf].row[i];
    
      spiTransmit();
    }
  }
  _matrix[buf].changed = ALL_CLEAR;
}

void MD_MAX72XX::spiSend(uint8_t dev, uint8_t opcode, uint8_t data) 
// Use this function when needing to send just one byte (eg, a command)
// dev does not need to be adjusted for virtual buffers as this is an internal function
{
  spiClearBuffer();

  // put our device data into the buffer
  _spiData[SPI_OFFSET(dev,1)] = opcode;
  _spiData[SPI_OFFSET(dev,0)] = data;
    
  spiTransmit();
}

void MD_MAX72XX::spiTransmit() 
{
  // enable the devices to receive data
  digitalWrite(_csPin, LOW);

  // shift out the data 
  if (_hardwareSPI)
  {
    for (int i=SPI_DATA_SIZE-1; i>=0; i--)
	{
	  SPDR = _spiData[i];
	  while (!(SPSR & _BV(SPIF)))	// wait for a clear bit
		;
	}
  }
  else
  {
    for (int i=SPI_DATA_SIZE-1; i>=0; i--)
      shiftOut(_dataPin, _clkPin, MSBFIRST, _spiData[i]);
  }
		
  // latch the data onto the display
  digitalWrite(_csPin, HIGH);
}    
