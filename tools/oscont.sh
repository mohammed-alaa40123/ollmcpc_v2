#!/bin/bash

pid=$1

if [[ -z "$pid" ]]; then
    echo " give me a pid "
    echo "usage: $0 <pid>"
    exit 1
fi

if ! ps -p "$pid" > /dev/null 2>&1; then
    echo "there's no process with pid $pid"
    exit 1
fi

owner=$(ps -o user= -p "$pid" | awk '{$1=$1};1')
me=$(whoami)

if [[ "$owner" != "$me" ]]; then
    echo "pid $pid is owned by $owner, not you"
    read -p "you still want to try resuming it? [y/n]: " confirm
    [[ "$confirm" != [yY] ]] && exit 1
fi

kill -CONT "$pid" 2>/dev/null
if [[ $? -ne 0 ]]; then
    echo "couldn't resume pid $pid something went wrong "
    exit 1
fi

sleep 0.2
state=$(ps -o state= -p "$pid" | awk '{$1=$1};1')

if [[ "$state" =~ [RS] ]]; then
  echo " pid $pid is running again  (state $state)"
else
    echo " pid $pid didn't go back to running, it's now $state"
fi