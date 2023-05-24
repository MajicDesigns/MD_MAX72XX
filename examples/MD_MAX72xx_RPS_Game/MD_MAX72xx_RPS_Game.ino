// Use the MD_MAX72XX library to implements a Rock/Paper/Scissors game.
// Play against another player or the Arduino.
//
// Rock/paper/scissors is a hand game usually played between two people, 
// in which each player simultaneously forms one of three shapes with 
// their hand. These shapes are a closed fist("rock"), a flat hand 
// ("paper"), and a fist with index and middle fingers forming a V 
// ("scissors").
//
// The game has three possible outcomes (draw, win loss) based on the 
// following simple rules:
// - Rock beats scissors (rock break scissors).
// - Paper beats rock (paper covers rock).
// - Scissors beat paper (scissors cuts paper).
// - The game is a draw if both players form the same shape.
// 
// When the game is played with more than 2 players, everyone in the group 
// plays their hand. The result is determined by
// - if everyone shows the same shape then everyone plays again.
// - if all 3 shapes shapes are showing then everyone plays again.
// - if only 2 shapes are showing, the rules for 2 players are applied and 
//   players holding the winning gesture stay in the game.
//
// More info at https://en.wikipedia.org/wiki/Rock_paper_scissors
//

#include <MD_MAX72xx.h>

#define DEBUG 0

#if DEBUG
#define PRINT(s, v)  do { Serial.print(F(s)); Serial.print(v); } while (false);
#define PRINTX(s, v) do { Serial.print(F(s)); Serial.print(v, HEX); } while (false);
#define PRINTS(s)    do { Serial.print(F(s)); } while (false);
#else
#define PRINT(s, v)
#define PRINTS(s)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

// Maximum number of players allowed for in the game.
// Used to size many items to max number required.
const uint8_t MAX_PLAYERS = 4;

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

const uint8_t CLK_PIN = 13;  // or SCK
const uint8_t DATA_PIN = 11; // or MOSI
const uint8_t CS_PIN = 10;   // or SS

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_PLAYERS);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Switch definitions - array indexes for user pin arrays and 
// player selection. Note these switches are not debounced as we are 
// only interested in the fact the switch has become active. After that 
// the user is locked out to further input until the next run of the game.
// Actual pin numbers are defined in the static initialization of the 
// player data (P array).
const uint8_t MAX_SWITCH = 3; // one switch each for R/P/S

const uint8_t SW_RCK = 0;   // Rock
const uint8_t SW_PPR = 1;   // Paper
const uint8_t SW_SCR = 2;   // Scissors

const uint8_t SW_NULL = 255;        // no pin/no selection
const uint8_t SW_ACTIVE = LOW;      // switch active signal state

// Define the game symbols as font characters
// Each is defined to fit into one LED module (8x8)
MD_MAX72XX::fontType_t RPSSymbols[] PROGMEM =
{
  'F', 1, 0, 13, 8,
  8, 0,0,0,0,0,0,0,0,                //  0 - Empty Cell
  8, 0,4,254,0,0,240,80,32,          //  1 - 1P (1 Player sel)
  8, 242,146,146,158,0,240,80,32,    //  2 - 2P (2 Player sel)
  8, 146,146,146,254,0,240,80,32,    //  3 - 3P (3 Player sel)
  8, 30,16,16,254,0,240,80,32,       //  4 - 4P (4 Player sel)
  8, 0,66,36,24,24,36,66,0,          //  5 - Cross 
  8, 0,126,66,66,66,66,126,0,		     //  6 - Square
  8, 0,102,153,69,145,202,34,28,     //  7 - Rock glyph
  8, 0,255,129,129,143,138,140,248,  //  8 - Paper glyph
  8, 56,40,56,8,255,168,232,8,       //  9 - Scissors glyph
  8, 255,129,129,129,129,129,129,255,// 10 - Ready/Set/Go seq 0
  8, 0,126,66,66,66,66,126,0,        // 11 - R/S/G seq 1
  8, 0,0,60,36,36,60,0,0,            // 12 - R/S/G seq 2
  8, 0,0,0,24,24,0,0,0,              // 13 - R/S/G seq 3
};

// Character definitions (names for char index in RPSSymbols)
const uint8_t CH_EMPTY = 0;
const uint8_t CH_P_BASE = 1;
const uint8_t CH_X = 5;
const uint8_t CH_O = 6;
const uint8_t CH_RCK = 7;
const uint8_t CH_PPR = 8;
const uint8_t CH_SCR = 9;
const uint8_t CH_RSG_BASE = 10;

// Structure to hold the data for each player.
// Includes the current state of play, user interface (like module number, 
// switches), etc
typedef enum:uint8_t { S_NUL, S_OUT, S_WIN, S_DRAW, S_LOSE } plyrState_t;

typedef struct
{
  plyrState_t state;        // player running state

  // Associated hardware
  uint8_t module;           // LED module number for this player
  uint8_t swPin[MAX_SWITCH];// RPS switches per user 
  
  // Game management
  bool autoPlayer;          // player is computer
  bool enabled;             // player is enabled
  uint8_t curSel;           // players current selection (SW_*)
  uint8_t lastKey;          // last key pressed (SW_*)

  // Display info
  bool flash;               // flash (blank out) this displayed symbol
  uint8_t curSymbol;        // current symbol displayed for player
}  playerData_t;

playerData_t P[MAX_PLAYERS] = // initialized with static data
{
  { S_NUL, 0, {  2,  3,  4 }, false, true, SW_NULL, SW_NULL, false, CH_P_BASE },
  { S_NUL, 1, {  5,  6,  7 }, false, true, SW_NULL, SW_NULL, false, CH_EMPTY },
  { S_NUL, 2, {  8,  9, A0 }, false, true, SW_NULL, SW_NULL, false, CH_EMPTY },
  { S_NUL, 3, { A1, A2, A3 }, false, true, SW_NULL, SW_NULL, false, CH_EMPTY },
};

// Other global data
uint8_t numPlayers = 1;   // assume only 1 human player to start with

#if DEBUG
const char* choice2name(uint8_t choice)
{
  switch (choice)
  {
  case SW_RCK: return("SW_RCK");
  case SW_PPR: return("SW_PPR");
  case SW_SCR: return("SW_SCR");
  case SW_NULL: return("SW_NULL");
  default: return("SW_??");
  }
}
#endif

void updateDisplay(void)
// Do the necessary to set up and display current 
// bitmap buffer to the LED display
{
  const uint8_t MAX_COL = 8;
  uint8_t buf[MAX_COL];

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  for (uint8_t i = 0; i < MAX_PLAYERS; i++)
  {
    mx.clear(P[i].module);

    if (!P[i].flash)
    {
      uint8_t cols = mx.getChar(P[i].curSymbol, MAX_COL, buf);

      for (uint8_t j = 0; j < cols; j++)
        mx.setColumn(P[i].module, j, buf[cols - j - 1]);
    }
  }

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void checkWinnerSimple(uint8_t p1, uint8_t p2)
// Check for a winner between 2 players p1 and p2 and set their
// status as appropriate.
{
  PRINT("\nCheck Simple P", p1);
  PRINT(" P", p2);
  if (P[p1].curSel == P[p2].curSel)
  {
    P[p1].state = P[p2].state = S_DRAW;
    PRINTS(" - draw");
  }
  else
  {
    if ((P[p1].curSel == SW_RCK && P[p2].curSel == SW_PPR)
      || (P[p1].curSel == SW_PPR && P[p2].curSel == SW_SCR)
      || (P[p1].curSel == SW_SCR && P[p2].curSel == SW_RCK))
    {
      P[p1].state = S_LOSE;
      P[p2].state = S_WIN;
      PRINT(" - win P", p2);
    }
    else
    {
      P[p1].state = S_WIN;
      P[p2].state = S_LOSE;
      PRINT(" - win P", p1);
    }
  }
}

void checkWinnerMulti(void)
// Check for a winner between all the players who are currently
// in the game and set their status as appropriate.
{
  uint8_t count[MAX_SWITCH];

  PRINTS("\nCheck Multi - ");

  // initialize the counts
  for (uint8_t i = 0; i < MAX_SWITCH; i++)
    count[i] = 0;

  // first get the counts of all the player choices
  for (uint8_t i=0; i < numPlayers; i++)
    if (P[i].state != S_OUT)
      count[P[i].curSel]++;

  // Scan all the players in the game and collect info for 
  // decision making.
  uint8_t hasCount = 0;      // how many categories have a count
  uint8_t tChoice = SW_NULL; // temp choice holder

  for (uint8_t i = 0; i < MAX_SWITCH; i++)
  {
    if (count[i] != 0) hasCount++;
    if (count[i] == 0) tChoice = i;
  }

  // if only one category has a count (ie all choices identical), 
  // or if all have counts (ie all choices are represented)
  // we have a DRAW.
  if (hasCount == 1 || hasCount == MAX_SWITCH)
  {
    PRINT("DRAW result: hasCount = ", hasCount);
    for (uint8_t i = 0; i < numPlayers; i++)
      if (P[i].state != S_OUT) P[i].state = S_DRAW;
  }
  else
  {
    // Now we know we have a result, and which count is a zero
    // so we can work out which one loses and which wins and set the 
    // player state accordingly.
    if (tChoice == SW_RCK)       // PPR and SCR are in play
      tChoice = SW_PPR;
    else if (tChoice == SW_PPR)  // RCK and SCR are in play
      tChoice = SW_SCR;
    else                         // RCK and PPR are in play
      tChoice = SW_RCK;
          
    PRINT("LOSE result: loseChoice = ", choice2name(tChoice));
    for (uint8_t i = 0; i < numPlayers; i++)
    {
      if (P[i].state != S_OUT)
        P[i].state = (P[i].curSel == tChoice) ? S_LOSE : S_WIN;
    }
  }
}

uint8_t readInput(uint8_t player)
// read the input from a player's switch bank
{
  uint8_t id = SW_NULL;

  for (uint8_t i = 0; i < MAX_SWITCH; i++)
  {
    if (digitalRead(P[player].swPin[i]) == SW_ACTIVE)
    {
      id = i;
      break;
    }
  }

  return(id);
}

void setTurn(uint8_t player, uint8_t selection)
// Utility function to set consistent selection and display glyph
{
  P[player].curSel = selection;
  switch (P[player].curSel)
  {
  case SW_RCK: P[player].curSymbol = CH_RCK; break;
  case SW_PPR: P[player].curSymbol = CH_PPR; break;
  case SW_SCR: P[player].curSymbol = CH_SCR; break;
  case SW_NULL: P[player].curSymbol = CH_EMPTY; break;
  default: P[player].curSymbol = CH_RSG_BASE; break;
  }
}

bool getNumPlayers(uint8_t &tPlayer)
// Input the number of players at the start of the session.
// The keys for P[0] are used to set the number of players.
// The SW_RCK switch is used to roll between the number of
// players, SW_PPR and SW_SCR will exit this setting mode.
// Returns true and set the global numPlayers when completed.
{
  bool b = false;
  uint8_t sw = readInput(0);

  if (sw != SW_NULL)
  {
    PRINT("\nChecking id ", sw);
    switch (sw)
    {
    case SW_RCK:
      tPlayer++;
      if (tPlayer > MAX_PLAYERS) tPlayer = 1;
      PRINT("\ntPlayer update ", tPlayer);
      P[0].curSymbol = CH_P_BASE + (tPlayer - 1);
      updateDisplay();
      delay(100);    // rudimentary debounce here!!
      break;

    case SW_PPR:
    case SW_SCR:
      b = true;
      numPlayers = tPlayer;
      PRINT("\nnumPlayers set ", numPlayers);
      P[0].curSymbol = CH_EMPTY;
      break;
    }
  }
  return(b);
}

void setup(void)
{
#if DEBUG
  Serial.begin(57600);
#endif // DEBUG
  PRINTS("\n[MD_MAX72xx Rock/Paper/Scissors]")

  // Matrix initialization
  mx.begin();
  mx.setFont(RPSSymbols);
  mx.clear();

  // Switch initialization
  for (uint8_t i = 0; i < MAX_PLAYERS; i++)
    for (uint8_t j = 0; j < MAX_SWITCH; j++)
      pinMode(P[i].swPin[j], SW_ACTIVE == LOW ? INPUT_PULLUP : INPUT);
}

void loop(void)
{
  // States for the execution progress.
  static enum:uint8_t 
  { 
    S_I_SESSION,      // initialize a new session (once only)
    S_I_GAME,         // initialize a new game (each game)
    S_I_RUN,          // initialize a new run (each run in a game)
    S_RSG,            // Ready/Set/Go display
    S_CPU_TURN,       // Each computer player has their selection if playing
    S_WAIT_PLYR,      // Wait for all other players to do their thing
    S_CHECK_RESULT,   // check the result for the run
    S_WINNER,         // show the winner(s) for the run
    S_END,            // end the run and decide what to do next
  } state = S_I_SESSION;

  static uint32_t timeLast = 0;   // generic baseline for millis() timing
  static uint8_t counter = 0;     // generic counter variable

  // Run the switch detection for all players in the run
  for (uint8_t i = 0; i < numPlayers; i++)
    if (P[i].state != S_OUT) 
      P[i].lastKey = readInput(i);

  // Run the game as a finite state machine
  switch (state)
  {
  case S_I_SESSION:      // initialize a new session (once only)
    if (counter == 0)    // first time through
    {
      PRINTS("\n** INIT SESSION");
      counter = numPlayers;
      updateDisplay();
    }

    if (getNumPlayers(counter)) 
    {
      numPlayers = counter;

      // now we have the number of players, work out what to do next
      if (numPlayers == 1)      // the other player must be the the computer
      {
        numPlayers = 2;
        P[1].autoPlayer = true;
      }

      // Disable the players not being used and update the display
      for (uint8_t i = numPlayers; i < MAX_PLAYERS; i++)
      {
        P[i].state = S_OUT;
        P[i].enabled = false;
        P[i].curSymbol = CH_EMPTY;
      }
      updateDisplay();

      // move on to next state
      state = S_I_GAME;
    }
    break;

  case S_I_GAME:         // initialize a new game (each game)
    PRINTS("\n** INIT GAME");
    for (uint8_t i = 0; i < numPlayers; i++)
      P[i].state = S_NUL;

    randomSeed(millis());// should be good as user timing is unpredictable

    // move on to next state
    state = S_I_RUN;
    break;

  case S_I_RUN:          // initialize a new run (multiple runs in a game)
    PRINTS("\n** INIT RUN");
    for (uint8_t i = 0; i < numPlayers; i++)
    {
      if (P[i].state != S_OUT)
      {
        P[i].lastKey = SW_NULL;
        P[i].state = S_NUL;
        P[i].flash = false;
        setTurn(i, SW_NULL);
      }
      else
      {
        P[i].curSymbol = CH_X;
      }
    }

    // move on to next state
    timeLast = millis();
    counter = 0;
    state = S_RSG;
    break;

  case S_RSG:            // Ready/Set/Go display
    if (millis() - timeLast >= 500)
    {
      if (counter == 0) PRINTS("\n** RDY_SET_GO");
      PRINT(" ", counter);
      for (uint8_t i = 0; i < numPlayers; i++)
        if (P[i].state != S_OUT)
          P[i].curSymbol = CH_RSG_BASE + counter;

      updateDisplay();

      // prepare for next cycle
      timeLast = millis();
      counter++;

      // if finished, move on to next state
      if (counter >= 4) 
        state = S_CPU_TURN;
    }
    break;

  case S_CPU_TURN:       // Each computer player has their selection if playing
    PRINTS("\n** CPU TURN");
    for (uint8_t i = 0; i < numPlayers; i++)
    {
      if (P[i].autoPlayer)
      {
        PRINT("\nComputer player ", i);
        P[i].curSel = random(120);
        if (P[i].curSel > 80)      P[i].curSel = SW_SCR;
        else if (P[i].curSel > 40) P[i].curSel = SW_PPR;
        else                       P[i].curSel = SW_RCK;
        PRINT(" selected ", choice2name(P[i].curSel));
        P[i].curSymbol = CH_O;
      }
    }
    updateDisplay();

    // move on to next state
    state = S_WAIT_PLYR;
    break;

  case S_WAIT_PLYR:      // Wait for all other players to do their thing
    // see if anyone has pressed a key
    // we remain in this state until everyone has made their selection
    for (uint8_t i = 0; i < numPlayers; i++)
    {
      if (P[i].curSel == SW_NULL && P[i].lastKey != SW_NULL)
      {
        PRINT("\nHuman player ", i);
        P[i].curSel = P[i].lastKey;
        PRINT(" selected ", choice2name(P[i].curSel));
        P[i].curSymbol = CH_O;
        updateDisplay();
      }
    }

    // check if we are all done
    counter = 0;
    for (uint8_t i = 0; i < numPlayers; i++)
    {
      if (P[i].curSel != SW_NULL)
        counter++;
    }

    if (counter == numPlayers)
    {
      PRINTS("\nAll turns complete");
      for (uint8_t i = 0; i < numPlayers; i++)
        if (P[i].state != S_OUT) 
          setTurn(i, P[i].curSel);
      updateDisplay();

      // move on to next state
      state = S_CHECK_RESULT;
    }
    break;

  case S_CHECK_RESULT:   // check the result for the run
    {
      uint8_t p[2] = { 0, 0 };

      PRINTS("\n** CHECK RESULT");
      // collect some info to work out which method of
      // winner determination applies to this run.
      counter = 0;
      PRINTS("\nIn ->");
      for (uint8_t i = 0; i < numPlayers; i++)
      {
        if (P[i].state != S_OUT)
        {
          PRINT(" P", i);
          counter++;
          p[1] = p[0];
          p[0] = i;
        }
      }

      if (counter == 2)
        checkWinnerSimple(p[0], p[1]);
      else
        checkWinnerMulti();

      // set up and move to next state
      counter = 0;
      timeLast = millis();
      state = S_WINNER;
    }
    break;

  case S_WINNER:         // show the winner(s) for the run
    // winners are indicated by a flashing glyph
    if (millis() - timeLast >= 500)
    {
      for (uint8_t i = 0; i < numPlayers; i++)
        if (P[i].state == S_WIN)
          P[i].flash = !P[i].flash;
      updateDisplay();
      counter++;
      timeLast = millis();
    }

    // when done, move on to next state
    if (counter == 6)
    {
      counter = 0;
      state = S_END;
    }
    break;

  case S_END:            // end the run and decide what to do next
    PRINTS("\n** END");
    for (uint8_t i = 0; i < numPlayers; i++)
    {
      if (P[i].state == S_LOSE)   // anyone who lost is now out of the game
      {
        PRINT("\n -> out P", i);
        P[i].state = S_OUT;
      }
      if (P[i].state == S_OUT)    // anyone out needs to be shown as such
        P[i].curSymbol = CH_X;
      if (P[i].state != S_OUT)    // anyone not out is still in for the next run
        counter++;
    }

    PRINT("\nPlayers remaining ", counter);
    if (counter > 1)
    {
      state = S_I_RUN;  // multiple winners, do another run
    }
    else
      state = S_I_GAME; // only one winner, restart the game 
    break;

  default:
    state = S_I_SESSION;
    break;
  }
}

