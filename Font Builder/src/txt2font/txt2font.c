// Text 2 Font for MD_MAX72xx library
//
// Quick and not very robust code to create a font definition data table for the
// MD_MAX72xx library from a text file representation. The text file has '.' commands
// to direct how the definition is structured.
//
#include "txt2font.h"

//#define	DEBUG

#define	DECIMAL_DATA  0 // decimal or hex data selection in font tables

// Global data ---------------
Global_t    G;
ASCIIDef_t  font[ASCII_SIZE] = { 0 };

// Code ----------------------
void usage(void)
{
  printf("\nusage: txt2font <root_name>\n");
  printf("\n\ninput file  <root_name>.txt");
  printf("\noutput file <root_name>.h");
  printf("\n");

  return;
}

int cmdLine(int argc, char *argv[])
// process the command line parameter
{
  if (argc != 2)
    return(1);

  strcpy(G.fileRoot, argv[1]);

  return(0);
}

int initialise(void)
// one time initialisation code
{
  char szFile[FILE_NAME_SIZE];

  // we have no font definition
  for (int i=0; i<ASCII_SIZE; i++)
  {
    font[i].comment[0] = NUL;
    font[i].size = 0;
    font[i].buf = NULL;
  }

  // open the file for reading
  strcpy(szFile, G.fileRoot);
  strcat(szFile, IN_FILE_EXT);
  if ((G.fpIn = fopen(szFile, "r")) == NULL)
  {
    printf("\nCannot open input ""%s\n""", szFile);
    return(2);
  }

  // open the file for writing
  strcpy(szFile, G.fileRoot);
  strcat(szFile, OUT_FILE_EXT);
  if ((G.fpOut = fopen(szFile, "w")) == NULL)
  {
    printf("\nCannot open output ""%s\n""", szFile);
    return(3);
  }

  // other stuff
  G.name[0] = NUL;
  G.doubleHeight = 0;
  G.bufSize = SINGLE_HEIGHT;
  G.fixedWidth = 0;
  G.fontHeight = 8;

  return(0);
}

void trimBuffer(char *buf)
// trim the buffer specified of all trailing white space
{
  int i = strlen(buf)-1;

  while (i > 0 && isspace(buf[i]))
    buf[i--] = NUL;

  return;
}

void padBuffer(char *buf, unsigned int len)
// pad the supplied buffer with spaces up to the length specified
{
  if (strlen(buf) < len)
  {
    for (unsigned int j=strlen(buf); j<len; j++)
      buf[j] = SPACE;
  }
  buf[len] = NUL;

  return;
}

unsigned int normaliseBuffers(void)
// normalize all the input buffers to the same size
{
  unsigned int max = 0;

  if (G.fixedWidth == 0)
  {
    for (unsigned int i=0; i<G.bufSize; i++)
      max = max < strlen(G.buf[i]) ? max = strlen(G.buf[i]) : max;
  }
  else
    max = G.fixedWidth;

  // now make them all the same size
  for (unsigned int i=0; i<G.bufSize; i++)
  {
    padBuffer(G.buf[i], max);
  }

  return(max);
}

char *getToken(char *buf)
// isolate the first token in the buffer and return a pointer to the next non white space after the token
{
  while (!isspace(*buf))
    buf++;
  *buf++ = NUL;
  while (isspace(*buf))
    buf++;

  return(buf);
}

void createFontChar(void)
// the font definition strings are created  as columns, so each byte will be a 'vertical'
// stripe of the input buffers.
{
  font[G.curCode].size = normaliseBuffers();  // make everything the same length; this is the width of the character

  // allocate memory for the font data
  if (font[G.curCode].buf != NULL)
    free(font[G.curCode].buf);
  font[G.curCode].buf = malloc(font[G.curCode].size * sizeof(*font[0].buf));

  // if double height, check the upper buffers and also copy the font data across
  if (G.doubleHeight)
  {
    if (font[G.curCode+DOUBLE_HEIGHT_OFFSET].buf != NULL)
      free(font[G.curCode+DOUBLE_HEIGHT_OFFSET].buf);
    font[G.curCode+DOUBLE_HEIGHT_OFFSET].buf = malloc(font[G.curCode].size * sizeof(*font[0].buf));

    font[G.curCode+DOUBLE_HEIGHT_OFFSET].size = font[G.curCode].size;
    strcpy(font[G.curCode+DOUBLE_HEIGHT_OFFSET].comment, font[G.curCode].comment);
  }

  // process the input strings in parallel [j] to create the column definition [i] for the first character
  for (unsigned int i=0; i<font[G.curCode].size; i++)
  {
    unsigned int colL, colH;

    if (G.doubleHeight)  // if we are double height, write lower and upper halves separately
    {
      colL = colH = 0;
      // high half of character stored in upper ASCII value, but in low buffers
      // low  half of character stored in lower ASCII value, but in high buffers
      for (unsigned int j=0; j<SINGLE_HEIGHT; j++)
      {
        colH |= (G.buf[j][i] == SPACE) ? 0 : (1 << j);
        colL |= (G.buf[j+SINGLE_HEIGHT][i] == SPACE) ? 0 : (1 << j);
      }
      font[G.curCode+DOUBLE_HEIGHT_OFFSET].buf[i] = colH;
      font[G.curCode].buf[i] = colL; 
    }
    else
    {
      colL = 0;
      for (unsigned int j=0; j<SINGLE_HEIGHT; j++)
        colL |= (G.buf[j][i] == SPACE) ? 0 : (1 << j);
      // save the column data
      font[G.curCode].buf[i] = colL; 
    } 
  }

  return;
}

void readInput(void)
// read the input file and process line by line
{
  char c, *cp, inLine[INPUT_BUFFER_SIZE];

  cp = inLine;

  while ((c = getc(G.fpIn)) != EOF)
  {
    if (c != '\n' && c != '\r') // not end of line
    {
      *cp++ = c;
      continue;
    }

    // we are at end of line, process the line
    *cp = NUL;
    trimBuffer(inLine);
    if (inLine[0] != DOT) // not a command? must be a chr definition line
    {
      if (G.curBuf < (SINGLE_HEIGHT*2))
      {
#ifdef DEBUG
        printf("\nC%02x L%d\t%s", G.curCode, G.curBuf, inLine);
#endif
        strcpy(G.buf[G.curBuf++], inLine);  // save the buffer
      }
    }
    else
    {
      cp = getToken(&inLine[1]);
#ifdef DEBUG
      printf("\nToken: '%s' - '%s'", inLine, cp);
#endif
      if (strcmp(&inLine[1], CMD_NAME) == 0)
      {
        strcpy(G.name, cp);
#ifdef DEBUG
        printf("\tfont name: '%s'", G.name);
#endif
      }
      else if (strcmp(&inLine[1], CMD_FONTHIGH) == 0)
      {
        G.fontHeight = abs(atoi(cp));
#ifdef DEBUG
        printf("\tfont height: %d", G.fontHeight);
#endif
      }
      else if (strcmp(&inLine[1], CMD_HEIGHT) == 0)
      {
        G.doubleHeight = (*cp=='1' ? 0 : 1);
        G.bufSize = (*cp=='1' ? SINGLE_HEIGHT : SINGLE_HEIGHT*2);
#ifdef DEBUG
        printf("\tdouble height: %d", G.doubleHeight);
#endif
      }
      else if (strcmp(&inLine[1], CMD_WIDTH) == 0)
      {
        G.fixedWidth = abs(atoi(cp));
#ifdef DEBUG
        printf("\twidth: %d", G.fixedWidth);
#endif
      }
      else if ((strcmp(&inLine[1], CMD_CHAR) == 0) || 
            (strcmp(&inLine[1], CMD_END) == 0))
      {
#ifdef DEBUG
        printf("\tend char %02x", G.curCode);
#endif
        if (G.curBuf != 0)		// we have buffers
        {
          // process the buffers into a font definition
          createFontChar();

          // reset the character buffers
          for (unsigned int i=0; i<G.bufSize; i++)
            G.buf[i][0] = NUL;
        }

        // set up the new character if not at the end
        if (strcmp(&inLine[1], CMD_END) != 0)
        {
          G.curBuf = 0;
          G.curCode = atoi(cp);
#ifdef DEBUG
          printf("\t set up %02x", G.curCode);
#endif
          if (G.curCode >= (G.doubleHeight ? ASCII_SIZE/2 : ASCII_SIZE))
          {
            G.curCode = 0;
#ifdef DEBUG
            printf("\t - boundary check fail");
#endif
          }
          font[G.curCode].comment[0] = NUL;
          font[G.curCode].size = 0;
        }
#ifdef DEBUG
        else
          printf("\tend of definition");
#endif
      }
      else if (strcmp(&inLine[1], CMD_NOTE) == 0)
      {
        strcpy(font[G.curCode].comment, cp);
#ifdef DEBUG
        printf("\tnote '%s' for char %02x", font[G.curCode].comment, G.curCode);
#endif
      }

    }
    // reset for next line
    cp = inLine;
    memset(inLine, NUL, sizeof(inLine));
  }

  return;
}

void saveOutput(void)
// save the current definition as a font definition header file
{
  unsigned int minAscii = 0, maxAscii = 0;
  
  // first parse the font table to work out the min and max ASCII values
  for (int i=0; i<ASCII_SIZE; i++)
    if (font[i].buf != NULL) maxAscii = i;
    
  for (int i=maxAscii; i>=0; i--)
    if (font[i].buf != NULL) minAscii = i;

  fprintf(G.fpOut, "// Autogenerated font - '%s'\n", G.name);
  fprintf(G.fpOut, "// %s height, ", (G.doubleHeight ? "Double" : "Single"));
  if (G.fixedWidth == 0)
    fprintf(G.fpOut, "Variable spaced");
  else
    fprintf(G.fpOut, "Fixed width (%d)", G.fixedWidth);
  fprintf(G.fpOut, "\n\n");

  fprintf(G.fpOut, "#pragma once\n\n");
  fprintf(G.fpOut, "const uint8_t PROGMEM _%s[] = \n{\n", (G.name[0] == NUL) ? "font" : G.name);
  fprintf(G.fpOut, "'F', 1, %d, %d, %d,\n", minAscii, maxAscii, G.fontHeight);


  for (int i=minAscii; i<=maxAscii; i++)
  {
    fprintf(G.fpOut, "\t%d,", font[i].size);
    if (font[i].buf != NULL)
    {
      for (unsigned int j=0; j<font[i].size; j++)
        fprintf(G.fpOut, (DECIMAL_DATA ? "%d," : "0x%02x,"), font[i].buf[j]);
    }
    fprintf(G.fpOut, "\t// %d", i);
    if (font[i].comment[0] != NUL)
      fprintf(G.fpOut," - %s", font[i].comment);
    fprintf(G.fpOut, "\n");
  }

  fprintf(G.fpOut, "};\n\n");
  }

  int main(int argc, char *argv[])
  {
  int	ret;

  if (cmdLine(argc, argv))
  {
    usage();
    return(1);
  }

  if ((ret = initialise()) != 0)
    return(ret);

  // read the input file
  readInput();

  // write the output file
  saveOutput();

  // close files and exit
  fclose(G.fpIn);
  fclose(G.fpOut);

  return(0);
  }
