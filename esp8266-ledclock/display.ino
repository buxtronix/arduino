
// ======== Display constants.
#define _A_ 0x80
#define _B_ 0x40
#define _C_ 0x20
#define _D_ 0x10
#define _E_ 0x08
#define _F_ 0x04
#define _G_ 0x02
#define _DP_ 0x01


#define DATA 0
#define CLOCK 1
#define BLANK 2

const char segments[] = {
  _A_|_B_|_C_|_D_|_E_|_F_,    // 0
  _B_|_C_,            // 1
  _A_|_B_|_G_|_D_|_E_,      // 2
  _A_|_B_|_C_|_D_|_G_,      // 3
  _B_|_C_|_F_|_G_,        // 4
  _A_|_C_|_D_|_F_|_G_,      // 5
  _A_|_C_|_D_|_E_|_F_|_G_,    // 6
  _A_|_B_|_C_,          // 7
  _A_|_B_|_C_|_D_|_E_|_F_|_G_,  // 8
  _A_|_B_|_C_|_D_|_F_|_G_,    // 9

  _A_|_B_|_C_|_E_|_F_|_G_, // A
  _C_|_D_|_E_|_F_|_G_, // b
  _D_|_E_|_G_, // c
  _B_|_C_|_D_|_E_|_G_, // d
  _A_|_D_|_E_|_F_|_G_, //e
  _A_|_E_|_F_|_G_, // f

  _A_|_B_|_E_|_F_|_G_, // P [0x10]

  _G_, // - [0x11]

  _A_, _B_, _C_, _D_, _E_, _F_, // 0x12 - 0x17

  0, // 0x18
};
  
char digits[4];
char decimals;

#define PULSE digitalWrite(CLOCK, LOW); delayMicroseconds(10) ; digitalWrite(CLOCK, HIGH); delayMicroseconds(10) ;

void clear() {
  char i;
  digitalWrite(DATA, LOW);
  for (i = 0 ; i < 32 ; i++) {
    PULSE;
  }
}

void display() {
  char i, d, digit;

  digitalWrite(BLANK, LOW);

  clear();
  for (d = 0 ; d < 4 ; d++) {
    digit = segments[digits[d]];

    if ((decimals >> d & 0x1) == 0x1) digit |= _DP_;

    for (i = 0 ; i < 8 ; i++) {
      digitalWrite(DATA, digit & 0x1 ? HIGH : LOW);
      digit >>= 1;
      PULSE;
    }
  }
  digitalWrite(BLANK, HIGH);
}

void displayAP() {
  digits[0] = 0x10;
  digits[1] = 0xA;
  digits[2] = 0x18;
  digits[3] = 0x18;
  display();
}

void displayDash() {
  digits[0] = digits[1] = digits[2] = digits[3] = 0x11;
  display();
}

void clearDigits() {
  digits[0] = digits[1] = digits[2] = digits[3] = 0x18;
}

// Twirler handler.
Ticker ticker;

volatile char busySegment = 0x12;
volatile char busyDigit;

void displayBusy(char digit) {
  busyDigit = digit;
  ticker.attach(0.1, _displayBusy);
}

void stopDisplayBusy() {
  ticker.detach();
}

void _displayBusy() {
  if (busySegment > 0x17) {
    busySegment = 0x12;
  }
  clearDigits();
  digits[busyDigit] = busySegment++;
  display();
}

// End twirler handler.

// IP Display handler.
volatile signed char dispOctet = -1;

char displayIP() {
  if (dispOctet > -1) {
    return 1;
  }
  if (digitalRead(SETUP_PIN) == 1) return 0;
  dispOctet = 0;
  ticker.attach(1.0, _displayIP);
  return 0;
}

void _displayIP() {
  if (dispOctet > 3) {
    ticker.detach();
    dispOctet = -1;
    clockMode == MODE_CLOCK ? displayClock() : displayAP();
    return;
  }
  clearDigits();
  uint8_t octet = (uint32_t(clockMode == MODE_CLOCK ? WiFi.localIP() : WiFi.softAPIP()) >> (8 * dispOctet++)) & 0xff;
  uint8_t d = 0;
  for (; octet > 99 ; octet -= 100) d++;
  digits[2] = d;
  d = 0;
  for (; octet > 9 ; octet -= 10) d++;
  digits[1] = d;
  digits[0] = octet;
  decimals = 0x1;
  display();
}

// end Ip display handler.

void displayClock() {
  int h = hour();
  int m = minute();
  digits[0] = digits[1] = digits[2] = digits[3] = decimals = 0;

  if (h > 19) digits[3] = 2;
  else if (h > 9) digits[3] = 1;
  digits[2] = h % 10;

  if (m > 49) digits[1] = 5;
  else if (m > 39) digits[1] = 4;
  else if (m > 29) digits[1] = 3;
  else if (m > 19) digits[1] = 2;
  else if (m > 9) digits[1] = 1;

  digits[0] = m % 10;

  if (second() & 0x1) decimals = 0x4;
  if (timeStatus() != timeSet) decimals |= 0x1;
  display();
}

void setupDisplay() {
  pinMode(DATA, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(BLANK, OUTPUT);
  displayDash();
}

