#include <mpi.h>
#include <stdio.h>
int main(int argc, char**argv){
    //openMPI的初始化函数
    MPI_Init(&argc, &argv);
    int world_size, wrank;
    //获取容器中进程数
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    //获取当前进程标识
    MPI_Comm_rank(MPI_COMM_WORLD, &wrank);
    printf("Hello world. I'm the process %d, we are %d process in total\n", wrank, world_size);
    //openMPI的结束函数
    MPI_Finalize();
    return 0;
}
