#!/bin/bash
set -e
cd "$(dirname "$0")"
clang++ -std=c++23 -Wall -Wextra guard_ok.cpp -o /tmp/opencode/guard_ok
/tmp/opencode/guard_ok
rm -f /tmp/opencode/guard_ok
echo "guard_ok OK"
