// EmotiveEye class static variables
#pragma once

#include "MD_RobotEyes.h"

// Sequences for animations
// Note: must add this to the lookupTable below as well so that the animation
// can be found by the animation engine.
const MD_RobotEyes::animFrame_t MD_RobotEyes::seqBlink[] PROGMEM =
{
  { { 0, 0 }, FRAME_TIME/2 },
  { { 1, 1 }, FRAME_TIME/2 },
  { { 2, 2 }, FRAME_TIME/2 },
  { { 3, 3 }, FRAME_TIME/2 },
  { { 4, 4 }, FRAME_TIME/2 },
  { { 5, 5 }, FRAME_TIME },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqWink[] PROGMEM =
{
  { { 0, 0 }, FRAME_TIME/2 },
  { { 1, 0 }, FRAME_TIME/2 },
  { { 2, 0 }, FRAME_TIME/2 },
  { { 3, 0 }, FRAME_TIME/2 },
  { { 4, 0 }, FRAME_TIME/2 },
  { { 5, 0 }, FRAME_TIME * 2 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqRight[] PROGMEM =
{
  { { 0, 0 }, FRAME_TIME },
  { { 6, 6 }, FRAME_TIME },
  { { 7, 7 }, FRAME_TIME * 5 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqLeft[] PROGMEM =
{
  { { 0, 0 }, FRAME_TIME },
  { { 8, 8 }, FRAME_TIME },
  { { 9, 9 }, FRAME_TIME * 5 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqUp[] PROGMEM =
{
  { { 00, 00 }, FRAME_TIME },
  { { 11, 11 }, FRAME_TIME },
  { { 12, 12 }, FRAME_TIME },
  { { 13, 13 }, FRAME_TIME * 5 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqDown[] PROGMEM =
{
  { { 00, 00 }, FRAME_TIME },
  { { 14, 14 }, FRAME_TIME },
  { { 15, 15 }, FRAME_TIME },
  { { 16, 16 }, FRAME_TIME * 5 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqAngry[] PROGMEM =
{
  { { 00, 00 }, FRAME_TIME },
  { { 22, 17 }, FRAME_TIME },
  { { 23, 18 }, FRAME_TIME },
  { { 24, 19 }, FRAME_TIME },
  { { 25, 20 }, 2000 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqSad[] PROGMEM =
{
  { { 00, 00 }, FRAME_TIME },
  { { 32, 27 }, FRAME_TIME },
  { { 33, 28 }, FRAME_TIME },
  { { 34, 29 }, 2000 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqEvil[] PROGMEM =
{
  { { 00, 00 }, FRAME_TIME },
  { { 39, 37 }, FRAME_TIME },
  { { 40, 38 }, 2000 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqEvil2[] PROGMEM =
{
  { { 00, 00 }, FRAME_TIME },
  { { 54, 17 }, FRAME_TIME },
  { { 55, 18 }, FRAME_TIME },
  { { 56, 19 }, FRAME_TIME },
  { { 57, 20 }, 2000 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqSquint[] PROGMEM =
{
  { { 00, 00 }, FRAME_TIME },
  { { 54, 54 }, FRAME_TIME },
  { { 55, 55 }, FRAME_TIME },
  { { 56, 56 }, FRAME_TIME },
  { { 57, 57 }, 2000 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqDead[] PROGMEM =
{
  { { 52, 52 }, FRAME_TIME * 4 },
  { { 53, 53 }, FRAME_TIME * 4 },
  { { 52, 52 }, FRAME_TIME * 2 },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqScanLeftRight[] PROGMEM =
{
  { { 41, 41 }, FRAME_TIME * 2 },
  { { 42, 42 }, FRAME_TIME },
  { { 43, 43 }, FRAME_TIME },
  { { 44, 44 }, FRAME_TIME },
};

const MD_RobotEyes::animFrame_t MD_RobotEyes::seqScanUpDown[] PROGMEM =
{
  { { 46, 46 }, FRAME_TIME * 2 },
  { { 47, 47 }, FRAME_TIME },
  { { 48, 48 }, FRAME_TIME },
  { { 49, 49 }, FRAME_TIME },
  { { 50, 50 }, FRAME_TIME },
  { { 51, 51 }, FRAME_TIME },
};

// Lookup table to find animation sequences
// Table associates the data for an emotion with the sequence table and it's size
const MD_RobotEyes::animTable_t MD_RobotEyes::lookupTable[] PROGMEM =
{
  { MD_RobotEyes::E_NEUTRAL, MD_RobotEyes::seqBlink, 1 }, // special case, fixed neutral stare
  { MD_RobotEyes::E_BLINK, MD_RobotEyes::seqBlink, ARRAY_SIZE(MD_RobotEyes::seqBlink) },
  { MD_RobotEyes::E_WINK, MD_RobotEyes::seqWink, ARRAY_SIZE(MD_RobotEyes::seqWink) },
  { MD_RobotEyes::E_LOOK_L, MD_RobotEyes::seqLeft, ARRAY_SIZE(MD_RobotEyes::seqLeft) },
  { MD_RobotEyes::E_LOOK_R, MD_RobotEyes::seqRight, ARRAY_SIZE(MD_RobotEyes::seqRight) },
  { MD_RobotEyes::E_LOOK_U, MD_RobotEyes::seqUp, ARRAY_SIZE(MD_RobotEyes::seqUp) },
  { MD_RobotEyes::E_LOOK_D, MD_RobotEyes::seqDown, ARRAY_SIZE(MD_RobotEyes::seqDown) },
  { MD_RobotEyes::E_ANGRY, MD_RobotEyes::seqAngry, ARRAY_SIZE(MD_RobotEyes::seqAngry) },
  { MD_RobotEyes::E_SAD, MD_RobotEyes::seqSad, ARRAY_SIZE(MD_RobotEyes::seqSad) },
  { MD_RobotEyes::E_EVIL, MD_RobotEyes::seqEvil, ARRAY_SIZE(MD_RobotEyes::seqEvil) },
  { MD_RobotEyes::E_EVIL2, MD_RobotEyes::seqEvil2, ARRAY_SIZE(MD_RobotEyes::seqEvil2) },
  { MD_RobotEyes::E_SQUINT, MD_RobotEyes::seqSquint, ARRAY_SIZE(MD_RobotEyes::seqSquint) },
  { MD_RobotEyes::E_DEAD, MD_RobotEyes::seqDead, ARRAY_SIZE(MD_RobotEyes::seqDead) },
  { MD_RobotEyes::E_SCAN_LR, MD_RobotEyes::seqScanLeftRight, ARRAY_SIZE(MD_RobotEyes::seqScanLeftRight) },
  { MD_RobotEyes::E_SCAN_UD, MD_RobotEyes::seqScanUpDown, ARRAY_SIZE(MD_RobotEyes::seqScanUpDown) },
};

// Font file (bitmaps for emotion animation frames)
MD_MAX72XX::fontType_t _RobotEyes_Font[] PROGMEM = 
{
  8, 0, 126, 129, 177, 177, 129, 126, 0,  // 0 - 'Rest Position'
  8, 0, 124, 130, 178, 178, 130, 124, 0,  // 1 - 'Blink 1'
  8, 0, 120, 132, 180, 180, 132, 120, 0,  // 2 - 'Blink 2'
  8, 0, 48, 72, 120, 120, 72, 48, 0,  // 3 - 'Blink 3'
  8, 0, 32, 80, 112, 112, 80, 32, 0,  // 4 - 'Blink 4'
  8, 0, 32, 96, 96, 96, 96, 32, 0,  // 5 - 'Blink 5'
  8, 0, 126, 129, 129, 177, 177, 126, 0,  // 6 - 'Right 1'
  8, 0, 0, 126, 129, 129, 177, 177, 126,  // 7 - 'Right 2'
  8, 0, 126, 177, 177, 129, 129, 126, 0,  // 8 - 'Left 1'
  8, 126, 177, 177, 129, 129, 126, 0, 0,  // 9 - 'Left 2'
  0,  // 10
  8, 0, 126, 129, 153, 153, 129, 126, 0,  // 11 - 'Up 1'
  8, 0, 126, 129, 141, 141, 129, 126, 0,  // 12 - 'Up 2'
  8, 0, 126, 129, 135, 135, 129, 126, 0,  // 13 - 'Up 3'
  8, 0, 126, 129, 225, 225, 129, 126, 0,  // 14 - 'Down 1'
  8, 0, 126, 129, 193, 193, 129, 126, 0,  // 15 - 'Down 2'
  8, 0, 124, 130, 194, 194, 130, 124, 0,  // 16 - 'Down 3'
  8, 0, 124, 130, 177, 177, 129, 126, 0,  // 17 - 'Angry L 1'
  8, 0, 120, 132, 178, 177, 129, 126, 0,  // 18 - 'Angry L 2'
  8, 0, 112, 136, 164, 178, 129, 126, 0,  // 19 - 'Angry L 3'
  8, 0, 96, 144, 168, 180, 130, 127, 0,  // 20 - 'Angry L 4'
  0,  // 21
  8, 0, 126, 129, 177, 177, 130, 124, 0,  // 22 - 'Angry R 1'
  8, 0, 126, 129, 177, 178, 132, 120, 0,  // 23 - 'Angry R 2'
  8, 0, 126, 129, 178, 164, 136, 112, 0,  // 24 - 'Angry R 3'
  8, 0, 127, 130, 180, 168, 144, 96, 0,  // 25 - 'Angry R 4'
  0,  // 26
  8, 0, 62, 65, 153, 153, 130, 124, 0,  // 27 - 'Sad L 1'
  8, 0, 30, 33, 89, 154, 132, 120, 0,  // 28 - 'Sad L 2'
  8, 0, 14, 17, 41, 90, 132, 120, 0,  // 29 - 'Sad L 3'
  0,  // 30
  0,  // 31
  8, 0, 124, 130, 153, 153, 65, 62, 0,  // 32 - 'Sad R 1'
  8, 0, 120, 132, 154, 89, 33, 30, 0,  // 33 - 'Sad R 2'
  8, 0, 120, 132, 90, 41, 17, 14, 0,  // 34 - 'Sad R 3'
  0,  // 35
  0,  // 36
  8, 0, 124, 194, 177, 177, 193, 126, 0,  // 37 - 'Evil L 1'
  8, 0, 56, 68, 178, 177, 66, 60, 0,  // 38 - 'Evil L 2'
  8, 0, 126, 193, 177, 177, 194, 124, 0,  // 39 - 'Evil R 1'
  8, 0, 60, 66, 177, 178, 68, 56, 0,  // 40 - 'Evil R 2'
  8, 0, 126, 129, 129, 129, 189, 126, 0,  // 41 - 'Scan H 1'
  8, 0, 126, 129, 129, 189, 129, 126, 0,  // 42 - 'Scan H 2'
  8, 0, 126, 129, 189, 129, 129, 126, 0,  // 43 - 'Scan H 3'
  8, 0, 126, 189, 129, 129, 129, 126, 0,  // 44 - 'Scan H 4'
  0,  // 45
  8, 0, 126, 129, 131, 131, 129, 126, 0,  // 46 - 'Scan V 1'
  8, 0, 126, 129, 133, 133, 129, 126, 0,  // 47 - 'Scan V 2'
  8, 0, 126, 129, 137, 137, 129, 126, 0,  // 48 - 'Scan V 3'
  8, 0, 126, 129, 145, 145, 129, 126, 0,  // 49 - 'Scan V 4'
  8, 0, 126, 129, 161, 161, 129, 126, 0,  // 50 - 'Scan V 5'
  8, 0, 126, 129, 193, 193, 129, 126, 0,  // 51 - 'Scan V 6'
  8, 0, 126, 137, 157, 137, 129, 126, 0,  // 52 - 'RIP 1'
  8, 0, 126, 129, 145, 185, 145, 126, 0,  // 53 - 'RIP 2'
  8, 0, 60, 66, 114, 114, 66, 60, 0,  // 54 - 'Peering 1'
  8, 0, 56, 68, 116, 116, 68, 56, 0,  // 55 - 'Peering 2'
  8, 0, 48, 72, 120, 120, 72, 48, 0,  // 56 - 'Peering 3'
  8, 0, 32, 80, 112, 112, 80, 32, 0,  // 57 - 'Peering 4'
  0,  // 58
  0,  // 59 - 'Unused'
  0,  // 60 - 'Unused'
  0,  // 61 - 'Unused'
  0,  // 62 - 'Unused'
  0,  // 63 - 'Unused'
  0,  // 64 - 'Unused'
  0,  // 65 - 'Unused'
  0,  // 66 - 'Unused'
  0,  // 67 - 'Unused'
  0,  // 68 - 'Unused'
  0,  // 69 - 'Unused'
  0,  // 70 - 'Unused'
  0,  // 71 - 'Unused'
  0,  // 72 - 'Unused'
  0,  // 73 - 'Unused'
  0,  // 74 - 'Unused'
  0,  // 75 - 'Unused'
  0,  // 76 - 'Unused'
  0,  // 77 - 'Unused'
  0,  // 78 - 'Unused'
  0,  // 79 - 'Unused'
  0,  // 80 - 'Unused'
  0,  // 81 - 'Unused'
  0,  // 82 - 'Unused'
  0,  // 83 - 'Unused'
  0,  // 84 - 'Unused'
  0,  // 85 - 'Unused'
  0,  // 86 - 'Unused'
  0,  // 87 - 'Unused'
  0,  // 88 - 'Unused'
  0,  // 89 - 'Unused'
  0,  // 90 - 'Unused'
  0,  // 91 - 'Unused'
  0,  // 92 - 'Unused'
  0,  // 93 - 'Unused'
  0,  // 94 - 'Unused'
  0,  // 95 - 'Unused'
  0,  // 96 - 'Unused'
  0,  // 97 - 'Unused'
  0,  // 98 - 'Unused'
  0,  // 99 - 'Unused'
  0,  // 100 - 'Unused'
  0,  // 101 - 'Unused'
  0,  // 102 - 'Unused'
  0,  // 103 - 'Unused'
  0,  // 104 - 'Unused'
  0,  // 105 - 'Unused'
  0,  // 106 - 'Unused'
  0,  // 107 - 'Unused'
  0,  // 108 - 'Unused'
  0,  // 109 - 'Unused'
  0,  // 110 - 'Unused'
  0,  // 111 - 'Unused'
  0,  // 112 - 'Unused'
  0,  // 113 - 'Unused'
  0,  // 114 - 'Unused'
  0,  // 115 - 'Unused'
  0,  // 116 - 'Unused'
  0,  // 117 - 'Unused'
  0,  // 118 - 'Unused'
  0,  // 119 - 'Unused'
  0,  // 120 - 'Unused'
  0,  // 121 - 'Unused'
  0,  // 122 - 'Unused'
  0,  // 123 - 'Unused'
  0,  // 124 - 'Unused'
  0,  // 125 - 'Unused'
  0,  // 126 - 'Unused'
  0,  // 127 - 'Unused'
  0,  // 128 - 'Unused'
  0,  // 129 - 'Unused'
  0,  // 130 - 'Unused'
  0,  // 131 - 'Unused'
  0,  // 132 - 'Unused'
  0,  // 133 - 'Unused'
  0,  // 134 - 'Unused'
  0,  // 135 - 'Unused'
  0,  // 136 - 'Unused'
  0,  // 137 - 'Unused'
  0,  // 138 - 'Unused'
  0,  // 139 - 'Unused'
  0,  // 140 - 'Unused'
  0,  // 141 - 'Unused'
  0,  // 142 - 'Unused'
  0,  // 143 - 'Unused'
  0,  // 144 - 'Unused'
  0,  // 145 - 'Unused'
  0,  // 146 - 'Unused'
  0,  // 147 - 'Unused'
  0,  // 148 - 'Unused'
  0,  // 149 - 'Unused'
  0,  // 150 - 'Unused'
  0,  // 151 - 'Unused'
  0,  // 152 - 'Unused'
  0,  // 153 - 'Unused'
  0,  // 154 - 'Unused'
  0,  // 155 - 'Unused'
  0,  // 156 - 'Unused'
  0,  // 157 - 'Unused'
  0,  // 158 - 'Unused'
  0,  // 159 - 'Unused'
  0,  // 160 - 'Unused'
  0,  // 161 - 'Unused'
  0,  // 162 - 'Unused'
  0,  // 163 - 'Unused'
  0,  // 164 - 'Unused'
  0,  // 165 - 'Unused'
  0,  // 166 - 'Unused'
  0,  // 167 - 'Unused'
  0,  // 168 - 'Unused'
  0,  // 169 - 'Unused'
  0,  // 170 - 'Unused'
  0,  // 171 - 'Unused'
  0,  // 172 - 'Unused'
  0,  // 173 - 'Unused'
  0,  // 174 - 'Unused'
  0,  // 175 - 'Unused'
  0,  // 176 - 'Unused'
  0,  // 177 - 'Unused'
  0,  // 178 - 'Unused'
  0,  // 179 - 'Unused'
  0,  // 180 - 'Unused'
  0,  // 181 - 'Unused'
  0,  // 182 - 'Unused'
  0,  // 183 - 'Unused'
  0,  // 184 - 'Unused'
  0,  // 185 - 'Unused'
  0,  // 186 - 'Unused'
  0,  // 187 - 'Unused'
  0,  // 188 - 'Unused'
  0,  // 189 - 'Unused'
  0,  // 190 - 'Unused'
  0,  // 191 - 'Unused'
  0,  // 192 - 'Unused'
  0,  // 193 - 'Unused'
  0,  // 194 - 'Unused'
  0,  // 195 - 'Unused'
  0,  // 196 - 'Unused'
  0,  // 197 - 'Unused'
  0,  // 198 - 'Unused'
  0,  // 199 - 'Unused'
  0,  // 200 - 'Unused'
  0,  // 201 - 'Unused'
  0,  // 202 - 'Unused'
  0,  // 203 - 'Unused'
  0,  // 204 - 'Unused'
  0,  // 205 - 'Unused'
  0,  // 206 - 'Unused'
  0,  // 207 - 'Unused'
  0,  // 208 - 'Unused'
  0,  // 209 - 'Unused'
  0,  // 210 - 'Unused'
  0,  // 211 - 'Unused'
  0,  // 212 - 'Unused'
  0,  // 213 - 'Unused'
  0,  // 214 - 'Unused'
  0,  // 215 - 'Unused'
  0,  // 216 - 'Unused'
  0,  // 217 - 'Unused'
  0,  // 218 - 'Unused'
  0,  // 219 - 'Unused'
  0,  // 220 - 'Unused'
  0,  // 221 - 'Unused'
  0,  // 222 - 'Unused'
  0,  // 223 - 'Unused'
  0,  // 224 - 'Unused'
  0,  // 225 - 'Unused'
  0,  // 226 - 'Unused'
  0,  // 227 - 'Unused'
  0,  // 228 - 'Unused'
  0,  // 229 - 'Unused'
  0,  // 230 - 'Unused'
  0,  // 231 - 'Unused'
  0,  // 232 - 'Unused'
  0,  // 233 - 'Unused'
  0,  // 234 - 'Unused'
  0,  // 235 - 'Unused'
  0,  // 236 - 'Unused'
  0,  // 237 - 'Unused'
  0,  // 238 - 'Unused'
  0,  // 239 - 'Unused'
  0,  // 240 - 'Unused'
  0,  // 241 - 'Unused'
  0,  // 242 - 'Unused'
  0,  // 243 - 'Unused'
  0,  // 244 - 'Unused'
  0,  // 245 - 'Unused'
  0,  // 246 - 'Unused'
  0,  // 247 - 'Unused'
  0,  // 248 - 'Unused'
  0,  // 249 - 'Unused'
  0,  // 250 - 'Unused'
  0,  // 251 - 'Unused'
  0,  // 252 - 'Unused'
  0,  // 253 - 'Unused'
  0,  // 254 - 'Unused'
  0,  // 255 - 'Unused'
};
