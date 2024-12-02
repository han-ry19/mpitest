for i in $(seq 1 64)
do
  mpiexec -np $i -f /mnt/mpitest/hosts /mnt/mpitest/mpi_kmp
done
