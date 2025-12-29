#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osmem_compare_pids.sh PID1 PID2

Compare mapped file paths between two processes.
EOF
}

if [ $# -eq 0 ] || [ "$1" = "--help" ]; then
  usage
  exit 0
fi

if [ $# -ne 2 ]; then
  echo "Error: expected two PID arguments" >&2
  usage >&2
  exit 1
fi

pid1="$1"
pid2="$2"

for pid in "$pid1" "$pid2"; do
  case "$pid" in
    ''|*[!0-9]*)
      echo "Error: PID must be a positive integer: $pid" >&2
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
done

tmp1="$(mktemp)"
tmp2="$(mktemp)"
tmp_common="$(mktemp)"
cleanup() {
  rm -f "$tmp1" "$tmp2" "$tmp_common"
}
trap cleanup EXIT

extract_paths() {
  awk 'NF>=6 {
    path = $6
    for (i=7; i<=NF; i++) path = path " " $i
    if (path ~ /^\//) print path
  }' "/proc/$1/maps" | sort -u
}

extract_paths "$pid1" > "$tmp1"
extract_paths "$pid2" > "$tmp2"

comm -12 "$tmp1" "$tmp2" > "$tmp_common"

count1="$(wc -l < "$tmp1" | tr -d ' ')"
count2="$(wc -l < "$tmp2" | tr -d ' ')"
count_common="$(wc -l < "$tmp_common" | tr -d ' ')"

echo "PID $pid1 unique mapped files: $count1"
echo "PID $pid2 unique mapped files: $count2"
echo "Common mapped files: $count_common"

if [ "$count_common" -gt 0 ]; then
  echo "Top common mapped files:"
  head -n 20 "$tmp_common"
  if [ "$count_common" -gt 20 ]; then
    echo "... showing first 20 of $count_common"
  fi
fi
