#!/bin/bash

TARGET_PID="${1:-$$}"

if ! ps -p "$TARGET_PID" > /dev/null 2>&1; then
    echo "PID $TARGET_PID does not exist."
    exit 1
fi

echo "process Tree starting from PID: $TARGET_PID"

ps -o pid,ppid,state,cmd --forest -p "$TARGET_PID" --ppid "$TARGET_PID" \
    --sort pid
