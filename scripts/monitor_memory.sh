#!/bin/bash

memory_limit=10485760

bash -c "$VGENERATOR_MONITOR_MEMORY_COMMAND" &
cmd_pid=$!

while true; do
    all_pids=$(pgrep -P $cmd_pid)
    for pid in $all_pids; do
        all_pids+=" $(pgrep -P $pid)"
    done
    all_pids="$cmd_pid $all_pids"

    mem_usage=0
    for pid in $all_pids; do
        mem=$(ps --no-headers -o rss --pid $pid 2>/dev/null)
        mem=${mem:-0}
        mem_usage=$((mem_usage + mem))
    done
    
    #echo "Current memory usage: ${mem_usage} KB"
    
    if [ "$mem_usage" -gt "$memory_limit" ]; then
        echo "Memory limit exceeded. Terminating process $cmd_pid. Current memory usage: ${mem_usage}"
        pkill -TERM -P $cmd_pid 
        sleep 1
        pkill -KILL -P $cmd_pid
        kill -9 $cmd_pid
        exit 1
    fi

    if ! ps -p $cmd_pid > /dev/null; then
        wait $cmd_pid
        exit_status=$?
        echo "Command completed with status $exit_status."
        exit $exit_status
    fi

    sleep 1
done
