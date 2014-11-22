/*
 * Demonstrates use of the Cli library.
 *
 * The print function here is just the built in Serial module, so user i/o
 * is via this.
 */
#include <Cli.h>

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

void setup() {
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
  Serial.print("Millis: ");
  Serial.println(millis());
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
  Serial.print("Sleeping for ");
  Serial.print(value);
  Serial.println(" millis...");
  delay(value);
}

// Pin write function.
void write_func() {
  char pin = cli->getArgi(1);
  char value = cli->getArgi(2);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, value);
  Serial.print("Wrote ");
  Serial.print((int)value);
  Serial.print(" to pin ");
  Serial.println((int)pin);
}

// Pin read function.
void read_func() {
  char pin = cli->getArgi(1);
  Serial.print("Value on pin ");
  Serial.print((int)pin);
  Serial.print(": ");
  pinMode(pin, INPUT);
  Serial.println(digitalRead(pin));
}

// Analog write function.
void awrite_func() {
  char pin = cli->getArgi(1);
  int value = cli->getArgi(2);
  pinMode(pin, OUTPUT);
  analogWrite(pin, value);
  Serial.print("Wrote pwm ");
  Serial.print((int)value);
  Serial.print(" to pin ");
  Serial.println((int)pin);
}

// Analog read function.
void aread_func() {
  char pin = cli->getArgi(1);
  Serial.print("Value on pin ");
  Serial.print((int)pin);
  Serial.print(": ");
  Serial.println(analogRead(pin));
}
