
#ifndef __MANCHESTER_H__
#define __MANCHESTER_H__

// Supported schemes, used in the arg of 'm_init'.
#define MANCHESTER 0
#define BIPHASE1 1  // Midpoint transition for 0.
#define BIPHASE2 2  // Midpoint transition for 1.

// Results
#define S_EMIT0 0x10 // Emit a 0 bit.
#define S_EMIT1 0x20 // Emit a 1 bit.
#define S_ERROR 0x40 // Error condition.

// Events
#define EV_SS 0x0  // Short space
#define EV_SP 0x1  // Short pulse
#define EV_LS 0x2  // Long space
#define EV_LP 0x3  // Long pulse
#define EV_00 0x4

// For Biphase, polarity is irrelevant, just short or long.
#define EV_S 0x0  // Short event.
#define EV_L 0x1  // Long event.

void m_init(char encoding);
unsigned char m_advance(unsigned char);
#endif
