#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osproc_openfiles.sh PID

Show open files for a process by PID.
EOF
}

if [ $# -eq 0 ] || [ "$1" = "--help" ]; then
  usage
  exit 0
fi

if [ $# -ne 1 ]; then
  echo "Error: expected a single PID argument" >&2
  usage >&2
  exit 1
fi

pid="$1"
case "$pid" in
  ''|*[!0-9]*)
    echo "Error: PID must be a positive integer" >&2
    exit 1
    ;;
esac

if ! kill -0 "$pid" 2>/dev/null; then
  echo "Error: process not found or not accessible: $pid" >&2
  exit 1
fi

if command -v lsof >/dev/null 2>&1; then
  lsof -p "$pid" 2>/dev/null | awk '
    NR <= 50 { print }
    END {
      if (NR > 50) {
        print "... showing first 50 lines (use lsof -p PID for full list)"
      }
    }'
  exit 0
fi

if [ -d "/proc/$pid/fd" ]; then
  if ! command -v readlink >/dev/null 2>&1; then
    echo "Error: readlink not available for /proc fallback" >&2
    exit 1
  fi
  echo "Open file descriptors:"
  for fd in /proc/"$pid"/fd/*; do
    name="$(basename "$fd")"
    target="$(readlink "$fd" 2>/dev/null || echo "unknown")"
    printf '%s -> %s\n' "$name" "$target"
  done
else
  echo "Error: /proc/$pid/fd not available" >&2
  exit 1
fi
