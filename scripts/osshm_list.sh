#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osshm_list.sh [--help]

List shared memory segments (ipcs -m).
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

if ! command -v ipcs >/dev/null 2>&1; then
  echo "Error: ipcs not available" >&2
  exit 1
fi

ipcs -m
