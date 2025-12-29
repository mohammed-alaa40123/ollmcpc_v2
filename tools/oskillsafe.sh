#!/bin/bash

pid=$1

if [[ -z "$pid" ]]; then
    echo " i need a pid to terminate "
    echo "usage: $0 <pid>"
    exit 1
fi

if ! ps -p "$pid" > /dev/null 2>&1; then
    echo " no process with pid $pid"
    exit 1
fi

owner=$(ps -o user= -p "$pid" | awk '{$1=$1};1')
me=$(whoami)

if [[ "$owner" != "$me" ]]; then
    echo " pid $pid is owned by $owner, not you"
    read -p "still want to try terminating it? [y/n]: " confirm
    [[ "$confirm" != [yY] ]] && exit 1
fi

echo "sending SIGTERM to pid $pid…"
kill -TERM "$pid" 2>/dev/null

if [[ $? -ne 0 ]]; then
    echo "couldn't send SIGTERM "
    exit 1
fi

sleep 1

if ps -p "$pid" > /dev/null 2>&1; then
    echo "pid $pid didn't exit yet…"
    read -p "do you want to force kill it with SIGKILL? [y/n]: " confirm
    if [[ "$confirm" == [yY] ]]; then
        kill -KILL "$pid" 2>/dev/null
        if [[ $? -eq 0 ]]; then
            echo " pid $pid is gone "
        else
            echo "couldn't kill pid $pid… something's weird "
        fi
    else
        echo " leaving pid $pid alive for now"
    fi
else
    echo " pid $pid terminated peacefully "
fi
