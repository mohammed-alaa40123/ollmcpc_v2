#!/bin/bash

pid=$1
if [ -z "$pid" ]; then
    echo "usage: $0 <pid>"
    exit 1
fi

ps -o pid,ppid,state,comm -p "$pid"
