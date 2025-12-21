#!/bin/bash
cmd=$1
if [ -z "$cmd" ]; then
    echo "Usage: $0 <command>"
    exit 1
fi
eval "$cmd"
