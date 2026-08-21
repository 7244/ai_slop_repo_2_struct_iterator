#!/bin/bash
set -e
cd "$(dirname "$0")"

for f in scalars_arrays aggregates; do
  clang++ -std=gnu++23 -Wall -Wextra -Werror "$f.cpp" \
    -o "/tmp/opencode/hash_$f"
  "/tmp/opencode/hash_$f"
  rm -f "/tmp/opencode/hash_$f"
done

if clang++ -std=gnu++23 -Wall -Wextra -Werror \
  -c invalid_volatile.cpp -o /dev/null 2>/dev/null; then
  echo "UNEXPECTED SUCCESS: invalid_volatile.cpp"
  clang++ -std=gnu++23 -Wall -Wextra -Werror \
    -c invalid_volatile.cpp -o /dev/null
  exit 1
fi

echo "hash OK"
