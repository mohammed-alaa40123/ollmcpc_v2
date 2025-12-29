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

env | sort | while IFS= read -r line; do
  case "$line" in
    *=*)
      key="${line%%=*}"
      value="${line#*=}"
      upper_key="$(printf '%s' "$key" | tr '[:lower:]' '[:upper:]')"
      case "$upper_key" in
        *PASS*|*PASSWORD*|*TOKEN*|*KEY*|*SECRET*)
          printf '%s=REDACTED\n' "$key"
          ;;
        *)
          printf '%s=%s\n' "$key" "$value"
          ;;
      esac
      ;;
    *)
      printf '%s\n' "$line"
      ;;
  esac
done
