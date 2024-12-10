#!/bin/bash

for param in $(seq 1 128); do
    mpiexec -np $param  /mnt/mpitest/mpi_kmp
done