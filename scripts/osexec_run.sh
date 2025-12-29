#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osexec_run.sh COMMAND [args...] [--bg]

Run a command safely, optionally in the background.
  --bg    Run in background and print PID
  --help  Show this help
EOF
}

if [ $# -eq 0 ]; then
  usage >&2
  exit 1
fi

bg=0
args=()

for arg in "$@"; do
  case "$arg" in
    --help)
      usage
      exit 0
      ;;
    --bg)
      bg=1
      ;;
    *)
      args+=("$arg")
      ;;
  esac
done

if [ "${#args[@]}" -eq 0 ]; then
  echo "Error: command required" >&2
  usage >&2
  exit 1
fi

command_name="${args[0]}"
if ! command -v "$command_name" >/dev/null 2>&1; then
  echo "Error: command not found: $command_name" >&2
  exit 1
fi

if [ "$bg" -eq 1 ]; then
  "$command_name" "${args[@]:1}" &
  pid=$!
  echo "Started '$command_name' in background. PID: $pid"
  exit 0
fi

"$command_name" "${args[@]:1}"
status=$?
echo "Exit code: $status"
exit "$status"
