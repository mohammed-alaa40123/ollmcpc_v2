#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osproc_find.sh "pattern"

Find processes by partial name and show a simple table.
EOF
}

if [ $# -eq 0 ] || [ "$1" = "--help" ]; then
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

matches="$(ps -eo pid,user,%cpu,%mem,comm | awk -v pat="$pattern" '
  BEGIN { IGNORECASE=1 }
  NR > 1 {
    line = $0
    if (index(tolower(line), tolower(pat)) > 0) {
      print line
    }
  }
')"

if [ -z "$matches" ]; then
  echo "No matching processes for pattern: $pattern" >&2
  exit 1
fi

printf '%-7s %-12s %-5s %-5s %s\n' "PID" "USER" "%CPU" "%MEM" "CMD"
printf '%s\n' "$matches"

count="$(printf '%s\n' "$matches" | wc -l | tr -d ' ')"
if [ "$count" -gt 1 ]; then
  echo "Matches: $count. Tip: use PID with other commands."
fi
