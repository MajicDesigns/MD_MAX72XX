/* Text 2 Font for MD_MAX72xx library

 Quick and not very robust code to create a font definition data table for the
 MD_MAX72xx library from a text file representation. The text file has '.' commands
 to direct how the definition is structured.
 
 This is a console application written in standard C.
 Original target is Win32, but no OS dependencies, so should be portable to other OS.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#pragma once

#define NAME_SIZE         50
#define FILE_NAME_SIZE    200 // may include path
#define FONT_NAME_SIZE    50
#define COMMENT_SIZE      40
#define ASCII_SIZE        256
#define INPUT_BUFFER_SIZE 200

#define SINGLE_HEIGHT 8
#define DOUBLE_HEIGHT_OFFSET  (ASCII_SIZE/2)  // ASCII code offset

#define IN_FILE_EXT   ".txt"
#define OUT_FILE_EXT  ".h"

#define SPACE ' '
#define STAR  '*'
#define NUL   '\0'
#define DOT '.'

#define CMD_NAME     "NAME"
#define CMD_FONTHIGH "FONT_HEIGHT"
#define CMD_HEIGHT   "HEIGHT"
#define CMD_WIDTH    "WIDTH"
#define CMD_CHAR     "CHAR"
#define CMD_NOTE     "NOTE"
#define CMD_END      "END"

// Data types ----------------
typedef struct
{
  // file handling
  FILE  *fpIn;
  FILE  *fpOut;
  char  fileRoot[FILE_NAME_SIZE];

  // font definition header
  char  name[FONT_NAME_SIZE];
  unsigned int doubleHeight; // 0 or 1
  unsigned int fixedWidth;   // 0 for variable, width otherwise
  unsigned int fontHeight;   // height in pixels, default to 8

  // input buffers and tracking
  unsigned int curCode; // the current ASCII character being processed
  unsigned int curBuf;  // the current buffer we are up to
  unsigned int bufSize; // the number of buffers used
  char buf[SINGLE_HEIGHT*2][INPUT_BUFFER_SIZE];

} Global_t, *pGlobal_t;

typedef struct
{
  char comment[COMMENT_SIZE]; // comment for this character
  unsigned int size;  // number of valid
  unsigned int *buf;  // size bytes allocated from memory

  } ASCIIDef_t, *pASCIIDef_t;
