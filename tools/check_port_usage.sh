#!/bin/bash
port=$1
if [ -z "$port" ]; then
    echo "Usage: $0 <port>"
    exit 1
fi
lsof -i :$port || echo "No process on port $port"
