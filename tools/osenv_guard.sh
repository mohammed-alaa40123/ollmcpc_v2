#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osenv_guard.sh [--help]

Print environment variables sorted by name, redacting sensitive values.

Exit codes:
  0  Success
  1  Usage error
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

env | sort | while IFS= read -r line; do
  if [ "${line#*=}" != "$line" ]; then
    key="${line%%=*}"
    value="${line#*=}"
    upper_key="$(printf '%s' "$key" | tr '[:lower:]' '[:upper:]')"
    if [ "${upper_key#*PASS}" != "$upper_key" ] || \
      [ "${upper_key#*PASSWORD}" != "$upper_key" ] || \
      [ "${upper_key#*TOKEN}" != "$upper_key" ] || \
      [ "${upper_key#*KEY}" != "$upper_key" ] || \
      [ "${upper_key#*SECRET}" != "$upper_key" ]; then
      printf '%s=REDACTED\n' "$key"
    else
      printf '%s=%s\n' "$key" "$value"
    fi
  else
    printf '%s\n' "$line"
  fi
done
