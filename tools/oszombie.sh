#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
src_file="$script_dir/zombie.c"
out_dir="$script_dir"
if [ ! -w "$out_dir" ]; then
    out_dir="/tmp/ollmcpc_tools"
    if [ ! -d "$out_dir" ]; then
        mkdir -p "$out_dir" || {
            echo "can't create $out_dir for helper output"
            exit 1
        }
    fi
fi
helper="$out_dir/zombie"

if [[ ! -x "$helper" ]]; then
    if [[ -f "$src_file" ]]; then
        gcc "$src_file" -o "$helper"
        if [ $? -ne 0 ]; then
            echo "build failed"
            exit 1
        fi
    else
        echo "cant find zombie.c "
        exit 1
    fi
fi

echo "running the zombie "
$helper &

helper_pid=$!
echo "helper running with pid $helper_pid"

sleep 2

echo "looking for zombies…"
children="$(pgrep -P "$helper_pid" 2>/dev/null)"
if [ -n "$children" ]; then
    ps -o pid,ppid,state,cmd --forest -p "$helper_pid" -p $children | awk '$3=="Z"{print "zombie detected ->", $0}'
else
    ps -o pid,ppid,state,cmd --forest -p "$helper_pid" | awk '$3=="Z"{print "zombie detected ->", $0}'
fi

echo "waiting a bit for the parent to reap…"
sleep 32  

echo "after cleanup check process tree again:"
children="$(pgrep -P "$helper_pid" 2>/dev/null)"
if [ -n "$children" ]; then
    ps -o pid,ppid,state,cmd --forest -p "$helper_pid" -p $children
else
    ps -o pid,ppid,state,cmd --forest -p "$helper_pid"
fi

echo " zombie should be gone "
