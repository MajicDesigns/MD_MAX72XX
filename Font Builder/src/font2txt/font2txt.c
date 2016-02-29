// Font 2 Text for MD_MAX72xx library
//
// Quick and not very robust code to create a font defintion text file
// from an existing font data table. This is considered a one-off task
// followed by some manual editing of the file.
// The file is expected to be in the similar format to the output from
// txt2font. Specifically, it is expected that each character is on one
// data line in order to recognise specific parts, like comments.
//
// The shared header file means that some elements in this 
// code are reversed (eg, the IN file extension is used as the OUT file 
// extension). Each font element is output as it is completed, without 
// buffering the while ASCII set.
//
#include "txt2font.h"

// Global data ---------------
Global_t	G;
ASCIIDef_t	font = { 0 };

// Code ----------------------
void usage(void)
{
	printf("\nusage: font2txt <root_name>\n");
	printf("\n\ninput file  <root_name>.h");
	printf("\noutput file <root_name>.txt");
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
// one time initiualisation code
{
	char	szFile[FILE_NAME_SIZE];

	// we have no font definition
	font.comment[0] = NUL;
	font.size = 0;
	font.buf = malloc(100*sizeof(font.buf[0])); // pick a big number...

	// open the file for reading
	strcpy(szFile, G.fileRoot);
	strcat(szFile, OUT_FILE_EXT);
	if ((G.fpIn = fopen(szFile, "r")) == NULL)
	{
		printf("\nCannot open input ""%s\n""", szFile);
		return(2);
	}

	// open the file for writing
	strcpy(szFile, G.fileRoot);
	strcat(szFile, IN_FILE_EXT);
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

	return(0);
}

void trimBuffer(char *buf)
// trim the buffer specified of all trailing white space
{
	int	i = strlen(buf)-1;

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
// normalise all the input buffers to the same size
{
	unsigned int	max = 0;

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
	while (isspace(*buf) || ispunct(*buf))
		buf++;
	while (!isspace(*buf) && !ispunct(*buf))
		buf++;
	*buf++ = NUL;
	while (isspace(*buf) || ispunct(*buf))
		buf++;

	return(buf);
}

void saveOutputHeader(void)
// save the current definition as a font definition header file
{
	fprintf(G.fpOut, "%c%s %s\n", DOT, CMD_NAME, G.fileRoot);
	fprintf(G.fpOut, "%c%s %d\n", DOT, CMD_HEIGHT, (G.doubleHeight ? 2: 1));
	fprintf(G.fpOut, "%c%s %d\n", DOT, CMD_WIDTH, G.fixedWidth);

	return;
}

void saveOutputFooter(void)
// save the current definition as a font definition header file
{
	fprintf(G.fpOut, "%c%s\n", DOT, CMD_END);

	return;
}

void saveOutputChar(void)
// Save a single character definition
{
	fprintf(G.fpOut, "%c%s %d\n", DOT, CMD_CHAR, G.curCode);
	fprintf(G.fpOut, "%c%s %s\n", DOT, CMD_NOTE, font.comment);

	if (G.bufSize != 0)
	{
		for (unsigned int i=0; i<G.bufSize; i++)
		{
			trimBuffer(G.buf[i]);
			fprintf(G.fpOut, "%s\n", G.buf[i]);
		}
	}
	else		// need at least one line
		fprintf(G.fpOut, "\n");

	return;
}

void createFontData(char *cp)
// the font defintions strings are column data columns, so each byte will be a 'vertical'
// stripe of the input buffers. Format is the size followed by size bytes of column data,
// separated by commas. The comment is found after the data in a '//' comment and following
// the first '-'.
{
	char	*cpNext;

	cpNext = getToken(cp);
	font.size = atoi(cp);
	cp = cpNext;

	// get all the data from the input into the font defintion
	for (unsigned int i=0; i<font.size; i++)
	{
		// get the data
		cpNext = getToken(cp);
		font.buf[i] = atoi(cp);
		cp = cpNext;
	}

	// unpack the column data into the text strings
	// create each input string [j] in turn from the column definition [i] for the char
	for (unsigned int i=0; i<SINGLE_HEIGHT; i++)
	{
		for (unsigned int j=0; j<font.size; j++)
			G.buf[i][j] = (font.buf[j] & (1<<i)) ? STAR : SPACE;
		G.buf[i][font.size] = NUL;	// terminate the string
	}

	// save the comment - first search up to after the first '-'
	while ((*cp != '-') && (*cp != NUL))
		cp++;
	if (cp != NUL) cp++;
	strcpy(font.comment, cp);

	return;
}

void processInput(void)
// read the input file and process line by line
{
	char	c, inLine[INPUT_BUFFER_SIZE];

	// skip to the opening brace then skip end of line
	do 
		c = getc(G.fpIn);
	while ((c != EOF) && (c != '{'));
	do 
		c = getc(G.fpIn);
	while ((c == '\n') || (c == '\r'));

	// now read each line in turn
	while (!feof(G.fpIn) && (G.curCode < ASCII_SIZE))
	{
		fgets(inLine, sizeof(inLine), G.fpIn); 

		// if we have a blank line, get another one
		trimBuffer(inLine);
		if (strlen(inLine) == 0) continue;

		// process the buffers into a font definition and write out what we have created			
		createFontData(inLine);
		saveOutputChar();

		// reset the font and character buffers, increment current ASCII code
		for (unsigned int i=0; i<G.bufSize; i++)
			G.buf[i][0] = NUL;
		font.size = 0;
		G.curCode++;
	}

	return;
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

	saveOutputHeader();
	processInput();
	saveOutputFooter();


	// close files and exit
	fclose(G.fpIn);
	fclose(G.fpOut);

	return(0);
}
