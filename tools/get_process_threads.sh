#!/bin/bash
pid=$1
if [ -z "$pid" ]; then
    echo "Usage: $0 <pid>"
    exit 1
fi
ps -L -p $pid -o pid,tid,pcpu,state,comm
