# minui-btntest

Input device event monitor and query tool for devices that support MinUI

## Requirements

- A minui union toolchain
- Docker (this folder is assumed to be the contents of the toolchain workspace directory)
- `make`

## Building

- todo: this is built inside-out. Ideally you can clone this into the MinUI workspace directory and build from there under each toolchain, but instead it gets cloned _into_ a toolchain workspace directory and built from there.

## Usage

> [!IMPORTANT]
> `LD_LIBRARY_PATH` must be set to the MinUI toolchain's lib directory, as it is in the `launch.sh` file of the `MainUI.pak`.

### Synopsis

```shell
evtest <mode> <state> <combination> [<button,>...]
```

### Modes

- `capture` - capture events from the input device
- `wait` - wait for events from the input device

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

In some cases, the `minui-btntest` command will write output to stderr. This is due to linking against the MinUI library for startup/teardown functionality, which may log errors to stderr. To suppress this output, you can redirect stderr to stdout.

```shell
minui-btntest capture just_pressed all btn_a,btn_b 2>&1 > /dev/null
```
