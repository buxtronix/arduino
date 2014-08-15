#include <cli.h>

#include <SPI.h>
#include "font.h"

/*
 * Controls a SPI based LED matrix board. Paltronics PAL0103 specifically.
 *
 * Board pinouts:
 *  (odd pins) GND
 *  2 - Serial in, red
 *  4 - Serial in, green
 *  6 - Clock
 *  8 - Latch (falling edge)
 * 10 - Row bit1
 * 12 - Row bit2
 * 14 - Row bit3
 * 16 - Enable (bar)
 */

#define XSIZE 64
#define YSIZE 8

#define ENABLE 5
#define LATCH 10

#define RED 0x1
#define GREEN 0x2
#define ORANGE RED|GREEN

// Current displayed row number.
byte row;

// The red and green display buffer.
byte reds[64];
byte greens[64];

#define MAXTEXT 64
char text[MAXTEXT];  // Text buffer to display.
char scroll;    // Speed to scroll at.
int scrollpos;  // Current text position.
char vpos;     // Vertical position of text.

Cli *cli;

Command cmds[] = {
  {"text", "Set displayed text", &cmd_text},
  {"scroll", "Scroll speed", &cmd_scroll},
  {"bitmap", "Set bitmap (512b)", &cmd_bitmap},
  {"pos", "Set text position", &cmd_pos},
  {"vpos", "Set vertical position", &cmd_vpos},
};

char ncommands = 5;

static long stamp;
static char ticks;

//  Called at 1000Hz to display each row.
ISR(TIMER1_OVF_vect) {
  writeData(row);
  row ++;
  if (row > 0x7) {
    row = 0;
  }
  if (++ticks > 9) {
    tick();
    ticks = 0;
  }
}

// Write the given row of data to the SPI.
void writeData(byte row) {
 digitalWrite(ENABLE, HIGH);
 PORTC &= 0xf0;
 PORTC |= row;
 // red
 for (char i = 7 ; i >=0 ; i--) {
   SPI.transfer(reds[row*8 + i]);
 }
 // green
 for (char i = 7 ; i >=0 ; i--) {
   SPI.transfer(greens[row*8 + i]);
 }
 digitalWrite(LATCH, HIGH);
 digitalWrite(LATCH, LOW);
 digitalWrite(ENABLE, LOW);
}

// COMMAND: Enter a line of text to display.
void cmd_text() {
  char *ttmp = new char[MAXTEXT];
  char *tptr = ttmp;
  memset(ttmp, 0, MAXTEXT);
  Serial.println("Enter text, terminate with a newline:");
  while (1) {
    if (!Serial.available()) continue;
    char c = Serial.read();
    Serial.write(c);
    if (c == 0xD) {
      Serial.println();
      break;
    }
    *tptr = c;
    tptr++;
    if (strlen(text) >= MAXTEXT) {
      Serial.print("Maximum text read.");
      break;
    }
  }
  Serial.println("Text read complete.");
  memcpy(text, ttmp, MAXTEXT);
  delete[] ttmp;
  all(0);
  write(scrollpos, vpos, RED, text);
}

// COMMAND: Set the speed to scroll the text.
void cmd_scroll() {
  scroll = cli->atoi(cli->getArg(1));
  Serial.print("Set scroll speed to: ");
  Serial.println(int(scroll));
}

// COMMAND: Enter a 512-bit map to display.
void cmd_bitmap() {
  byte *nreds = new byte[XSIZE*YSIZE/8];
  byte *ngreens = new byte[XSIZE*YSIZE/8];
  Serial.print("Reading 128 bytes of bitmap data.");
  for (byte i = 0 ; i < 64 ; i++) {
    while (!Serial.available());
      nreds[i] = Serial.read();
  }
  for (byte i = 0 ; i < 64 ; i++) {
    while (!Serial.available());
      ngreens[i] = Serial.read();
  }
  memcpy(reds, nreds, XSIZE*YSIZE/8);
  memcpy(greens, ngreens, XSIZE*YSIZE/8);
  delete[] nreds;
  delete[] ngreens;
  Serial.println("Done.");
}

void setup() {
  // Set latch to output, default low.
  pinMode(LATCH, OUTPUT);
  digitalWrite(LATCH, LOW);
  
  // Set row select pins as outputs.
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  // Setup SPI interface.
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);
  initTimer(1000);
  Serial.begin(57600);
  
  memset(text, 0, MAXTEXT);
  write(0, 0, 0, ' ');
  
  cli = new Cli("arduino> ", &Serial);

  cli->commands = cmds;
  cli->ncommands = ncommands;
  Serial.println("LedBoard SPI edition.");
}

// Called 100x / sec
void tick() {
  static char scrollticks = 0;
  if ((scroll > 0 && scrollticks++ >= scroll) ||
      (scroll < 0 && scrollticks++ >= (0-scroll))) {
        do_scroll();
        scrollticks = 0;
  }
}

// COMMAND: Set the absolute horizontal scroll position.
void cmd_pos() {
  scrollpos = cli->atoi(cli->getArg(1));
  Serial.print("Set position to: ");
  Serial.println(scrollpos);
  all(0);
  write(scrollpos, vpos, RED, text);
}

// COMMAND: Set the absolute vertical scroll position.
void cmd_vpos() {
  vpos = cli->atoi(cli->getArg(1));
  Serial.print("Set vposition to: ");
  Serial.println(vpos);
  all(0);
  write(scrollpos, vpos, RED, text);
}

// Perform the scroll by shifting text.
void do_scroll() {
  if (scroll == 0) return;
  int textlen = 8*strlen(text);
  all(0);
  if (scroll < 0) {
    if (--scrollpos < 0-textlen) {
      scrollpos = XSIZE;
    }
  } else {
    if (++scrollpos > XSIZE) {
      scrollpos = 0 - textlen;
    }
  } 
  write(scrollpos, vpos, RED, text);
}

// Display a dot in a random position.
void randomdot() {
  byte dbyte = random(64);
  byte dbit = random(8);
  byte *b;
  if (random(2)) {
    b = &reds[dbyte];
  } else {
    b = &greens[dbyte];
  }
  if (*b >> dbit & 0x1) {
    *b &= ~(1 << dbit);
  } else {
    *b |= (1 << dbit);
  }
}

// Display the given block of data.
void block(int x, int y, byte colour, byte *data) {
  if (x > 63 || x < -7) return;
  if (y > 7 || y < -7) return;
  byte xbits;
  if (x >= 0) {
    xbits = x % 8;
  } else {
    xbits = (0-x) % 8;
  }
  for (int ypos = y ; ypos < y+8 ; ypos++) {
    int yrow = ypos*8;
    int crow = *data;
    if (ypos > 7) break;
    if (colour & RED) {
      if (x < 0) {
        reds[yrow] |= crow >> xbits;
      } else {
        reds[x/8 + yrow] |= crow << xbits;
        if (xbits && x < 56) reds[x/8+1 + yrow] |= crow >> (8-xbits);
      }
    }
    if (colour & GREEN) {
      if (x < 0) {
        greens[yrow] |= crow >> xbits;
      } else {
        greens[x/8 + yrow] |= crow << xbits;
        if (xbits && x < 56) greens[x/8+1 + yrow] |= crow >> (8-xbits);
      }
    } else if (!colour) {
      if (x < 0) {
        greens[yrow] &= (0xff ^ (crow >> xbits));
        reds[yrow] &= (0xff ^ (crow >> xbits));
      } else {
        greens[x/8 + yrow] &= (0xff ^ (crow << xbits));
        if (xbits && x < 56) greens[x/8+1 + yrow] &= (0xff ^ (crow >> (8-xbits)));
        reds[x/8 + yrow] &= (0xff ^ (crow << xbits));
        if (xbits && x < 56) reds[x/8+1 + yrow] &= (0xff ^ (crow >> (8-xbits)));
      }
    }
    data++; 
  }
  return;
}

// Set all pixels to the given colour.
void all(byte colour) {
  memset(reds, 0, 64);
  memset(greens, 0, 64);
  if (colour & RED) {
    memset(reds, 0xff, 64);
  }
  if (colour & GREEN) {
    memset(greens, 0xff, 64);
  }
}

// Write the given character.
void write(int x, int y, byte colour, char chr) {
  byte letterdata[8];  
  for (byte i = 0 ; i < 8 ; i++) {
    letterdata[i] = pgm_read_byte_near(font8x8_basic + chr*8 + i);
  }
  block(x, y, colour, letterdata);
  return;
}

// Write the given line of text.
void write(int x, int y, byte colour, char *str) {
  for (;*str != '\0' ; str++) {
     write(x, y, colour, *str);
     x += 8;
  } 
}

void loop() {
  if (Serial.available()) {
    cli->addChar(Serial.read());
  }
}

#define RESOLUTION 65536

void initTimer(long period) {
  TCCR1A = 0;
  TCCR1B = _BV(WGM13);
  unsigned char clockSelectBits;
  long cycles = (F_CPU / 2000000) * period;

  if(cycles < RESOLUTION)              clockSelectBits = _BV(CS10);
  else if((cycles >>= 3) < RESOLUTION) clockSelectBits = _BV(CS11);
  else if((cycles >>= 3) < RESOLUTION) clockSelectBits = _BV(CS11) | _BV(CS10);
  else if((cycles >>= 2) < RESOLUTION) clockSelectBits = _BV(CS12);
  else if((cycles >>= 2) < RESOLUTION) clockSelectBits = _BV(CS12) | _BV(CS10);
  else        cycles = RESOLUTION - 1, clockSelectBits = _BV(CS12) | _BV(CS10);

  char oldSREG = SREG;
  cli();
  ICR1 = cycles;
  SREG = oldSREG;

  TIMSK1 = _BV(TOIE1);
  TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
  TCCR1B |= clockSelectBits;
}

