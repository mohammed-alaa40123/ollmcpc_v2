#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osproc_children_list.sh PID

Show child processes recursively as a simple tree.
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

if ! command -v ps >/dev/null 2>&1; then
  echo "Error: ps not available" >&2
  exit 1
fi

ps -o pid,ppid,comm --forest --ppid "$pid"
