#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osproc_openfiles.sh PID

Show open files for a process by PID.
EOF
}

for arg in "$@"; do
  if [ "$arg" = "--help" ]; then
    usage
    exit 0
  fi
done

if [ $# -eq 0 ]; then
  usage
  exit 0
fi

if [ $# -ne 1 ]; then
  echo "Error: expected a single PID argument" >&2
  usage >&2
  exit 1
fi

pid="$1"
if [ -z "$pid" ] || [ "${pid#*[!0-9]*}" != "$pid" ]; then
  echo "Error: PID must be a positive integer" >&2
  exit 1
fi

if ! kill -0 "$pid" 2>/dev/null; then
  echo "Error: process not found or not accessible: $pid" >&2
  exit 1
fi

if command -v lsof >/dev/null 2>&1; then
  total="$(lsof -p "$pid" 2>/dev/null | wc -l | tr -d ' ')"
  if [ "$total" -gt 50 ]; then
    lsof -p "$pid" 2>/dev/null | head -n 50
    echo "... showing first 50 lines"
  else
    lsof -p "$pid" 2>/dev/null
  fi
  exit 0
fi

if [ -d "/proc/$pid/fd" ]; then
  echo "Open file descriptors:"
  ls -l "/proc/$pid/fd"
else
  echo "Error: /proc/$pid/fd not available" >&2
  exit 1
fi
