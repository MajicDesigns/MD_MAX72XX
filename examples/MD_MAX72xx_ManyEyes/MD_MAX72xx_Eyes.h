#ifndef MDMAX72XXEYES_H
#define MDMAX72XXEYES_H

// Header file for enum and other data structure definitions 

#define	SMALL_EYEBALL	0

// Pupil related information
enum posPupil // Initials are for Top, Middle and Bottom; Left, Center and Right (eg, TL = Top Left)
{
  P_TL, P_TC, P_TR, 
  P_ML, P_MC, P_MR,
  P_BL, P_BC, P_BR 
};

typedef struct coordPupil
{
	posPupil	pos;	// the position related to the coord
	uint8_t		rc;		// top LH corner of the 2x2 square pupil packed into one byte
};
#define	PACK_RC(r, c)	((r<<4)|(c&0xf))
#define	UNPACK_R(rc)	(rc>>4)
#define	UNPACK_C(rc)	(rc&0xf)

#if SMALL_EYEBALL

coordPupil pupilData[] = 
{
	{ P_TL, PACK_RC(2,5) },
	{ P_TC, PACK_RC(2,4) },
	{ P_TR, PACK_RC(2,3) },
	{ P_ML, PACK_RC(3,5) },
	{ P_MC, PACK_RC(3,4) },
	{ P_MR, PACK_RC(3,3) },
	{ P_BL, PACK_RC(4,5) },
	{ P_BC, PACK_RC(4,4) },
	{ P_BR, PACK_RC(4,3) },
};

// Eye related information
uint8_t eyeballData[] = { 0x00, 0x3c, 0x7e, 0x7e, 0x7e, 0x7e, 0x3c, 0x00 }; // row data
#define	LAST_BLINK_ROW	6		// last row for the blink animation	

#else

coordPupil pupilData[] = 
{
	{ P_TL, PACK_RC(3,5) },
	{ P_TC, PACK_RC(3,4) },
	{ P_TR, PACK_RC(3,3) },
	{ P_ML, PACK_RC(4,5) },
	{ P_MC, PACK_RC(4,4) },
	{ P_MR, PACK_RC(4,3) },
	{ P_BL, PACK_RC(5,5) },
	{ P_BC, PACK_RC(5,4) },
	{ P_BR, PACK_RC(5,3) },
};
uint8_t eyeballData[] = { 0x3c, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x3c };	// row data
#define	LAST_BLINK_ROW	7		// last row for the blink animation	

#endif


#endif