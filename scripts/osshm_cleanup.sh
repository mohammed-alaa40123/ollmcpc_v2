#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osshm_cleanup.sh [--all] [--shmid ID]

Safely remove shared memory segments.
  --all       Remove all segments owned by current user
  --shmid ID  Remove a specific shared memory segment
  --help      Show this help
EOF
}

if [ $# -eq 0 ]; then
  usage >&2
  exit 1
fi

mode=""
shmid=""

while [ $# -gt 0 ]; do
  case "$1" in
    --help)
      usage
      exit 0
      ;;
    --all)
      if [ -n "$mode" ]; then
        echo "Error: only one of --all or --shmid may be used" >&2
        exit 1
      fi
      mode="all"
      ;;
    --shmid)
      if [ -n "$mode" ]; then
        echo "Error: only one of --all or --shmid may be used" >&2
        exit 1
      fi
      shift
      if [ $# -eq 0 ]; then
        echo "Error: --shmid requires an ID" >&2
        exit 1
      fi
      shmid="$1"
      mode="one"
      ;;
    *)
      echo "Error: unexpected argument '$1'" >&2
      usage >&2
      exit 1
      ;;
  esac
  shift
done

if [ -z "$mode" ]; then
  echo "Error: must specify --all or --shmid" >&2
  usage >&2
  exit 1
fi

if ! command -v ipcs >/dev/null 2>&1; then
  echo "Error: ipcs not available" >&2
  exit 1
fi
if ! command -v ipcrm >/dev/null 2>&1; then
  echo "Error: ipcrm not available" >&2
  exit 1
fi

if [ "$mode" = "one" ]; then
  case "$shmid" in
    ''|*[!0-9]*)
      echo "Error: SHMID must be a positive integer" >&2
      exit 1
      ;;
  esac
  echo "WARNING: This will remove shared memory segment ID $shmid." >&2
  printf 'Type YES to continue: ' >&2
  read -r confirm
  if [ "$confirm" != "YES" ]; then
    echo "Aborted." >&2
    exit 1
  fi
  if ! ipcrm -m "$shmid" 2>/dev/null; then
    echo "Error: failed to remove SHMID $shmid" >&2
    exit 1
  fi
  echo "Removed SHMID $shmid."
  exit 0
fi

echo "Shared memory segments (ipcs -m):"
ipcs -m

user_name="$(id -un 2>/dev/null || whoami)"
ids="$(ipcs -m | awk -v user="$user_name" '$1 ~ /^0x/ && $3 == user {print $2}')"

if [ -z "$ids" ]; then
  echo "No shared memory segments owned by user $user_name."
  exit 0
fi

echo "WARNING: This will remove all shared memory segments owned by $user_name." >&2
printf 'Type YES to continue: ' >&2
read -r confirm
if [ "$confirm" != "YES" ]; then
  echo "Aborted." >&2
  exit 1
fi

for id in $ids; do
  if ipcrm -m "$id" 2>/dev/null; then
    echo "Removed SHMID $id."
  else
    echo "Error: failed to remove SHMID $id" >&2
  fi
done
