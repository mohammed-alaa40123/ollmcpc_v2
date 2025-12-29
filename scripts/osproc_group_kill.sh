#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osproc_group_kill.sh PGID [--force]

Show and terminate all processes in a process group.
  --force  After SIGTERM, send SIGKILL if still running
  --help   Show this help
EOF
}

if [ $# -eq 0 ] || [ "$1" = "--help" ]; then
  usage
  exit 0
fi

pgid="$1"
shift

force=0
while [ $# -gt 0 ]; do
  case "$1" in
    --help)
      usage
      exit 0
      ;;
    --force)
      force=1
      ;;
    *)
      echo "Error: unexpected argument '$1'" >&2
      usage >&2
      exit 1
      ;;
  esac
  shift
done

case "$pgid" in
  ''|*[!0-9]*)
    echo "Error: PGID must be a positive integer" >&2
    exit 1
    ;;
esac

if ! command -v ps >/dev/null 2>&1; then
  echo "Error: ps not available" >&2
  exit 1
fi

list="$(ps -o pid,pgid,cmd | awk -v pgid="$pgid" '
  NR==1 {print; next}
  $2==pgid {print; found=1}
  END {if (!found) exit 1}
')"
if [ $? -ne 0 ]; then
  echo "No processes found for PGID $pgid" >&2
  exit 1
fi

echo "Processes in PGID $pgid:"
printf '%s\n' "$list"

echo "WARNING: This will send SIGTERM to all processes in PGID $pgid." >&2
printf 'Type YES to continue: ' >&2
read -r confirm
if [ "$confirm" != "YES" ]; then
  echo "Aborted." >&2
  exit 1
fi

if ! kill -TERM -- "-$pgid" 2>/dev/null; then
  echo "Error: failed to send SIGTERM to PGID $pgid" >&2
  exit 1
fi
echo "SIGTERM sent to PGID $pgid."

if [ "$force" -eq 1 ]; then
  sleep 2
  if ps -o pgid= -e | awk -v pgid="$pgid" '$1==pgid {found=1} END {exit !found}'; then
    echo "Processes still running; sending SIGKILL." >&2
    if ! kill -KILL -- "-$pgid" 2>/dev/null; then
      echo "Error: failed to send SIGKILL to PGID $pgid" >&2
      exit 1
    fi
    echo "SIGKILL sent to PGID $pgid."
  else
    echo "All processes exited after SIGTERM."
  fi
fi
