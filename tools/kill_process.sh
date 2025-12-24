#!/bin/bash

pid=$1
if [ -z "$pid" ]; then
    echo "usage: $0 <pid>"
    exit 1
fi

kill -9 "$pid" && echo "process $pid killed"
