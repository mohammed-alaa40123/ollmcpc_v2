#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osecho_plus.sh [--level INFO|WARN|ERROR] [--ts] [--log FILE] "message"

Options:
  --level  Prefix level (INFO, WARN, ERROR)
  --ts     Prefix timestamp (YYYY-MM-DD HH:MM:SS)
  --log    Append final line to file
  --help   Show this help

If no message argument is provided, a single line is read from stdin.
EOF
}

level=""
with_ts=0
log_file=""

while [ $# -gt 0 ]; do
  case "$1" in
    --help)
      usage
      exit 0
      ;;
    --level)
      shift
      if [ $# -eq 0 ]; then
        echo "Error: --level requires a value" >&2
        exit 1
      fi
      case "$1" in
        INFO|WARN|ERROR)
          level="$1"
          ;;
        *)
          echo "Error: invalid level '$1' (use INFO, WARN, or ERROR)" >&2
          exit 1
          ;;
      esac
      ;;
    --ts)
      with_ts=1
      ;;
    --log)
      shift
      if [ $# -eq 0 ]; then
        echo "Error: --log requires a file path" >&2
        exit 1
      fi
      log_file="$1"
      ;;
    --*)
      echo "Error: unknown option '$1'" >&2
      exit 1
      ;;
    *)
      break
      ;;
  esac
  shift
done

message=""
if [ $# -gt 0 ]; then
  message="$*"
else
  if ! IFS= read -r message; then
    message=""
  fi
fi

if [ -z "$message" ]; then
  echo "Error: message required (argument or stdin)" >&2
  exit 1
fi

line="$message"
if [ "$with_ts" -eq 1 ]; then
  timestamp="$(date '+%Y-%m-%d %H:%M:%S')"
  line="$timestamp $line"
fi
if [ -n "$level" ]; then
  line="$level $line"
fi

printf '%s\n' "$line"

if [ -n "$log_file" ]; then
  if ! printf '%s\n' "$line" >> "$log_file"; then
    echo "Error: cannot write to log file '$log_file'" >&2
    exit 1
  fi
fi
