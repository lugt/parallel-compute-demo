//
// Created by xc5 on 6/20/20.
//
#include "mpi.h"

typedef unsigned int UINT32;
typedef unsigned int UINT;

typedef int INT32;
typedef int INT;

INT main(INT argc, char **argv) {
  INT32 rank, p;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);

  fprintf(stdout, "Worker: %d, of size = %d\n", rank, p);
}