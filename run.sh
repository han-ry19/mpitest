#!/bin/bash

for iteration in $(seq 1 20); do
    echo "Iteration $iteration:"
    for param in $(seq 1 16); do
        parama=$((2 * param))  # 修正变量赋值语法
        mpiexec -f "/mnt/mpitest/hosts${param}" -np "${parama}" /mnt/mpitest/mpi_kmp
    done
    echo    # 输出一个空行，区分不同循环的结果
done

