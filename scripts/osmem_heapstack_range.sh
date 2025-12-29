#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osmem_heapstack_range.sh PID

Show heap and stack address ranges from /proc/PID/maps.
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

if [ ! -r "/proc/$pid/maps" ]; then
  echo "Error: cannot read /proc/$pid/maps" >&2
  exit 1
fi

lines="$(awk '/\[(heap|stack)\]/ {print $1, $2, $NF}' "/proc/$pid/maps")"
if [ -z "$lines" ]; then
  echo "No heap/stack entries found for PID $pid" >&2
  exit 1
fi

echo "Range               Perms Segment"
printf '%s\n' "$lines" | while read -r range perms segment; do
  printf '%-18s %-5s %s\n' "$range" "$perms" "$segment"
done
