#!/bin/bash

cmd="$1"

if [ $# -eq 0 ]; then
    echo "Mini-OS Commands"
    echo "================"
    echo "PROCESS   : psmini "
    echo "MEMORY    : meminfo "
    echo "THREADS   : threaddemo mutexdemo"
    echo "UTILITIES : cpuusage diskusage uptimemini"
    echo "Usage:"
    echo " oshelp <command> [args]"
    exit 0
fi

case "$cmd" in

psmini)
    if [ $# -eq 1 ]; then
        ps -eo pid,ppid,state,cmd
    else
        echo " psmini"
    fi
;;


meminfo)
    free -h
;;


threaddemo)
    if [ $# -ne 2 ]; then
        echo "SYNTAX: threaddemo <N>"
        exit 1
    fi

    for ((i=1;i<=$2;i++)); do
        (
            echo "Thread $i running (PID $$)"
            sleep 2
        ) &
    done
    wait
;;

mutexdemo)
    lock="/tmp/mini_os_lock"

    exec 200>"$lock"
    flock -x 200

    echo "Mutex acquired by PID $$"
    sleep 3
    echo "Mutex released by PID $$"

    flock -u 200
;;

cpuusage)
    top -bn1 | grep "Cpu(s)" | awk '{print "CPU Usage: " 100-$8 "%"}'
;;

diskusage)
    df -h
;;

uptimemini)
    uptime
;;

esac
