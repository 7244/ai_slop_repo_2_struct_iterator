#!/bin/bash
cd "$(dirname "$0")"
for f in *.cpp; do
  if clang++ -std=c++23 -c "$f" -o /dev/null 2>/dev/null; then
    echo "UNEXPECTED SUCCESS: $f"
    clang++ -std=c++23 -c "$f" -o /dev/null
    exit 1
  fi
done
