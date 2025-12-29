#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osuptime_plus.sh [--help]

Show uptime, load average, and top CPU/MEM processes.
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

echo "Uptime:"
if command -v uptime >/dev/null 2>&1; then
  if uptime -p >/dev/null 2>&1; then
    echo "  $(uptime -p)"
  else
    echo "  $(uptime)"
  fi
else
  echo "  not available"
fi

if [ -f /proc/loadavg ]; then
  load_avg="$(awk '{print $1" "$2" "$3}' /proc/loadavg)"
  echo "Load average: $load_avg"
else
  echo "Load average: not available"
fi

echo "Top 5 CPU processes:"
if command -v ps >/dev/null 2>&1; then
  ps -eo pid,comm,%cpu --sort=-%cpu | head -n 6
else
  echo "  ps not available"
fi

echo "Top 5 MEM processes:"
if command -v ps >/dev/null 2>&1; then
  ps -eo pid,comm,%mem --sort=-%mem | head -n 6
else
  echo "  ps not available"
fi
