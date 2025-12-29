#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osmem_dependency_demo.sh [--help]

Run the memory dependency demo and snapshot /proc maps.
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

bin_path="scripts/bin/mem_dep"
src_path="scripts/c_src/mem_dep.c"

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

tmpfile="$(mktemp)"
demo_pid=""
parent_pid=""
child_pid=""

cleanup() {
  if [ -n "${child_pid:-}" ]; then
    kill "$child_pid" 2>/dev/null || true
  fi
  if [ -n "${demo_pid:-}" ]; then
    kill "$demo_pid" 2>/dev/null || true
  fi
  rm -f "$tmpfile"
}
trap cleanup EXIT INT TERM

"$bin_path" > "$tmpfile" 2>&1 &
demo_pid=$!

attempts=0
while [ $attempts -lt 50 ]; do
  if [ -z "$parent_pid" ]; then
    parent_pid="$(grep -m 1 '^parent pid=' "$tmpfile" | sed 's/.*=//')"
  fi
  if [ -z "$child_pid" ]; then
    child_pid="$(grep -m 1 '^child pid=' "$tmpfile" | sed 's/.*=//')"
  fi
  if [ -n "$parent_pid" ] && [ -n "$child_pid" ]; then
    break
  fi
  attempts=$((attempts + 1))
  sleep 0.1
done

if [ -z "$parent_pid" ] || [ -z "$child_pid" ]; then
  echo "Error: failed to capture parent/child PIDs" >&2
  echo "Output so far:" >&2
  cat "$tmpfile" >&2
  exit 1
fi

echo "Captured parent PID: $parent_pid"
echo "Captured child PID: $child_pid"

show_maps() {
  label="$1"
  pid="$2"
  echo
  echo "$label /proc/$pid/maps (top lines):"
  if [ ! -r "/proc/$pid/maps" ]; then
    echo "  not readable or process exited" >&2
    return
  fi
  head -n 5 "/proc/$pid/maps"
  echo "$label heap/stack lines:"
  grep -E '\[(heap|stack)\]' "/proc/$pid/maps" || echo "  (none found)"
}

show_maps "Parent" "$parent_pid"
show_maps "Child" "$child_pid"

echo
echo "Note: shared libraries often overlap, while heap/stack ranges differ."
