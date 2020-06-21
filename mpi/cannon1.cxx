//
// Created by xc5 on 6/19/20.
//

#include "mpi.h"
#include <math.h>
#include <iostream>
#include <cstdlib>

void verifyResult(int q, int n, int m, double t, double pt, double st, const int *Z);

using namespace std;

void matrixMultiply(const int * a, const int * b, int rows, int cols, int *c) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      c[i * rows + j] = 0;
      for (int k = 0; k < rows; k++) {
        c[i * rows + j] += a[i * rows + k] * b[k * rows + j];
      }
    }
  }
}

void init(int *z, int n, int m, int q, int x, int y) {
  for (int i = 0; i < m; i++)
    for (int j = 0; j < m; j++)
      if ((x * m) + i < n && y * m + j < n)
        z[i * m + j] = ((x * m) + i) * n + (y * m + j);
      else
        z[i * m + j] = 0;//多出的部分补0
}

void gather(int *Z, int *localC, int rank, int m, int n, int q, int tag) {
  if (rank == 0) {
    int *buffer = new int[m * m];
    MPI_Status status;
    for (int i = 0; i < q * q; i++) {
      int x, y;
      int *e;
      if (i == 0) {
        e = localC;
        x = 0;
        y = 0;
      } else {
        MPI_Recv(buffer, m * m, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
        e = buffer;
        x = status.MPI_SOURCE / q;
        y = status.MPI_SOURCE % q;
        // printf("Source : %d, size = %d\n", i, m * m);
      }
      for (int j = 0; j < m; j++)
        for (int k = 0; k < m; k++) {
          // printf ("Value from rank %d = %d\n", i,  e[j * m + k]);
          int origx = j;
          int origy = k;
          int targx = (x * m + j),
              targy = (y * m + k);
          // printf("x = %d, origx = %d, targx = %d\n", x, origx, targx);
          // printf("y = %d, origy = %d, targy = %d\n", y, origy, targy);
          Z[targx * n + targy] = e[j * m + k];
        }
    }
    delete[]buffer;
  } else {
    MPI_Send(localC, m * m, MPI_INT, 0, tag, MPI_COMM_WORLD);
  }
}

int main(int argc, char **argv) {
  MPI_Comm cartComm;
  int rank, p, q, n, m, x, y;//q*q个处理器P(x,y)，子块大小为m*m
  int *a, *b, *c;
  double t, pt, st;

  int *temp;
  int left, right, up, down;
  int dim[2], period[2], reorder;
  int bCastData[4];
  int coord[2];
  int blockDim;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);

  n = 200;
  q = int(sqrt(double(p)));
  p = q * q;
  if (rank >= p) {
    MPI_Finalize();
    return 0;
  }
  x = rank / q;
  y = rank % q;
  m = n / q + (n % q != 0);
  blockDim = m;

  // printf("rank = %d, n = %d, m = %d,x = %d, y = %d, p = %d, q = %d, blockDim=%d\n", rank, n,m,x,y,p,q, blockDim);

  a = new int[m * m];
  b = new int[m * m];
  c = new int[m * m];
  temp = new int [m * m];
  init(a, n, m, q, x, y);
  init(b, n, m, q, x, y);

  //
//  if (rank == 0) {
//    bCastData[0] = q;
//    bCastData[1] = m / q;
//    bCastData[2] = m;
//    bCastData[3] = m;
//  }
//
//  MPI_Bcast(&bCastData, 4, MPI_INT, 0, MPI_COMM_WORLD);
//  q = bCastData[0];
//  blockDim = bCastData[1];
//  m = bCastData[2];
//  m = bCastData[3];
  dim[0] = q; dim[1] = q;
  period[0] = 1; period[1] = 1;
  reorder = 1;
  MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &cartComm);

  for (int i = 0; i < m; i++)
    for (int j = 0; j < m; j++)
      c[i * m + j] = 0;
  //cannon乘法
  // Initial skew
  MPI_Cart_coords(cartComm, rank, 2, coord);
  MPI_Cart_shift(cartComm, 1, coord[0], &left, &right);
  MPI_Sendrecv_replace(a, blockDim * blockDim, MPI_INT, left, 1, right, 1, cartComm, MPI_STATUS_IGNORE);
  MPI_Cart_shift(cartComm, 0, coord[1], &up, &down);
  MPI_Sendrecv_replace(b, blockDim * blockDim, MPI_INT, up, 1, down, 1, cartComm, MPI_STATUS_IGNORE);

  if (rank == 0)
    t = MPI_Wtime();

  for (int k = 0; k < q; k++) {
    //if (rank == 0) {
      //printf("One exec: L:%d, R:%d, U:%d, D:%d\n", left, right, up, down);
    //}
    // printf("blockDim = %d\n", blockDim);
    matrixMultiply(a, b, blockDim, blockDim, temp);
    for (int i = 0; i < blockDim; i++) {
      for (int j = 0; j < blockDim; j++) {
        c[i * blockDim + j] += temp[i * blockDim + j];
         //printf ("R[%d] : c [%d * %d + %d] += temp [%d * %d + %d] -> %d\n", rank, i, blockDim, j, i, blockDim, j, c[i * blockDim + j]);
      }
    }
    // Shift A once (left) and B once (up)
    MPI_Cart_shift(cartComm, 1, 1, &left, &right);
    MPI_Cart_shift(cartComm, 0, 1, &up, &down);
    MPI_Sendrecv_replace(a, blockDim * blockDim, MPI_INT, left, 1, right, 1, cartComm, MPI_STATUS_IGNORE);
    MPI_Sendrecv_replace(b, blockDim * blockDim, MPI_INT, up, 1, down, 1, cartComm, MPI_STATUS_IGNORE);
  }

  if (rank == 0)
    pt = MPI_Wtime() - t;

  int *Z = NULL;
  if (rank == 0)
    Z = new int[q * q * m * m];
  gather(Z, c, rank, m, n, q, 0);
  if (rank == 0) {
    verifyResult(q, n, m, t, pt, 0, Z);
  }
  delete[] a;
  delete[] b;
  delete[] c;
  MPI_Finalize();
  return 0;
}

void verifyResult(int q, int n, int m, double t, double pt, double st, const int *Z) {
//  for (int i = 0; i < n; i++) {
//    for (int j = 0; j < n; j++)
//      cout << Z[i * n + j] << "\t";
//    cout << endl;
//  }
  int *A, *B, *C;
  A = new int[n * n];
  B = new int[n * n];
  C = new int[n * n];
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) {
      A[i * n + j] = i * n + j;
      B[i * n + j] = i * n + j;
      C[i * n + j] = 0;
    }
  t = MPI_Wtime(); // serial time point 1
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      for (int k = 0; k < n; k++)
        C[i * n + j] += A[i * n + k] * B[k * n + j];
  st = MPI_Wtime() - t; // serial time point 2
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      if (C[i * n + j] != Z[i * q * m + j]) {
        cout << "error!" << C[i * n + j] << " != " << Z[i * q * m + j] << endl;
        exit(1);
      }
  delete[] A;
  delete[] B;
  delete[] C;
  delete[] Z;
  cout << "parallel execution time is " << pt << "s" << endl;
  cout << "serial execution time is " << st << "s" << endl;
  cout << "speedup is " << st / pt << endl;
}
