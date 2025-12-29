#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osshm_list.sh [--help]

List shared memory segments (ipcs -m).
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

if ! command -v ipcs >/dev/null 2>&1; then
  echo "Error: ipcs not available" >&2
  exit 1
fi

ipcs -m
