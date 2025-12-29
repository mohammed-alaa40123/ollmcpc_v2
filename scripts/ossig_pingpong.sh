#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: ossig_pingpong.sh [rounds]

Run the signal ping-pong demo.
EOF
}

if [ $# -gt 1 ]; then
  echo "Error: too many arguments" >&2
  usage >&2
  exit 1
fi

if [ $# -eq 1 ]; then
  if [ "$1" = "--help" ]; then
    usage
    exit 0
  fi
  case "$1" in
    ''|*[!0-9]*)
      echo "Error: rounds must be a positive integer" >&2
      exit 1
      ;;
  esac
fi

bin_path="scripts/bin/sig_pingpong"
src_path="scripts/c_src/sig_pingpong.c"

if [ ! -x "$bin_path" ]; then
  if ! command -v gcc >/dev/null 2>&1; then
    echo "Error: gcc not found; cannot build $bin_path" >&2
    exit 1
  fi
  gcc -O2 -Wall -Wextra -o "$bin_path" "$src_path"
  if [ $? -ne 0 ]; then
    echo "Error: build failed" >&2
    exit 1
  fi
fi

if [ $# -eq 1 ]; then
  "$bin_path" "$1"
else
  "$bin_path"
fi
