# minui-btntest

Input device event monitor and query tool for devices that support MinUI

## Requirements

- A minui union toolchain
- Docker (this folder is assumed to be the contents of the toolchain workspace directory)
- `make`

## Building

- todo: this is built inside-out. Ideally you can clone this into the MinUI workspace directory and build from there under each toolchain, but instead it gets cloned _into_ a toolchain workspace directory and built from there.

### NextUI builds

`tg5050` and `h700` devices only run NextUI, so their binaries are built against a NextUI toolchain and carry a `-nextui` suffix:

| Platform id     | Upstream repo      | Version (Makefile var)        | Toolchain image                        |
|-----------------|--------------------|-------------------------------|----------------------------------------|
| `tg5050-nextui` | `loveRetro/NextUI` | `v6.14.0` (`NEXTUI_VERSION`)  | `savant/minui-toolchain:tg5050-nextui` |
| `h700-nextui`   | `pvaibhav/NextUI`  | `h700-rc11` (`H700_VERSION`)  | `savant/minui-toolchain:h700-nextui`   |

The binary still reports the bare device (`tg5050`, `h700`) at runtime, so it resolves the same on-card paths as the firmware.

### Tests

Run `make test` to check the per-platform build wiring as well as argument parsing and command execution. The test suites require `bats` and a host C compiler (`cc`, override with `CC`), but no toolchain.

## Usage

> [!IMPORTANT]
> `LD_LIBRARY_PATH` must be set to the MinUI toolchain's lib directory, as it is in the `launch.sh` file of the `MainUI.pak`.

### Synopsis

```shell
minui-btntest <mode> <state> <combination> [<button,>...] [-- <command> [<args>...]]
```

### Modes

- `capture` - capture events from the input device
- `wait` - wait for events from the input device
- `watch` - run a command every time the events occur (requires a command)

### States

- `just_pressed` - the button was just pressed
- `is_pressed` - the button is currently pressed
- `just_released` - the button was just released
- `just_repeated` - the button was just repeated

### Combinations

- `all` - all of the buttons specified
- `any` - any button that exists will match (no need to specify)
- `either` - any of the buttons specified

### Buttons

- `btn_a` - the A button
- `btn_b` - the B button
- `btn_x` - the X button
- `btn_y` - the Y button
- `btn_l1` - the L1 button
- `btn_l2` - the L2 button
- `btn_l3` - the L3 button
- `btn_r1` - the R1 button
- `btn_r2` - the R2 button
- `btn_r3` - the R3 button
- `btn_menu` - the Menu button
- `btn_minus` - the Minus button
- `btn_plus` - the Plus button
- `btn_power` - the Power button
- `btn_poweroff` - the Power Off button
- `btn_select` - the Select button
- `btn_start` - the Start button
- `btn_up` - the Up button
- `btn_down` - the Down button
- `btn_left` - the Left button
- `btn_right` - the Right button
- `btn_dpad_up` - the D-Pad Up button
- `btn_dpad_down` - the D-Pad Down button
- `btn_dpad_left` - the D-Pad Left button
- `btn_dpad_right` - the D-Pad Right button
- `btn_analog_up` - the Analog Stick Up button
- `btn_analog_down` - the Analog Stick Down button
- `btn_analog_left` - the Analog Stick Left button
- `btn_analog_right` - the Analog Stick Right button
- `btn_none` - the None button

> [!NOTE]
> On `tg5040`, `btn_l3`, `btn_r3`, `btn_plus` and `btn_minus` are only detected on the Trimui Brick when the `DEVICE` environment variable is set to `brick`. The MinUI launcher already sets it, so this only matters when running `minui-btntest` outside of MinUI.

### Examples

In the case where you want to check the current input, you can use the `capture` mode. This will exit 0 if the current input matches what was specified, and 1 otherwise.

```shell
# will check to see if both A and B are pressed
minui-btntest capture just_pressed all btn_a,btn_b

# will check to see if any button is pressed
minui-btntest capture just_pressed any

# will check to see if either A or B is pressed
minui-btntest capture just_pressed either btn_a,btn_b
```

In the case where you want to wait for an input, you can use the `wait` mode. This will exit 0 if the input matches what was specified, and 1 otherwise.

```shell
# will wait for both A and B to be pressed
minui-btntest wait just_pressed all btn_a,btn_b

# will wait for any button to be pressed
minui-btntest wait just_pressed any

# will wait for either A or B to be pressed
minui-btntest wait just_pressed either btn_a,btn_b
```

### Running a command

A command can be specified after `--`. It is run directly (not through a shell), and its arguments are passed through as-is rather than being uppercased like the rest of the arguments.

- `capture` - if the current input matches, runs the command and exits with its exit code. Otherwise exits 1 without running the command.
- `wait` - waits for the input to match, runs the command once, and exits with its exit code.
- `watch` - runs the command every time the input matches until `minui-btntest` receives `SIGINT` or `SIGTERM`. The command's exit code is ignored.

The command runs in the foreground, so input is not checked while it is running. In `watch` mode, any input that occurred while the command was running is discarded once it exits, so a button held through the command must be released and pressed again to trigger the command again.

In `capture` and `wait` modes, if the command cannot be found, `minui-btntest` exits 127. If it cannot be executed, it exits 126. If the command is terminated by a signal, `minui-btntest` exits with 128 plus the signal number. A `SIGINT` or `SIGTERM` received while the command is running is forwarded to the command, and `minui-btntest` exits with 130 or 143 respectively once the command exits.

```shell
# will take a screenshot every time L1 and R1 are pressed together
minui-btntest watch just_pressed all btn_l1,btn_r1 -- /path/to/screenshot.sh --format png

# will wait for either A or B to be pressed, then run a command once
minui-btntest wait just_pressed either btn_a,btn_b -- /path/to/command.sh

# will run a command if Start is currently pressed
minui-btntest capture is_pressed all btn_start -- /path/to/command.sh
```

Using `watch` instead of running `minui-btntest wait` in a loop avoids re-initializing the device settings every time the command is run.

In some cases, the `minui-btntest` command will write output to stderr. This is due to linking against the MinUI library for startup/teardown functionality, which may log errors to stderr. To suppress this output, you can redirect stderr to stdout.

```shell
minui-btntest capture just_pressed all btn_a,btn_b 2>&1 > /dev/null
```
