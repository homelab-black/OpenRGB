#!/bin/sh
# run-rx7900xtx-tests.sh - Hardware-free reference RX 7900 XTX tests
# Tanner Krewson (tannerkrewson), 25 Oct 2026
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
export TMPDIR="${TMPDIR:-$HOME/.cache/tmp}"
mkdir -p "$TMPDIR"
build=$(mktemp -d "$TMPDIR/openrgb-rx7900xtx-test.XXXXXX")
trap 'rm -rf "$build"' EXIT HUP INT TERM

cd "$repo"
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -pthread \
    -I. -Ii2c_smbus -Ipci_ids -IRGBController -Idependencies/json \
    -IControllers/AMDRadeonRX7900XTXController \
    tests/AMDRadeonRX7900XTXControllerTest.cpp \
    Controllers/AMDRadeonRX7900XTXController/AMDRadeonRX7900XTXController.cpp \
    i2c_smbus/i2c_smbus.cpp -o "$build/controller-test"
"$build/controller-test"
printf '%s\n' 'RX 7900 XTX transport, captured commands, bus gating, and failure tests passed'
