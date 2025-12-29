#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osassist_memory_info.sh [--help]

Memory and battery information.
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

if [ -z "$mem_total" ] || [ -z "$mem_avail" ]; then
  echo "Error: failed to read memory info" >&2
  exit 1
fi

echo "MemTotal: $mem_total kB"
echo "MemAvailable: $mem_avail kB"
echo "SwapTotal: $swap_total kB"
echo "SwapFree: $swap_free kB"

status="OK"
avail_pct=$((mem_avail * 100 / mem_total))
if [ "$avail_pct" -lt 10 ]; then
  status="LOW"
elif [ "$avail_pct" -le 20 ]; then
  status="PRESSURE"
fi
echo "Status: $status"

if [ "$swap_total" -gt 0 ]; then
  swap_pct=$((swap_free * 100 / swap_total))
  if [ "$swap_pct" -lt 20 ]; then
    echo "Warning: swap is low ($swap_free kB free)" >&2
  fi
else
  echo "Warning: no swap configured" >&2
fi

echo
echo "Top memory processes:"
if command -v ps >/dev/null 2>&1; then
  ps -eo pid,comm,%mem --sort=-%mem | head -n 6
else
  echo "ps not available" >&2
fi

echo
echo "Battery info:"
percent=""
status_batt=""
time_est=""

if command -v upower >/dev/null 2>&1; then
  device="$(upower -e | grep -i battery | head -n 1)"
  if [ -n "$device" ]; then
    info="$(upower -i "$device")"
    percent="$(printf '%s\n' "$info" | awk -F: '/percentage/ {gsub(/^[ \t]+/,"",$2); print $2; exit}')"
    status_batt="$(printf '%s\n' "$info" | awk -F: '/state/ {gsub(/^[ \t]+/,"",$2); print $2; exit}')"
    time_est="$(printf '%s\n' "$info" | awk -F: '/time to empty/ {gsub(/^[ \t]+/,"",$2); print $2; exit}')"
    if [ -z "$time_est" ]; then
      time_est="$(printf '%s\n' "$info" | awk -F: '/time to full/ {gsub(/^[ \t]+/,"",$2); print $2; exit}')"
    fi
  fi
fi

if [ -z "$percent" ] && [ -z "$status_batt" ]; then
  battery_dir="$(ls -d /sys/class/power_supply/BAT* 2>/dev/null | head -n 1)"
  if [ -n "$battery_dir" ]; then
    if [ -f "$battery_dir/capacity" ]; then
      percent="$(cat "$battery_dir/capacity")%"
    fi
    if [ -f "$battery_dir/status" ]; then
      status_batt="$(cat "$battery_dir/status")"
    fi
    if [ -f "$battery_dir/time_to_empty_now" ]; then
      time_est="$(cat "$battery_dir/time_to_empty_now") min (to empty)"
    elif [ -f "$battery_dir/time_to_full_now" ]; then
      time_est="$(cat "$battery_dir/time_to_full_now") min (to full)"
    elif [ -f "$battery_dir/energy_now" ] && [ -f "$battery_dir/power_now" ]; then
      time_est="$(awk -v e="$(cat "$battery_dir/energy_now")" -v p="$(cat "$battery_dir/power_now")" \
        'BEGIN { if (p > 0) printf "%.1f hours", e / p; else print "" }')"
    fi
  fi
fi

if [ -z "$percent" ] && [ -z "$status_batt" ]; then
  echo "No battery found"
else
  if [ -z "$percent" ]; then
    percent="unknown"
  fi
  if [ -z "$status_batt" ]; then
    status_batt="unknown"
  fi
  if [ -z "$time_est" ]; then
    time_est="unknown"
  fi
  echo "Battery percentage: $percent"
  echo "Battery status: $status_batt"
  echo "Estimated time: $time_est"
fi
