#ifndef BTNTEST_H
#define BTNTEST_H

// this header must not include SDL or MinUI headers so that
// the argument parsing and command execution can be tested on the host

#include <stdbool.h>

enum list_result_t
{
    ExitCodeSuccess = 0,
    ExitCodeError = 1,
    ExitCodeCancelButton = 2,
    ExitCodeMenuButton = 3,
    ExitCodeActionButton = 4,
    ExitCodeInactionButton = 5,
    ExitCodeStartButton = 6,
    ExitCodeParseError = 10,
    ExitCodeSerializeError = 11,
    ExitCodeTimeout = 124,
    ExitCodeKeyboardInterrupt = 130,
    ExitCodeSigterm = 143,
};
typedef int ExitCode;

enum ButtonMode
{
    MODE_CAPTURE,
    MODE_WAIT,
    MODE_WATCH,
};

enum ButtonState
{
    STATE_JUST_PRESSED,
    STATE_IS_PRESSED,
    STATE_JUST_RELEASED,
    STATE_JUST_REPEATED,
};

enum ButtonCombination
{
    // all of the buttons specified
    BTN_COMBO_ALL,
    // any button that exists will match (no need to specify)
    // note that this will exit for any button state in question,
    // so you don't need to have every button in the specified event type
    BTN_COMBO_ANY,
    // either of the buttons specified
    BTN_COMBO_EITHER,
};

// ButtonState holds the state of what we are tracking
struct Button
{
    enum ButtonCombination combination; // the type of button combination
    enum ButtonState state;             // the type of button state
    enum ButtonMode mode;
    char *buttons[30]; // a list of buttons to track
};

// AppState holds the current state of the application
struct AppState
{
    int quitting;         // whether the app should exit
    int exit_code;        // the exit code to return
    struct Button button; // the state of what we are tracking
    char **command;       // a NULL-terminated command to run on a match, or NULL
};

// args.c
void log_error(const char *msg);
void usage(char *argv[]);
bool is_valid_button(char *button);
int parse_args(struct AppState *state, int argc, char *argv[]);

// command.c
int run_command(char *const argv[]);
void signal_handler(int signal);
void install_signal_handlers(void);
int pending_signal_exit_code(void);

#endif
