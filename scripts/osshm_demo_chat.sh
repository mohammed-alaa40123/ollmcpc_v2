#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osshm_demo_chat.sh [--help]

Run the shared memory chat demo.
EOF
}

if [ $# -gt 0 ]; then
  case "$1" in
    --help)
      usage
      exit 0
      ;;
    *)
      echo "Error: unexpected argument '$1'" >&2
      usage >&2
      exit 1
      ;;
  esac
fi

bin_path="scripts/bin/shm_chat"
src_path="scripts/c_src/shm_chat.c"

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

echo "Shared Memory Chat Demo"
echo "Running: $bin_path"
echo "Output:"
"$bin_path"
