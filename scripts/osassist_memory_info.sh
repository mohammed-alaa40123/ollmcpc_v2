#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osassist_ram_or_swap.sh [--help]

Answer: "Should I add RAM, increase swap, or manage processes?"
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
echo "Recommended actions:"
echo "1) Close or pause heavy apps from the list above."
echo "2) Stop a hog temporarily (SIGSTOP) or terminate it (SIGTERM)."
echo "3) Consider increasing swap (manual steps):"
echo "   sudo fallocate -l 2G /swapfile"
echo "   sudo chmod 600 /swapfile"
echo "   sudo mkswap /swapfile && sudo swapon /swapfile"
echo "   echo '/swapfile none swap sw 0 0' | sudo tee -a /etc/fstab"

if [ -t 0 ]; then
  echo
  echo "Optional action: signal a PID from the list above."
  printf "Send signal? (s=SIGSTOP, t=SIGTERM, n=skip): " >&2
  read -r choice
  case "$choice" in
    s|S|t|T)
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
      if [ "$choice" = "s" ] || [ "$choice" = "S" ]; then
        signal="SIGSTOP"
      else
        signal="SIGTERM"
      fi
      printf "Type YES to send %s to PID %s: " "$signal" "$pid" >&2
      read -r confirm
      if [ "$confirm" = "YES" ]; then
        if kill -"$signal" "$pid" 2>/dev/null; then
          echo "Sent $signal to PID $pid."
        else
          echo "Error: failed to send $signal to PID $pid" >&2
          exit 1
        fi
      else
        echo "Aborted." >&2
      fi
      ;;
  esac
fi
