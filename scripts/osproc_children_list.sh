#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osproc_children_list.sh PID

Show child processes recursively as a simple tree.
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

if ! command -v ps >/dev/null 2>&1; then
  echo "Error: ps not available" >&2
  exit 1
fi

root_cmd="$(ps -p "$pid" -o comm= 2>/dev/null)"
if [ -z "$root_cmd" ]; then
  echo "Error: failed to read command for PID $pid" >&2
  exit 1
fi

proc_list="$(ps -e -o pid=,ppid=,comm=)"

print_children() {
  parent="$1"
  indent="$2"
  while read -r cpid cppid ccmd; do
    if [ "$cppid" = "$parent" ]; then
      echo "${indent}${cpid} ${ccmd}"
      print_children "$cpid" "  $indent"
    fi
  done <<< "$proc_list"
}

echo "$pid $root_cmd"
print_children "$pid" "  "
