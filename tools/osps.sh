#!/bin/bash

ps -eo pid,ppid,state,%cpu,%mem,etime,cmd --sort=-%cpu
