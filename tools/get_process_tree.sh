#!/bin/bash
pid=${1:-1}
pstree -p $pid
