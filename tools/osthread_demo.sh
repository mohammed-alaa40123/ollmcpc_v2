#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osthread_demo.sh [--help]

Run the thread roles demo.
EOF
}

for arg in "$@"; do
  if [ "$arg" = "--help" ]; then
    usage
    exit 0
  fi
done

if [ $# -gt 0 ]; then
  echo "Error: unexpected argument '$1'" >&2
  usage >&2
  exit 1
fi

bin_path="tools/bin/thread_roles"
src_path="tools/c_src/thread_roles.c"

if [ ! -x "$bin_path" ]; then
  if ! command -v gcc >/dev/null 2>&1; then
    echo "Error: gcc not found; cannot build $bin_path" >&2
    exit 1
  fi
  gcc -O2 -Wall -Wextra -pthread -o "$bin_path" "$src_path"
  if [ $? -ne 0 ]; then
    echo "Error: build failed" >&2
    exit 1
  fi
fi

echo "Thread Roles Demo"
echo "Running: $bin_path"
echo "Output:"
"$bin_path"
