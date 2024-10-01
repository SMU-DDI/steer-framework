#!/bin/bash

valgrind \
    --tool=memcheck \
    --show-error-list=yes \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --log-file="../logs/assess_valgrind_log.txt" \
    ./assess 1000000
