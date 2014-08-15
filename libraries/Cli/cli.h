/*
 * Command line library for Arduino.
 *
 * This simple library allows you to add a CLI interface to your project.
 */

#ifndef cli_h
#define cli_h

#include "Arduino.h"
#include <avr/pgmspace.h>

#define COMMAND_LENGTH 10
#define BUFFER_LENGTH 64

typedef void (*CommandProc)();


/*
 * This defines a single command.
 */
struct Command {
  // The name of the command, what the user types to run it.
  const char *name;
  // Help text associated with the command.
  const char *help;
  // The function to run when the command is run.
  CommandProc run;
};

/*
 * The main CLI definition.
 */
class Cli
{
  public:
    /* Constructor. Args: the prompt string and a printer pointer */
    Cli(const char *, Print *);
    /* Adds an incoming character, e.g from serial port. */
    void addChar(char c);
    /* Used by the command's function to get the command argument at position n */
    char *getArg(char n);
    /* Fetch a pointer to the start of the command's arguments. */
    char *getArg();
    /* Converts the given string to a signed char. */
    char atoi(char *);
    /* The printable function */
    Print *printfunc;
    /* The valid commands in this cli. */
    Command *commands;
    /* Number of commands */
    char ncommands;

  private:
    /* The prompt to display. */
    const char *prompt;
    /* Clear the cli buffer */
    void _clear();
    /* Buffer holding text at the prompt */
    char _cliBuffer[BUFFER_LENGTH];
    unsigned char _cliBufferLength;
    /* Contains first word on the cli */
    char _commandBuffer[COMMAND_LENGTH];
    /* Contains all other words on the cli */
    char _argBuffer[COMMAND_LENGTH];
    /* Lightweight utility functions */
    // Length of the string.
    unsigned char _strlen(const char *);
    // Whether the two strings match.
    char _strncmp(const char *, const char *, char);
    // When enter is pressed, run the command.
    void _parseAndRun();
    // Display help.
    void _help(void);
};

#endif
