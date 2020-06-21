#include "mpi.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

int main(int argc, char *argv[])
{
  const int n = 1000;
  int rank, p;
  int NRA = n, NRB = n, NRC = n;
  int NCA = n, NCB = n, NCC = n;
  
  int(*A)[n];// = new int[n][n];
  int(*B)[n];// = new int[n][n];
  int(*C)[n];//
  A = new int[n][n];
  B = new int[n][n];
  C = new int[n][n];
  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  int numworkers = p - 1;
  double tb, te;

  MPI_Status status;
  int rows, offset, averow, extra, desk, source, mtype, i, j, k;
  if (rank == 0)
  {
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < n; j++)
      {
        A[i][j] = i + j;
        B[i][j] = 1;
      }
    }

    tb = MPI_Wtime();
    if (p > 1) {
      
      printf("Num workers : %d", numworkers);
      averow = n / numworkers;
      extra = n % numworkers;
      offset = 0;
      mtype = FROM_MASTER;
      for (int dest=1; dest<=numworkers; dest++)
      {
        rows = (dest <= extra) ? averow+1 : averow;   	
        // printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
        MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
        MPI_Send(&A[offset][0], rows*NCA, MPI_DOUBLE, dest, mtype,
                 MPI_COMM_WORLD);
        MPI_Send(&B, NCA*NCB, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
        offset = offset + rows;
        // printf("Sent %d rows to task %d offset=%d\n",rows,dest,offset);
        
      }
      /* Receive results from worker tasks */
      mtype = FROM_WORKER;
      for (int i=1; i<=numworkers; i++)
      {
        source = i;
        MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&C[offset][0], rows*NCB, MPI_DOUBLE, source, mtype, 
                 MPI_COMM_WORLD, &status);
        // printf("Received results from task %d\n",source);
      }
      //      printf("GD");
    } else {
      for (int i = 0; i < n; i++)
      {
        for (int j = 0; j < n; j++)
        {
          C[i][j] = 0;
          for (int k = 0; k < n; k++)
            C[i][j] = A[i][k] * B[k][j];
        }
      }
    }
    
    te = MPI_Wtime();
    cout << "Count: " << numworkers << ", time is " << te - tb << "s" << endl;
    
    // delete[] A;
    // delete[] B;
    // delete[] C;
  } else if (p > 1) {
    /**************************** worker task ************************************/
    int mtype = FROM_MASTER;
    MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
    MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
    MPI_Recv(&A, rows*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
    MPI_Recv(&B, NCA*NCB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
    for (k=0; k<n; k++)
      for (i=0; i<rows; i++)
      {
        C[i][k] = 0.0;
        for (j=0; j<NCA; j++)
          C[i][k] = C[i][k] + A[i][j] * B[j][k];
      }
    mtype = FROM_WORKER;
    MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
    MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
    //MPI_Send(&C, rows*NCB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
   
  }
  MPI_Finalize();
  return 0;
}
