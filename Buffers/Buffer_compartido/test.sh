#!/usr/bin/env bash

# Andre Augusto Giannotti Scota (https://sites.google.com/view/a2gs/)

# Script exit if a command fails:
#set -e

# Script exit if a referenced variable is not declared:
#set -u

# If one command in a pipeline fails, its exit code will be returned as the result of the whole pipeline:
#set -o pipefail

# Activate tracing:
#set -x

if [ "$#" -eq 1 ] ;
then

	rm /dev/shm/"$1"

	./prod "$1" 1 > /dev/null &
	sleep 1
	./cons "$1" 2 > /dev/null &
	./cons "$1" 3 > /dev/null &

	watch -n 1 "ls -l /dev/shm/$1; echo \"----\"; lsof /dev/shm/$1; echo \"----\"; hexdump -Cv /dev/shm/$1"

else

	echo "Usage: $0 [QUEUE NAME]"

fi
