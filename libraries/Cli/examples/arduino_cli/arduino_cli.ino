/*
 * Demonstrates use of the Cli library.
 *
 * The print function here is just the built in Serial module, so user i/o
 * is via this.
 */
#include <Cli.h>
#include <stdio.h>

// Setup for being able to use "printf".
static FILE uartout = {0};

// Initialise the Cli with the given prompt.
const char *prompt = "arduino> ";
Cli *cli;

/* This is where all of the commands are defined.
 * Each entry contains four parameters:
 * 1. The command that the user enters.
 * 2. The help string displayed to the user.
 * 3. Minimum number of arguments required.
 * 4. A pointer to the function called when the command is entered.
 */
Command commands[7] = {
  {"dl", "Sleep for given number of seconds", 1, &delay_func},
  {"ms", "Show Arduino millis() value.", 0, &millis_func},
  {"reset", "Reset the Arduino.", 0, &reset_func},
  {"dw", "Write a value to a digital pin. [pin] [0|1]", 2, &write_func},
  {"dr", "Read digital value from a pin.", 1, &read_func},
  {"aw", "Write a value to an analog pin. [pin] [0-255]", 2, &awrite_func},
  {"ar", "Read analogue value from a pin.", 1, &aread_func},
};
// Number of commands above.
char ncommands = 7;

// putchar method for "printf" (http://playground.arduino.cc/Main/Printf)
static int uart_putchar(char c, FILE *stream) {
  Serial.write(c);
  return 0;
}

void setup() {
  // Setup for printf to work.
  fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &uartout;
  
  // Initialise the serial port.
  Serial.begin(57600);
  // Initialise the Cli module with the commands defined above.
  cli  = new Cli(prompt, &Serial);
  cli->commands = commands;
  cli->ncommands = ncommands;
  // Show some initial help.
  cli->help();
}

void loop() {
  // Read characters from the serial port and pass them to the Cli module.
  if (Serial.available()) {
    cli->addChar(Serial.read());
  }
}

// Handler for the "millis" command.
void millis_func() {
  printf("Millis: %ld\r\n", millis());
}

// Handler for the "reset" command.
void(* resetFunc)(void) = 0;
void reset_func() {
  Serial.println("Resetting...");
  Serial.flush();
  resetFunc();
}

// Delay function.
void delay_func() {
  int value = cli->getArgi(1);
  printf("Sleeping for %d millis...\r\n", value);
  delay(value);
}

// Pin write function.
void write_func() {
  char pin = cli->getArgi(1);
  char value = cli->getArgi(2);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, value);
  printf("Wrote %d to pin %d\r\n", value, pin);
}

// Pin read function.
void read_func() {
  char pin = cli->getArgi(1);
  pinMode(pin, INPUT);
  printf("Value on pin %d: %d\r\n", pin, digitalRead(pin));
}

// Analog write function.
void awrite_func() {
  char pin = cli->getArgi(1);
  int value = cli->getArgi(2);
  pinMode(pin, OUTPUT);
  analogWrite(pin, value);
  printf("Write PWM %d to pin %d\r\n", value, pin);
}

// Analog read function.
void aread_func() {
  char pin = cli->getArgi(1);
  printf("Value on pin %d: %d\r\n", pin, analogRead(pin));
}
