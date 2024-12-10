#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    // 初始化MPI环境
    MPI_Init(&argc, &argv);

    // 获取总的进程数
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // 获取当前进程的ID (rank)
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // 获取当前进程所在的处理器名称
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // 打印Hello World消息，显示当前进程ID、总进程数、处理器名称
    std::cout << "Hello world from processor " << processor_name
              << ", rank " << world_rank 
              << " out of " << world_size << " processors" << std::endl;

    // 结束MPI环境
    MPI_Finalize();

    return 0;
}
