#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <msettings.h>
#include <string.h>
#ifdef USE_SDL2
#include <SDL2/SDL_ttf.h>
#else
#include <SDL/SDL_ttf.h>
#endif

#include "defines.h"
#include "api.h"
#include "utils.h"

#include "btntest.h"

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

    if (result && state->command != NULL)
    {
        int command_exit_code = run_command(state->command);

        // a signal was forwarded to the command, so exit as if we received it
        int signal_exit_code = pending_signal_exit_code();
        if (signal_exit_code != 0)
        {
            state->quitting = 1;
            state->exit_code = signal_exit_code;
            return;
        }

        if (state->button.mode == MODE_WATCH)
        {
            // discard any input that happened while the command was running
            // so that it doesn't immediately trigger the command again
            PAD_poll();
            PAD_reset();
            return;
        }

        state->quitting = 1;
        state->exit_code = command_exit_code;
        return;
    }

    if (result)
    {
        state->quitting = 1;
        state->exit_code = ExitCodeSuccess;
        return;
    }

    if (state->button.mode == MODE_CAPTURE)
    {
        state->quitting = 1;
        state->exit_code = ExitCodeError;
        return;
    }
}

static void set_cloexec(int fd)
{
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1)
    {
        return;
    }

    fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

// suppress_output suppresses stdout and stderr
// returns a single integer containing both file descriptors
int suppress_output(void)
{
    int stdout_fd = dup(STDOUT_FILENO);
    int stderr_fd = dup(STDERR_FILENO);

    // Prevent child processes started while stdout is suppressed from
    // inheriting the saved descriptors and keeping command-substitution
    // pipes open after the main process exits.
    set_cloexec(stdout_fd);
    set_cloexec(stderr_fd);

    int dev_null_fd = open("/dev/null", O_WRONLY);
    dup2(dev_null_fd, STDOUT_FILENO);
    dup2(dev_null_fd, STDERR_FILENO);
    close(dev_null_fd);

    return (stdout_fd << 16) | stderr_fd;
}

// restore_output restores stdout and stderr to the original file descriptors
void restore_output(int saved_fds)
{
    int stdout_fd = (saved_fds >> 16) & 0xFFFF;
    int stderr_fd = saved_fds & 0xFFFF;

    fflush(stdout);
    fflush(stderr);

    dup2(stdout_fd, STDOUT_FILENO);
    dup2(stderr_fd, STDERR_FILENO);

    close(stdout_fd);
    close(stderr_fd);
}

// swallow_stdout_from_function swallows stdout from a function
// this is useful for suppressing output from a function
// that we don't want to see in the log file
// the InitSettings() function is an example of this (some implementations print to stdout)
void swallow_stdout_from_function(void (*func)(void))
{
    int saved_fds = suppress_output();

    func();

    restore_output(saved_fds);
}

// init initializes the app state
// everything is placed here as MinUI sometimes logs to stdout
// and the logging happens depending on the platform
void init()
{
    // set the cpu speed to the menu speed
    // this is done here to ensure we downclock
    // the menu (no need to draw power unnecessarily)
    PWR_setCPUSpeed(CPU_SPEED_MENU);

#ifdef PLATFORM_TG5040
    // tg5040 covers both the Brick and the Smart Pro. MinUI only sets is_brick in
    // PLAT_initVideo, which this tool never calls, so without this L3/R3/plus/minus are dropped
    is_brick = exactMatch("brick", getenv("DEVICE"));
#endif

    // initialize:
    // - input from the pad/joystick/buttons/etc.
    // - sync hardware settings (brightness, hdmi, speaker, etc.)
    PAD_init();
    InitSettings();
}

// destruct cleans up the app state in reverse order
void destruct()
{
    QuitSettings();
    PAD_quit();
}

// main is the entry point for the app
int main(int argc, char *argv[])
{
    char *buttons[30] = {NULL};
    struct AppState state = {
        .quitting = 0,
        .exit_code = ExitCodeError,
        .button = {
            .combination = BTN_COMBO_ALL,
            .state = STATE_JUST_PRESSED,
            .mode = MODE_CAPTURE,
            .buttons = *buttons},
        .command = NULL};

    if (parse_args(&state, argc, argv) != ExitCodeSuccess)
    {
        usage(argv);
        return ExitCodeParseError;
    }

    // swallow all stdout from init calls
    // MinUI will sometimes randomly log to stdout
    swallow_stdout_from_function(init);

    install_signal_handlers();

    while (!state.quitting)
    {
        handle_input(&state);
        usleep(50000);
    }

    swallow_stdout_from_function(destruct);

    return state.exit_code;
}
