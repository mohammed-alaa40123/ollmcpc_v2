#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osfile_watch.sh PATH [--interval N] [--lines N]

Monitor a file or directory in a loop.
  --interval  Seconds between checks (default 2)
  --lines     If PATH is a file, print last N lines each cycle
  --help      Show this help
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

path="$1"
shift

interval=2
lines=""

while [ $# -gt 0 ]; do
  if [ "$1" = "--interval" ]; then
    shift
    if [ $# -eq 0 ]; then
      echo "Error: --interval requires a number" >&2
      exit 1
    fi
    interval="$1"
    shift
    continue
  fi

  if [ "$1" = "--lines" ]; then
    shift
    if [ $# -eq 0 ]; then
      echo "Error: --lines requires a number" >&2
      exit 1
    fi
    lines="$1"
    shift
    continue
  fi

  echo "Error: unexpected argument '$1'" >&2
  usage >&2
  exit 1
done

if [ ! -e "$path" ]; then
  echo "Error: path not found: $path" >&2
  exit 1
fi

if ! [ "$interval" -gt 0 ] 2>/dev/null; then
  echo "Error: --interval must be a positive integer" >&2
  exit 1
fi

if [ -n "$lines" ]; then
  if ! [ "$lines" -gt 0 ] 2>/dev/null; then
    echo "Error: --lines must be a positive integer" >&2
    exit 1
  fi
  if [ ! -f "$path" ]; then
    echo "Error: --lines is only valid for files" >&2
    exit 1
  fi
  if ! command -v tail >/dev/null 2>&1; then
    echo "Error: tail not available" >&2
    exit 1
  fi
fi

trap 'echo "Stopped by user." >&2; exit 0' INT

while true; do
  now="$(date '+%Y-%m-%d %H:%M:%S')"
  if command -v stat >/dev/null 2>&1; then
    size="$(stat -c '%s' "$path" 2>/dev/null || echo "unknown")"
    mtime="$(stat -c '%y' "$path" 2>/dev/null | cut -d'.' -f1)"
    if [ -z "$mtime" ]; then
      mtime="unknown"
    fi
  else
    size="unknown"
    mtime="unknown"
  fi

  echo "$now | size: $size | modified: $mtime"

  if [ -n "$lines" ]; then
    echo "Last $lines lines:"
    tail -n "$lines" "$path"
  fi

  sleep "$interval"
done
