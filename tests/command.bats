#!/usr/bin/env bats
#
# Tests for argument parsing and running a command on a button match.
#
# These build tests/driver.c against args.c and command.c with the host C
# compiler (override with CC), so they need neither a cross toolchain nor the
# MinUI sources.

bats_require_minimum_version 1.5.0

setup_file() {
    REPO_ROOT="$(cd "$(dirname "$BATS_TEST_FILENAME")/.." && pwd)"
    export DRIVER="$BATS_FILE_TMPDIR/driver"
    "${CC:-cc}" -std=gnu99 -Wall -Werror -I"$REPO_ROOT" -o "$DRIVER" \
        "$REPO_ROOT/tests/driver.c" "$REPO_ROOT/args.c" "$REPO_ROOT/command.c"
}

# argument parsing

@test "parses a command after -- without uppercasing it" {
    run "$DRIVER" parse wait just_pressed all btn_a,btn_b -- /bin/Echo --Flag value
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "mode=wait" ]
    [ "${lines[1]}" = "state=just_pressed" ]
    [ "${lines[2]}" = "combination=all" ]
    [ "${lines[3]}" = "buttons=BTN_A,BTN_B" ]
    [ "${lines[4]}" = "command=/bin/Echo" ]
    [ "${lines[5]}" = "command=--Flag" ]
    [ "${lines[6]}" = "command=value" ]
}

@test "only the first -- separates the command" {
    run "$DRIVER" parse capture just_pressed either btn_a -- cmd -- arg
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "mode=capture" ]
    [ "${lines[4]}" = "command=cmd" ]
    [ "${lines[5]}" = "command=--" ]
    [ "${lines[6]}" = "command=arg" ]
}

@test "parses watch mode with a command" {
    run "$DRIVER" parse WATCH Just_Released either BTN_L1,btn_r1 -- ./screenshot.sh
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "mode=watch" ]
    [ "${lines[1]}" = "state=just_released" ]
    [ "${lines[2]}" = "combination=either" ]
    [ "${lines[3]}" = "buttons=BTN_L1,BTN_R1" ]
    [ "${lines[4]}" = "command=./screenshot.sh" ]
}

@test "parses any combination with a command" {
    run "$DRIVER" parse watch just_pressed any -- true
    [ "$status" -eq 0 ]
    [ "${lines[2]}" = "combination=either" ]
    [[ "${lines[3]}" == *"BTN_A"* ]]
    [ "${lines[4]}" = "command=true" ]
}

@test "watch mode requires a command" {
    run "$DRIVER" parse watch just_pressed all btn_a
    [ "$status" -eq 10 ]
    [[ "$output" == *"watch mode requires a command after --"* ]]
}

@test "-- without a command is a parse error" {
    run "$DRIVER" parse wait just_pressed all btn_a --
    [ "$status" -eq 10 ]
    [[ "$output" == *"missing command after --"* ]]
}

@test "buttons before -- are still validated" {
    run "$DRIVER" parse wait just_pressed all btn_nope -- true
    [ "$status" -eq 10 ]
    [[ "$output" == *"invalid button: BTN_NOPE"* ]]
}

@test "the mode must come before --" {
    run "$DRIVER" parse -- true
    [ "$status" -eq 10 ]
}

@test "arguments without -- have no command" {
    run "$DRIVER" parse capture just_pressed all btn_a,btn_b
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "mode=capture" ]
    [ "${lines[3]}" = "buttons=BTN_A,BTN_B" ]
    [ "${lines[4]}" = "command=" ]
}

# running a command

@test "run passes the arguments through and exits with the command's exit code" {
    run "$DRIVER" run sh -c 'echo "$1 $2"; exit 3' sh Hello --World
    [ "$status" -eq 3 ]
    [ "$output" = "Hello --World" ]
}

@test "run exits 0 when the command succeeds" {
    run "$DRIVER" run true
    [ "$status" -eq 0 ]
}

@test "run exits 127 when the command does not exist" {
    run -127 "$DRIVER" run minui-btntest-command-that-does-not-exist
    [[ "$output" == *"failed to execute minui-btntest-command-that-does-not-exist"* ]]
}

@test "run exits 126 when the command is not executable" {
    local script="$BATS_TEST_TMPDIR/not-executable"
    printf '#!/bin/sh\nexit 0\n' >"$script"
    chmod 0644 "$script"

    run "$DRIVER" run "$script"
    [ "$status" -eq 126 ]
}

@test "run exits 128 + the signal number when the command is killed" {
    run "$DRIVER" run sh -c 'kill -9 $$'
    [ "$status" -eq 137 ]
}

@test "SIGTERM is forwarded to the running command" {
    local ready="$BATS_TEST_TMPDIR/ready"
    local marker="$BATS_TEST_TMPDIR/terminated"

    "$DRIVER" run sh -c 'trap "touch \"$2\"; exit 0" TERM; touch "$1"; while :; do sleep 0.1; done' sh "$ready" "$marker" &
    local pid=$!

    for _ in $(seq 1 50); do
        [ -f "$ready" ] && break
        sleep 0.1
    done
    [ -f "$ready" ]

    kill -TERM "$pid"
    local status=0
    wait "$pid" || status=$?

    [ -f "$marker" ]
    [ "$status" -eq 143 ]
}
