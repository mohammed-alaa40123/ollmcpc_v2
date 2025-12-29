#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
src_file="$script_dir/orphan.c"
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
helper="$out_dir/orphan"

if [[ ! -x "$helper" ]]; then
    if [[ -f "$src_file" ]]; then
        echo "compiling the orphan… "
        gcc "$src_file" -o "$helper"
        if [ $? -ne 0 ]; then
            echo "build failed"
            exit 1
        fi
    else
        echo "can't find orphan.c "
        exit 1
    fi
fi

echo "running the orphan  "
$helper &

helper_pid=$!
echo "helper running with pid $helper_pid"

sleep 1

echo "showing process tree and parent info"
children="$(pgrep -P "$helper_pid" 2>/dev/null)"
if [ -n "$children" ]; then
    ps -o pid,ppid,state,cmd --forest -p "$helper_pid" -p $children
else
    ps -o pid,ppid,state,cmd --forest -p "$helper_pid"
fi

echo "watching the child's PPID change "
for i in {1..10}; do
    if ps -p "$helper_pid" > /dev/null 2>&1; then
        ppid=$(ps -o ppid= -p "$helper_pid" | awk '{$1=$1};1')
        echo "child pid $helper_pid current parent pid: $ppid"
    fi
    sleep 1
done

echo "the child should now be adopted by init/systemd "
