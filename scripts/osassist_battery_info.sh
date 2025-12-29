#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osassist_battery_saver.sh [--help]

Answer: "How do I make my battery last longer?"
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

percent=""
status=""
time_est=""

if command -v upower >/dev/null 2>&1; then
  device="$(upower -e | grep -i battery | head -n 1)"
  if [ -n "$device" ]; then
    info="$(upower -i "$device")"
    percent="$(printf '%s\n' "$info" | awk -F: '/percentage/ {gsub(/^[ \t]+/,"",$2); print $2; exit}')"
    status="$(printf '%s\n' "$info" | awk -F: '/state/ {gsub(/^[ \t]+/,"",$2); print $2; exit}')"
    time_est="$(printf '%s\n' "$info" | awk -F: '/time to empty/ {gsub(/^[ \t]+/,"",$2); print $2; exit}')"
    if [ -z "$time_est" ]; then
      time_est="$(printf '%s\n' "$info" | awk -F: '/time to full/ {gsub(/^[ \t]+/,"",$2); print $2; exit}')"
    fi
  fi
fi

if [ -z "$percent" ] && [ -z "$status" ]; then
  battery_dir="$(ls -d /sys/class/power_supply/BAT* 2>/dev/null | head -n 1)"
  if [ -n "$battery_dir" ]; then
    if [ -f "$battery_dir/capacity" ]; then
      percent="$(cat "$battery_dir/capacity")%"
    fi
    if [ -f "$battery_dir/status" ]; then
      status="$(cat "$battery_dir/status")"
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

if [ -z "$percent" ] && [ -z "$status" ]; then
  echo "No battery found"
  exit 0
fi

if [ -z "$percent" ]; then
  percent="unknown"
fi
if [ -z "$status" ]; then
  status="unknown"
fi
if [ -z "$time_est" ]; then
  time_est="unknown"
fi

echo "Battery percentage: $percent"
echo "Battery status: $status"
echo "Estimated time: $time_est"

echo
echo "Top CPU processes:"
if command -v ps >/dev/null 2>&1; then
  ps -eo pid,comm,%cpu --sort=-%cpu | head -n 6
else
  echo "ps not available" >&2
fi

echo
echo "Checklist (ranked):"
echo "1) Lower screen brightness / disable keyboard backlight."
echo "2) Close or pause heavy apps (video, games, VMs)."
echo "3) Enable power saver / battery saver mode."
echo "4) Turn off Bluetooth/Wi-Fi if not needed."
echo "5) Pause background sync or backups."

if [ -t 0 ]; then
  echo
  echo "Optional action: stop a PID from the list above (SIGSTOP)."
  printf "Stop a PID now? (y/N): " >&2
  read -r answer
  case "$answer" in
    y|Y)
      printf "Enter PID: " >&2
      read -r pid
      case "$pid" in
        ''|*[!0-9]*)
          echo "Error: PID must be a positive integer" >&2
          exit 1
          ;;
      esac
      if ! kill -0 "$pid" 2>/dev/null; then
        echo "Error: PID not found or not accessible: $pid" >&2
        exit 1
      fi
      printf "Type YES to SIGSTOP PID %s: " "$pid" >&2
      read -r confirm
      if [ "$confirm" = "YES" ]; then
        if kill -STOP "$pid" 2>/dev/null; then
          echo "Sent SIGSTOP to PID $pid."
        else
          echo "Error: failed to SIGSTOP PID $pid" >&2
          exit 1
        fi
      else
        echo "Aborted." >&2
      fi
      ;;
  esac
fi
