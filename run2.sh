for iteration in $(seq 1 20); do
    echo "Iteration $iteration:"
    /mnt/mpitest/kmp
    echo    # 输出一个空行，区分不同循环的结果
done
