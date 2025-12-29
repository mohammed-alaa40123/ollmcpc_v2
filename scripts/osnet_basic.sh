#!/usr/bin/env bash

set -u

usage() {
  cat <<'EOF'
Usage: osnet_basic.sh [--help]

Show basic network status:
  - hostname
  - default route
  - IP addresses
  - DNS servers from /etc/resolv.conf
  - active TCP listeners (ss -lnt if available)
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

if command -v hostname >/dev/null 2>&1; then
  host_name="$(hostname 2>/dev/null)"
elif command -v uname >/dev/null 2>&1; then
  host_name="$(uname -n 2>/dev/null)"
else
  host_name="unknown"
fi
echo "Hostname: $host_name"

default_route=""
if command -v ip >/dev/null 2>&1; then
  default_route="$(ip route show default 2>/dev/null | head -n 1)"
elif command -v route >/dev/null 2>&1; then
  default_route="$(route -n 2>/dev/null | awk '$1=="0.0.0.0" {print $2; exit}')"
fi

if [ -n "$default_route" ]; then
  echo "Default route: $default_route"
else
  echo "Default route: not available (ip/route missing or no default)"
fi

echo "IP addresses:"
if command -v ip >/dev/null 2>&1; then
  ip -o -4 addr show 2>/dev/null | awk '{print "  " $2 " " $4}'
  ip -o -6 addr show 2>/dev/null | awk '{print "  " $2 " " $4}'
elif command -v ifconfig >/dev/null 2>&1; then
  ifconfig 2>/dev/null | awk '/inet / {print "  " $2} /inet6 / {print "  " $2}'
else
  echo "  not available (ip/ifconfig missing)"
fi

if [ -f /etc/resolv.conf ]; then
  dns_list="$(grep -E '^[[:space:]]*nameserver' /etc/resolv.conf | awk '{print $2}')"
  if [ -n "$dns_list" ]; then
    echo "DNS servers:"
    printf '  %s\n' $dns_list
  else
    echo "DNS servers: none found in /etc/resolv.conf"
  fi
else
  echo "DNS servers: /etc/resolv.conf not found"
fi

if command -v ss >/dev/null 2>&1; then
  echo "Active TCP listeners:"
  ss -lnt 2>/dev/null
else
  echo "ss not available"
fi
