#!/usr/bin/env bats
#
# Build-wiring tests for the Makefile.
#
# These assert how the Makefile resolves per-platform build variables, with a
# focus on the NextUI-specific variants. They introspect the Makefile with
# `make print-<VAR> PLATFORM=<p>`, so they need neither a cross toolchain nor a
# cloned upstream tree and run on any host with make.

setup() {
    REPO_ROOT="$(cd "$(dirname "$BATS_TEST_FILENAME")/.." && pwd)"
}

# print-<VAR> for a platform; sets $status/$output/$lines via bats `run`.
mk() { # <VAR> <PLATFORM>
    run make --no-print-directory -C "$REPO_ROOT" "print-$1" PLATFORM="$2"
    [ "$status" -eq 0 ]
}

# tg5050-nextui: NextUI-only device that needs the standalone mali blob

@test "tg5050-nextui uses the loveRetro NextUI upstream at the pinned tag" {
    mk UPSTREAM_REPO tg5050-nextui
    [ "$output" = "UPSTREAM_REPO=https://github.com/loveRetro/NextUI" ]
    mk UPSTREAM_VERSION tg5050-nextui
    [ "$output" = "UPSTREAM_VERSION=v6.14.0" ]
}

@test "tg5050-nextui builds the bare tg5050 workspace and is flagged NextUI" {
    mk WORKSPACE tg5050-nextui
    [ "$output" = "WORKSPACE=tg5050" ]
    mk IS_NEXTUI tg5050-nextui
    [ "$output" = "IS_NEXTUI=1" ]
}

@test "tg5050-nextui bakes the device id (not the -nextui variant) into -DPLATFORM" {
    mk CFLAGS tg5050-nextui
    [[ "$output" == *'-DPLATFORM_NEXTUI'* ]]
    [[ "$output" == *'-DPLATFORM=\"tg5050\"'* ]]
    [[ "$output" != *'-DPLATFORM=\"tg5050-nextui\"'* ]]
}

@test "tg5050-nextui compiles the tg5050 platform source and NextUI config" {
    mk SOURCE tg5050-nextui
    [[ "$output" == *'minui/workspace/tg5050/platform/platform.c'* ]]
    [[ "$output" == *'minui/workspace/all/common/config.c'* ]]
}

@test "tg5050-nextui links the mali blob explicitly" {
    mk NEXTUI_GL_LIBS tg5050-nextui
    [[ "$output" == *'-lGLESv2'* ]]
    [[ "$output" == *'-lmali'* ]]
    [[ "$output" == *'-lsamplerate'* ]]
}

# h700-nextui: NextUI-only device sourced from the pvaibhav fork

@test "h700-nextui uses the pvaibhav NextUI fork at the h700 tag" {
    mk UPSTREAM_REPO h700-nextui
    [ "$output" = "UPSTREAM_REPO=https://github.com/pvaibhav/NextUI" ]
    mk UPSTREAM_VERSION h700-nextui
    [ "$output" = "UPSTREAM_VERSION=h700-rc11" ]
}

@test "h700-nextui builds the bare h700 workspace and is flagged NextUI" {
    mk WORKSPACE h700-nextui
    [ "$output" = "WORKSPACE=h700" ]
    mk IS_NEXTUI h700-nextui
    [ "$output" = "IS_NEXTUI=1" ]
}

@test "h700-nextui bakes the device id (not the -nextui variant) into -DPLATFORM" {
    mk CFLAGS h700-nextui
    [[ "$output" == *'-DPLATFORM_NEXTUI'* ]]
    [[ "$output" == *'-DPLATFORM=\"h700\"'* ]]
    [[ "$output" != *'-DPLATFORM=\"h700-nextui\"'* ]]
}

@test "h700-nextui compiles the h700 platform source and NextUI config" {
    mk SOURCE h700-nextui
    [[ "$output" == *'minui/workspace/h700/platform/platform.c'* ]]
    [[ "$output" == *'minui/workspace/all/common/config.c'* ]]
}

@test "h700-nextui links GLESv2 and samplerate but not mali" {
    mk NEXTUI_GL_LIBS h700-nextui
    [[ "$output" == *'-lGLESv2'* ]]
    [[ "$output" == *'-lsamplerate'* ]]
    [[ "$output" != *'-lmali'* ]]
}

@test "h700-nextui produces the -nextui artifact id" {
    mk PLATFORM h700-nextui
    [ "$output" = "PLATFORM=h700-nextui" ]
}

# regression: a plain MinUI platform is untouched by the NextUI wiring

@test "tg5040 (MinUI) uses the shauninman upstream and is not a NextUI build" {
    mk UPSTREAM_REPO tg5040
    [ "$output" = "UPSTREAM_REPO=https://github.com/shauninman/MinUI" ]
    mk UPSTREAM_VERSION tg5040
    [ "$output" = "UPSTREAM_VERSION=v20251023-0" ]
    mk IS_NEXTUI tg5040
    [ "$output" = "IS_NEXTUI=" ]
}

@test "tg5040 (MinUI) keeps its own workspace and device id" {
    mk WORKSPACE tg5040
    [ "$output" = "WORKSPACE=tg5040" ]
    mk CFLAGS tg5040
    [[ "$output" == *'-DPLATFORM=\"tg5040\"'* ]]
    [[ "$output" != *'-DPLATFORM_NEXTUI'* ]]
}

@test "tg5040 (MinUI) is flagged so the Brick's L3/R3 are detected" {
    mk CFLAGS tg5040
    [[ "$output" == *'-DPLATFORM_TG5040'* ]]
}

@test "other platforms are not flagged as tg5040" {
    for platform in rg35xxplus miyoomini tg5050-nextui h700-nextui; do
        mk CFLAGS "$platform"
        [[ "$output" != *'-DPLATFORM_TG5040'* ]]
    done
}

@test "tg5040 (MinUI) does not compile NextUI config" {
    mk SOURCE tg5040
    [[ "$output" != *'config.c'* ]]
}

# the argument parsing and command runner are compiled on every platform

@test "tg5040 (MinUI) compiles the argument parser and command runner" {
    mk SOURCE tg5040
    [[ "$output" == *' args.c '* ]]
    [[ "$output" == *' command.c '* ]]
}

@test "tg5050-nextui compiles the argument parser and command runner" {
    mk SOURCE tg5050-nextui
    [[ "$output" == *' args.c '* ]]
    [[ "$output" == *' command.c '* ]]
}
