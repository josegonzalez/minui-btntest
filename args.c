#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btntest.h"

// log_error logs a message to stderr for debugging purposes
void log_error(const char *msg)
{
    // Set stderr to unbuffered mode
    setvbuf(stderr, NULL, _IONBF, 0);
    fprintf(stderr, "%s\n", msg);
}

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

void usage(char *argv[])
{
    printf("usage: %s <mode> <state> <combination> [<buttons>] [-- <command> [<args>...]]\n", argv[0]);
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
    // everything after the first "--" is a command to run on a match
    // argv[argc] is always NULL, so the command is already NULL-terminated
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--") == 0)
        {
            if (i + 1 >= argc)
            {
                log_error("missing command after --");
                return ExitCodeParseError;
            }

            state->command = &argv[i + 1];
            argc = i;
            break;
        }
    }

    // uppercase all arguments other than the first
    // the command and its arguments are left untouched
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
        else if (strcmp(argv[1], "WATCH") == 0)
        {
            state->button.mode = MODE_WATCH;
        }
        else
        {
            char buff[256];
            snprintf(buff, sizeof(buff), "invalid mode: %s", argv[1]);
            log_error(buff);
            return ExitCodeParseError;
        }
    }
    else
    {
        return ExitCodeParseError;
    }

    if (state->button.mode == MODE_WATCH && state->command == NULL)
    {
        log_error("watch mode requires a command after --");
        return ExitCodeParseError;
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
            snprintf(buff, sizeof(buff), "invalid event type: %s", argv[2]);
            log_error(buff);
            return ExitCodeParseError;
        }
    }
    else
    {
        return ExitCodeParseError;
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
            snprintf(buff, sizeof(buff), "invalid combination: %s", argv[3]);
            log_error(buff);
            return ExitCodeParseError;
        }
    }
    else
    {
        return ExitCodeParseError;
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
                snprintf(buff, sizeof(buff), "invalid button: %s", buttons);
                log_error(buff);
                return ExitCodeParseError;
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

    return ExitCodeSuccess;
}
