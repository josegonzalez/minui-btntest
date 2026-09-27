// driver is a host-only test harness for args.c and command.c.
// It is built by tests/command.bats and needs neither a toolchain
// nor the MinUI sources.
//
//   driver parse <args...>  parses the args and prints the resulting state
//   driver run <cmd...>     runs the command and exits with its exit code

#include <stdio.h>
#include <string.h>

#include "btntest.h"

static const char *mode_name(enum ButtonMode mode)
{
    switch (mode)
    {
    case MODE_CAPTURE:
        return "capture";
    case MODE_WAIT:
        return "wait";
    case MODE_WATCH:
        return "watch";
    }
    return "unknown";
}

static const char *state_name(enum ButtonState state)
{
    switch (state)
    {
    case STATE_JUST_PRESSED:
        return "just_pressed";
    case STATE_IS_PRESSED:
        return "is_pressed";
    case STATE_JUST_RELEASED:
        return "just_released";
    case STATE_JUST_REPEATED:
        return "just_repeated";
    }
    return "unknown";
}

static const char *combination_name(enum ButtonCombination combination)
{
    switch (combination)
    {
    case BTN_COMBO_ALL:
        return "all";
    case BTN_COMBO_ANY:
        return "any";
    case BTN_COMBO_EITHER:
        return "either";
    }
    return "unknown";
}

static int parse(int argc, char *argv[])
{
    struct AppState state = {
        .quitting = 0,
        .exit_code = ExitCodeError,
        .button = {
            .combination = BTN_COMBO_ALL,
            .state = STATE_JUST_PRESSED,
            .mode = MODE_CAPTURE,
            .buttons = {NULL}},
        .command = NULL};

    int result = parse_args(&state, argc, argv);
    if (result != ExitCodeSuccess)
    {
        return result;
    }

    printf("mode=%s\n", mode_name(state.button.mode));
    printf("state=%s\n", state_name(state.button.state));
    printf("combination=%s\n", combination_name(state.button.combination));

    printf("buttons=");
    for (int i = 0; i < 30 && state.button.buttons[i] != NULL; i++)
    {
        printf("%s%s", i > 0 ? "," : "", state.button.buttons[i]);
    }
    printf("\n");

    if (state.command == NULL)
    {
        printf("command=\n");
    }
    for (int i = 0; state.command != NULL && state.command[i] != NULL; i++)
    {
        printf("command=%s\n", state.command[i]);
    }

    return ExitCodeSuccess;
}

static int run(char *argv[])
{
    install_signal_handlers();

    int exit_code = run_command(argv);

    int signal_exit_code = pending_signal_exit_code();
    if (signal_exit_code != 0)
    {
        return signal_exit_code;
    }

    return exit_code;
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "parse") == 0)
    {
        // shift off the subcommand so argv[1] is the mode
        return parse(argc - 1, argv + 1);
    }

    if (argc > 2 && strcmp(argv[1], "run") == 0)
    {
        return run(argv + 2);
    }

    fprintf(stderr, "usage: %s parse <args...> | run <cmd...>\n", argv[0]);
    return 2;
}
