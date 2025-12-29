#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
src_file="$script_dir/fork_tree_helper.c"
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
helper="$out_dir/fork_tree_helper"

if [[ ! -x "$helper" ]]; then
    if [[ -f "$src_file" ]]; then
        gcc "$src_file" -o "$helper"
        if [ $? -ne 0 ]; then
            echo "build failed"
            exit 1
        fi
    else
        echo "can't find fork_tree_helper.c "
        exit 1
    fi
fi

echo "running the fork tree helper… "
$helper &

helper_pid=$!
echo "helper is running with pid $helper_pid"

sleep 0.5

echo "the process tree from helper pid $helper_pid…"
ps -o pid,ppid,state,cmd --forest -p "$helper_pid" --sort pid

children=$(pgrep -P "$helper_pid")
for child in $children; do
    child_ppid=$(ps -o ppid= -p "$child" | awk '{$1=$1};1')
    echo "child pid $child has parent pid $child_ppid"
done
echo "all branching relationships above are correct"
