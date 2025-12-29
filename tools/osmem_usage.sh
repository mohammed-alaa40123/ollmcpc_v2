#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osmem_usage.sh [--help]

Show memory summary and a simple status line.
EOF
}

for arg in "$@"; do
  if [ "$arg" = "--help" ]; then
    usage
    exit 0
  fi
done

if [ $# -gt 0 ]; then
  echo "Error: unexpected argument '$1'" >&2
  usage >&2
  exit 1
fi

if [ ! -f /proc/meminfo ]; then
  echo "Error: /proc/meminfo not available" >&2
  exit 1
fi

mem_total="$(awk '/^MemTotal:/ {print $2}' /proc/meminfo)"
mem_avail="$(awk '/^MemAvailable:/ {print $2}' /proc/meminfo)"
swap_total="$(awk '/^SwapTotal:/ {print $2}' /proc/meminfo)"
swap_free="$(awk '/^SwapFree:/ {print $2}' /proc/meminfo)"

if [ -z "$mem_total" ] || [ -z "$mem_avail" ] || [ -z "$swap_total" ] || [ -z "$swap_free" ]; then
  echo "Error: failed to read required fields from /proc/meminfo" >&2
  exit 1
fi

echo "MemTotal: $mem_total kB"
echo "MemAvailable: $mem_avail kB"
echo "SwapTotal: $swap_total kB"
echo "SwapFree: $swap_free kB"

if [ "$mem_total" -le 0 ]; then
  echo "Status: unknown (invalid MemTotal)" >&2
  exit 1
fi

avail_pct=$((mem_avail * 100 / mem_total))
swap_used=$((swap_total - swap_free))

status="OK"
if [ "$avail_pct" -lt 10 ] || [ "$swap_used" -gt 0 ]; then
  status="SWAPPING / LOW"
elif [ "$avail_pct" -ge 10 ] && [ "$avail_pct" -le 20 ]; then
  status="PRESSURE"
fi

echo "Status: $status"
