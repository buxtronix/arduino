/*
 * State machine for processing Manchester and Biphase encoded
 * data.
 *
 * Your code needs to decode the length and polarity (Manchester only)
 * of each 'event' in the bitstream. Then this short library will 
 * convert those to 0s and 1s.
 *
 * Simple call 'm_init(<encoding>)' at the start, then m_advance() with
 * an argument defining what the even is (see header file for events).
 * m_advance() will then return one of the results defined in the header.
 *
 */
#include <avr/io.h>
#include <manchester.h>

// States
#define S_START1 0x0
#define S_MID1 0x1
#define S_MID0 0x2
#define S_START0 0x3
#define S_BEGIN 0x4

const unsigned char Manchester[4][4] = {
  // Short space, Short pulse, Long space, Long pulse
  // START1
  {S_MID1|S_EMIT1, S_START1|S_ERROR, S_START1|S_ERROR, S_START1|S_ERROR },
  // MID1
  {S_START1|S_ERROR, S_START1, S_START1|S_ERROR, S_MID0|S_EMIT0 },
  // MID0
  {S_START0, S_START1|S_ERROR, S_MID1|S_EMIT1, S_START1|S_ERROR },
  // START0
  {S_START1|S_ERROR, S_MID0|S_EMIT0, S_START1|S_ERROR, S_START1|S_ERROR },
};

const unsigned char BiPhase1[3][2] = {
  // Short event, long event.
  {S_MID1, S_START1|S_EMIT1},
  {S_START1|S_EMIT0, S_START1|S_ERROR},
  {S_START1|S_EMIT1},
};

const unsigned char BiPhase2[4][4] = {
  // Short event, long event.
  {S_MID1, S_BEGIN|S_ERROR},
  {S_START1|S_EMIT1, S_BEGIN|S_EMIT0},
  {S_START1|S_EMIT1},
};


static unsigned char state;
static unsigned char encoding;

void m_init(char enc) {
  encoding = enc;
  if (encoding == MANCHESTER)
    state = S_MID1;
  else
    state = S_START1;
}

unsigned char m_advance(unsigned char event) {
  unsigned char newState;
  if (encoding == MANCHESTER)
    newState = Manchester[state][event];
  else if (encoding == BIPHASE1)
    newState = BiPhase1[state][event];
  else if (encoding == BIPHASE2)
    newState = BiPhase2[state][event];
  state = newState & 0x0f;
  return newState & 0xf0;
}

