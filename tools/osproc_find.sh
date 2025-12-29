#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osproc_find.sh "pattern"

Find processes by partial name and show a simple table.
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
  echo "Error: expected a single pattern argument" >&2
  usage >&2
  exit 1
fi

pattern="$1"
if [ -z "$pattern" ]; then
  echo "Error: pattern must not be empty" >&2
  exit 1
fi

if ! command -v ps >/dev/null 2>&1; then
  echo "Error: ps not available" >&2
  exit 1
fi

list="$(ps -eo pid,user,%cpu,%mem,comm | tail -n +2)"
matches="$(printf '%s\n' "$list" | grep -F -i -- "$pattern" || true)"

if [ -z "$matches" ]; then
  echo "No matching processes for pattern: $pattern" >&2
  exit 1
fi

printf '%-10s %-10s %-10s %-10s %s\n' "PID" "USER" "%CPU" "%MEM" "CMD"
printf '%s\n' "$matches"

count="$(printf '%s\n' "$matches" | wc -l | tr -d ' ')"
if [ "$count" -gt 1 ]; then
  echo "Matches: $count. Tip: use PID with other commands."
fi
