/* Cli library for Arduino */

#include "Arduino.h"
#include "Cli.h"
#include <string.h>


Cli::Cli(const char *_prompt, Print *pf) {
  _clear();
  prompt = _prompt;
  printfunc = pf;
  printfunc->write(prompt);
}

// Clear the CLI buffer
void Cli::_clear() {
  _cliBufferLength = 0;
  memset(_cliBuffer, 0, BUFFER_LENGTH);
  memset(_commandBuffer, 0, COMMAND_LENGTH);
}

void Cli::addChar(char c) {
  if (c == 0xD || c == ';') {
    printfunc->print("\r\n");
    _parseAndRun();
    return;
  }
  if (c == 0x8 || c == 0x7F) {
    if (_cliBufferLength > 0) {
      _cliBuffer[--_cliBufferLength] = '\0';
      printfunc->print("\010 \010");
    }
    return;
  }
  _cliBuffer[_cliBufferLength++] = c;
  if (_cliBufferLength >= BUFFER_LENGTH)
    _cliBufferLength = BUFFER_LENGTH-1;
  else
    printfunc->write(c);
}

char Cli::_strncmp(const char *s1, const char *s2, char n) {
const char *idx1 = s1;
const char *idx2 = s2;
char numindex = 0;

  while (idx1 != 0 && idx2 != 0 && numindex < n) {
    if (*idx1 != *idx2)
      return 0;
    idx1++; idx2++; numindex++;
  }
  return 1;
}

unsigned char Cli::_strlen(const char *s) {
unsigned char len = 0;
  for ( ; *s != '\0' ; s++)
    len++;
  return len;
}

int Cli::atoi(char *s) {
  int value = 0;
  int mult = 1;  
  char *end = &s[_strlen(s)];
  while (end != s) {
    end--;
    if (*end > 0x39 || *end < 0x30) break;
    value += (*end - 0x30)*mult;
    mult *= 10;
  }
  if (*end == '-') {
    value *= -1;
  }
  return value;
}

// Returns a pointer to the arguments on the command line.
char *Cli::getArg() {
char *ptr;
  ptr = _cliBuffer;
  while (*ptr != '\0' && *ptr != ' ')
    *ptr++;
  if (*ptr == ' ')
    *ptr++;
  return ptr;
}

// Returns the nth arg on the command line (0 is the command).
char *Cli::getArg(char n) {
char *ptr;

  ptr = _cliBuffer;
  for (char i = 0 ; i < _cliBufferLength ; i++) {
    if (n == 0) break;
    if (*ptr == '\0') break;
    if (*ptr == ' ') n--;
    *ptr++;
  }
  memset(_argBuffer, '\0', COMMAND_LENGTH);
  for (unsigned char i = 0 ; i < COMMAND_LENGTH ; i++) {
    _argBuffer[i] = *ptr++;
    if (*ptr == '\0' || *ptr == ' ') break;
  }
  return _argBuffer;
}

// Returns the nth arg on the command line parsed as an int (0 is the command).
int Cli::getArgi(char n) {
  char *arg = getArg(n);
  return atoi(arg);
}

// Displays help - all of the available commands.
void Cli::help() {
    printfunc->println("Command help:");
    for (unsigned char i = 0 ; i < ncommands ; i++) {
      printfunc->write(' ');
      printfunc->write(commands[i].name);
      printfunc->write("  ");
      printfunc->println(commands[i].help);
    }
    printfunc->write(prompt);
    _clear();
    return;
}

void Cli::_parseAndRun() {
unsigned char i;
  if (_cliBufferLength == 0) {
    // Blank line
    printfunc->print(prompt);
    return;
  }
  if (_strlen(_cliBuffer) == 1 && _strncmp("?", _cliBuffer, 1)) {
    help();
    return;
  }

  for (i = 0 ; i < _cliBufferLength ; i++)
    if (_cliBuffer[i] != '\0' && _cliBuffer[i] != ' ')
      _commandBuffer[i] = _cliBuffer[i];
    else
      break;
  for (i = 0 ; i < ncommands ; i++) {
    Command c = commands[i];
    if (_strlen(_commandBuffer) == _strlen(c.name) &&
        _strncmp(_commandBuffer, c.name, _strlen(c.name))) {
      runFunc(c);
      _clear();
      printfunc->print(prompt);
      return;
    }
  }
  
  _clear();
  printfunc->println("Unknown command. '?' for help.");
  printfunc->print(prompt);
}

// Runs the command, if the number of args matches.
void Cli::runFunc(Command c) {
  if (c.args > 0 && getArg(c.args)[0] == '\0') {
    printfunc->print("Command '");
    printfunc->print(c.name);
    printfunc->print("' requires ");
    printfunc->print((int)c.args);
    printfunc->println(" arguments.");
    return;
  }
  c.run();
}
