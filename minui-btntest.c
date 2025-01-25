#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <msettings.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>

#include "defines.h"
#include "api.h"
#include "utils.h"

FILE *log_file = NULL;

// log_error logs a message to stderr for debugging purposes
void log_error(const char *msg)
{
    // Set stderr to unbuffered mode
    setvbuf(stderr, NULL, _IONBF, 0);
    fprintf(stderr, "%s\n", msg);
}

enum ButtonMode
{
    MODE_CAPTURE,
    MODE_WAIT,
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
};

const char *valid_buttons[] = {
    "BTN_A",
    "BTN_ANALOG_DOWN",
    "BTN_ANALOG_LEFT",
    "BTN_ANALOG_RIGHT",
    "BTN_ANALOG_UP",
    "BTN_B",
    "BTN_DOWN",
    "BTN_DPAD_DOWN",
    "BTN_DPAD_LEFT",
    "BTN_DPAD_RIGHT",
    "BTN_DPAD_UP",
    "BTN_L1",
    "BTN_L2",
    "BTN_L3",
    "BTN_LEFT",
    "BTN_MENU",
    "BTN_MINUS",
    "BTN_NONE",
    "BTN_PLUS",
    "BTN_POWER",
    "BTN_POWEROFF",
    "BTN_R1",
    "BTN_R2",
    "BTN_R3",
    "BTN_RIGHT",
    "BTN_SELECT",
    "BTN_START",
    "BTN_UP",
    "BTN_X",
    "BTN_Y",
};

int button_to_enum(char *button_str)
{
    int button;
    if (strcmp(button_str, "BTN_A") == 0)
        button = BTN_A;
    else if (strcmp(button_str, "BTN_ANALOG_UP") == 0)
        button = BTN_ANALOG_UP;
    else if (strcmp(button_str, "BTN_ANALOG_DOWN") == 0)
        button = BTN_ANALOG_DOWN;
    else if (strcmp(button_str, "BTN_ANALOG_LEFT") == 0)
        button = BTN_ANALOG_LEFT;
    else if (strcmp(button_str, "BTN_ANALOG_RIGHT") == 0)
        button = BTN_ANALOG_RIGHT;
    else if (strcmp(button_str, "BTN_B") == 0)
        button = BTN_B;
    else if (strcmp(button_str, "BTN_DOWN") == 0)
        button = BTN_DOWN;
    else if (strcmp(button_str, "BTN_DPAD_DOWN") == 0)
        button = BTN_DPAD_DOWN;
    else if (strcmp(button_str, "BTN_DPAD_LEFT") == 0)
        button = BTN_DPAD_LEFT;
    else if (strcmp(button_str, "BTN_DPAD_RIGHT") == 0)
        button = BTN_DPAD_RIGHT;
    else if (strcmp(button_str, "BTN_DPAD_UP") == 0)
        button = BTN_DPAD_UP;
    else if (strcmp(button_str, "BTN_L1") == 0)
        button = BTN_L1;
    else if (strcmp(button_str, "BTN_L2") == 0)
        button = BTN_L2;
    else if (strcmp(button_str, "BTN_L3") == 0)
        button = BTN_L3;
    else if (strcmp(button_str, "BTN_LEFT") == 0)
        button = BTN_LEFT;
    else if (strcmp(button_str, "BTN_MENU") == 0)
        button = BTN_MENU;
    else if (strcmp(button_str, "BTN_MINUS") == 0)
        button = BTN_MINUS;
    else if (strcmp(button_str, "BTN_POWER") == 0)
        button = BTN_POWER;
    else if (strcmp(button_str, "BTN_POWEROFF") == 0)
        button = BTN_POWEROFF;
    else if (strcmp(button_str, "BTN_R1") == 0)
        button = BTN_R1;
    else if (strcmp(button_str, "BTN_R2") == 0)
        button = BTN_R2;
    else if (strcmp(button_str, "BTN_R3") == 0)
        button = BTN_R3;
    else if (strcmp(button_str, "BTN_RIGHT") == 0)
        button = BTN_RIGHT;
    else if (strcmp(button_str, "BTN_START") == 0)
        button = BTN_START;
    else if (strcmp(button_str, "BTN_SELECT") == 0)
        button = BTN_SELECT;
    else if (strcmp(button_str, "BTN_UP") == 0)
        button = BTN_UP;
    else if (strcmp(button_str, "BTN_X") == 0)
        button = BTN_X;
    else if (strcmp(button_str, "BTN_Y") == 0)
        button = BTN_Y;
    else
        button = BTN_NONE;

    return button;
}

bool handle_just_pressed(struct AppState *state)
{
    for (int i = 0; i < 30; i++)
    {
        if (state->button.buttons[i] == NULL)
        {
            break;
        }

        char *button = state->button.buttons[i];
        int button_enum = button_to_enum(button);
        if (state->button.combination == BTN_COMBO_EITHER)
        {
            if (PAD_justPressed(button_enum))
            {
                return true;
            }
        }
        else if (!PAD_justPressed(button_enum))
        {
            return false;
        }
    }

    if (state->button.combination == BTN_COMBO_EITHER)
    {
        return false;
    }

    return true;
}

bool handle_is_pressed(struct AppState *state)
{
    for (int i = 0; i < 30; i++)
    {
        if (state->button.buttons[i] == NULL)
        {
            break;
        }

        char *button_str = state->button.buttons[i];
        int button_enum = button_to_enum(button_str);
        if (state->button.combination == BTN_COMBO_EITHER)
        {
            if (PAD_isPressed(button_enum))
            {
                return true;
            }
        }
        else if (!PAD_isPressed(button_enum))
        {
            return false;
        }
    }

    if (state->button.combination == BTN_COMBO_EITHER)
    {
        return false;
    }

    return true;
}

bool handle_just_released(struct AppState *state)
{
    for (int i = 0; i < 30; i++)
    {
        if (state->button.buttons[i] == NULL)
        {
            break;
        }

        char *button = state->button.buttons[i];
        int button_enum = button_to_enum(button);
        if (state->button.combination == BTN_COMBO_EITHER)
        {
            if (PAD_justReleased(button_enum))
            {
                return true;
            }
        }
        else if (!PAD_justReleased(button_enum))
        {
            return false;
        }
    }

    if (state->button.combination == BTN_COMBO_EITHER)
    {
        return false;
    }

    return true;
}

bool handle_just_repeated(struct AppState *state)
{
    for (int i = 0; i < 30; i++)
    {
        if (state->button.buttons[i] == NULL)
        {
            break;
        }

        char *button = state->button.buttons[i];
        int button_enum = button_to_enum(button);
        if (state->button.combination == BTN_COMBO_EITHER)
        {
            if (PAD_justRepeated(button_enum))
            {
                return true;
            }
        }
        else if (!PAD_justRepeated(button_enum))
        {
            return false;
        }
    }

    if (state->button.combination == BTN_COMBO_EITHER)
    {
        return false;
    }

    return true;
}

// handle_input interprets input events and mutates app state
void handle_input(struct AppState *state)
{
    PAD_poll();

    bool result = false;
    switch (state->button.state)
    {
    case STATE_JUST_PRESSED:
        result = handle_just_pressed(state);
        break;
    case STATE_IS_PRESSED:
        result = handle_is_pressed(state);
        break;
    case STATE_JUST_RELEASED:
        result = handle_just_released(state);
        break;
    case STATE_JUST_REPEATED:
        result = handle_just_repeated(state);
        break;
    }

    if (result)
    {
        state->quitting = 1;
        state->exit_code = EXIT_SUCCESS;
        return;
    }

    if (state->button.mode == MODE_CAPTURE)
    {
        state->quitting = 1;
        state->exit_code = EXIT_FAILURE;
        return;
    }
}

void usage(char *argv[])
{
    printf("usage: %s <event_type> <combination> [<buttons>]\n", argv[0]);
}

bool is_valid_button(char *button)
{
    if (button == NULL)
    {
        return false;
    }

    for (int i = 0; i < sizeof(valid_buttons) / sizeof(valid_buttons[0]); i++)
    {
        if (strcmp(button, valid_buttons[i]) == 0)
        {
            return true;
        }
    }

    return false;
}

char *strtoupper(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = toupper(str[i]);
    }
    return str;
}

int parse_args(struct AppState *state, int argc, char *argv[])
{
    // uppercase all arguments other than the first
    for (int i = 1; i < argc; i++)
    {
        argv[i] = strtoupper(argv[i]);
    }

    if (argc > 1)
    {
        if (strcmp(argv[1], "CAPTURE") == 0)
        {
            state->button.mode = MODE_CAPTURE;
        }
        else if (strcmp(argv[1], "WAIT") == 0)
        {
            state->button.mode = MODE_WAIT;
        }
        else
        {
            char buff[256];
            sprintf(buff, "invalid mode: %s", argv[1]);
            log_error(buff);
            return EXIT_FAILURE;
        }
    }
    else
    {
        return EXIT_FAILURE;
    }

    if (argc > 2)
    {
        if (strcmp(argv[2], "JUST_PRESSED") == 0)
        {
            state->button.state = STATE_JUST_PRESSED;
        }
        else if (strcmp(argv[2], "IS_PRESSED") == 0)
        {
            state->button.state = STATE_IS_PRESSED;
        }
        else if (strcmp(argv[2], "JUST_RELEASED") == 0)
        {
            state->button.state = STATE_JUST_RELEASED;
        }
        else if (strcmp(argv[2], "JUST_REPEATED") == 0)
        {
            state->button.state = STATE_JUST_REPEATED;
        }
        else
        {
            char buff[256];
            sprintf(buff, "invalid event type: %s", argv[2]);
            log_error(buff);
            return EXIT_FAILURE;
        }
    }
    else
    {
        return EXIT_FAILURE;
    }

    if (argc > 3)
    {
        if (strcmp(argv[3], "ALL") == 0)
        {
            state->button.combination = BTN_COMBO_ALL;
        }
        else if (strcmp(argv[3], "ANY") == 0)
        {
            state->button.combination = BTN_COMBO_ANY;
        }
        else if (strcmp(argv[3], "EITHER") == 0)
        {
            state->button.combination = BTN_COMBO_EITHER;
        }
        else
        {
            char buff[256];
            sprintf(buff, "invalid combination: %s", argv[3]);
            log_error(buff);
            return EXIT_FAILURE;
        }
    }
    else
    {
        return EXIT_FAILURE;
    }

    if (argc > 4)
    {
        char *buttons = strtok(argv[4], ",");
        int i = 0;
        while (buttons != NULL)
        {
            if (!is_valid_button(buttons))
            {
                char buff[256];
                sprintf(buff, "invalid button: %s", buttons);
                log_error(buff);
                return EXIT_FAILURE;
            }

            state->button.buttons[i] = buttons;
            buttons = strtok(NULL, ",");
            i++;
        }
    }

    // trick: we want to match any button, so we use EITHER
    // and then add every button to the list
    if (state->button.combination == BTN_COMBO_ANY)
    {
        state->button.combination = BTN_COMBO_EITHER;

        for (int i = 0; i < sizeof(valid_buttons) / sizeof(valid_buttons[0]); i++)
        {
            state->button.buttons[i] = strdup(valid_buttons[i]);
        }
    }

    return EXIT_SUCCESS;
}

// swallow_stdout_from_function swallows stdout from a function
// this is useful for suppressing output from a function
// that we don't want to see in the log file
// the InitSettings() function is an example of this (some implementations print to stdout)
void swallow_stdout_from_function(void (*func)(void))
{
    int original_stdout = dup(STDOUT_FILENO);
    int dev_null = open("/dev/null", O_WRONLY);

    dup2(dev_null, STDOUT_FILENO);
    close(dev_null);

    func();

    dup2(original_stdout, STDOUT_FILENO);
    close(original_stdout);
}

void signal_handler(int signal)
{
    // if the signal is a ctrl+c, exit with code 130
    if (signal == SIGINT)
    {
        exit(130);
    }
    else
    {
        exit(1);
    }
}

// main is the entry point for the app
int main(int argc, char *argv[])
{
    char *buttons[30] = {NULL};
    struct AppState state = {
        .quitting = 0,
        .exit_code = EXIT_FAILURE,
        .button = {
            .combination = BTN_COMBO_ALL,
            .state = STATE_JUST_PRESSED,
            .mode = MODE_CAPTURE,
            .buttons = *buttons}};

    if (parse_args(&state, argc, argv) != EXIT_SUCCESS)
    {
        usage(argv);
        return EXIT_FAILURE;
    }

    // initialize:
    // - input from the pad/joystick/buttons/etc.
    // - sync hardware settings (brightness, hdmi, speaker, etc.)
    PAD_init();
    swallow_stdout_from_function(InitSettings);

    signal(SIGINT, signal_handler);

    while (!state.quitting)
    {
        handle_input(&state);
    }

    QuitSettings();
    PAD_quit();

    return state.exit_code;
}