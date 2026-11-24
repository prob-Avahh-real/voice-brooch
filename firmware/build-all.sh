#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

run() {
  echo "==> $*"
  "$@"
}

echo "Voice Brooch — build all targets"
echo

run pio run -e native_test
run "$ROOT/test/native/test_protocol"

run node --test "$ROOT/test/js/protocol.test.mjs"

run pio run -d esp32 -e esp32_brooch

run pio run -e nrf52840_brooch

echo
echo "All targets OK."
