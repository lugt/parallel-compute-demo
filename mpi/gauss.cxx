//
// Created by xc5 on 6/21/20.
//
#include <mpi.h>
#include "gauss.h"
#include "codetimer.h"
#include <iostream>
#include <vector>
#include <ctime>

using DVEC = std::vector<double>;
using IVEC = std::vector<int>;
using UINT32 = size_t;

void sort(DVEC list2, DVEC& list1, UINT32 size) {
  UINT32 index = 0;
  for(UINT32 i = 0; i < size; i++) {
    for(UINT32 j = i; j < list1.size(); j += size) {
      list1[index] = list2[j];
      index++;
    }
  }
}

namespace {
    void swap(std::vector<DVEC>& list, UINT32 count, UINT32 row1, UINT32 row2) {
      for(UINT32 i = 0; i < count; i++) {
        std::swap(list[i][row1], list[i][row2]);
      }
    }
}

Gauss::Gauss(Gauss::TOTALTYPE data, UINT32 rows, UINT32 cols, int id, int s)
  : dat(std::move(data)), r(rows), c(cols), id(id), size(s), sending(rows)
{}

double Gauss::determinant() const {
  return d;
}

void Gauss::operator()() {
  int cur_control = 0;
  UINT32 swp = 0;
  UINT32 now_r = 0;
  double determ = 1.0;
  UINT32 cur_index = 0;
  for(UINT32 i = 0; i < r; i++) {
    // Locate the row to swap
    UINT32 row_mid;
    if(cur_control == id) {
      row_mid = now_r;
      double max = dat[cur_index][now_r];
      // Find the row to swap with.
      for(UINT32 j = now_r + 1; j < r; j++) {
        if(dat[cur_index][j] > max) {
          row_mid = j;
          max = dat[cur_index][j];
        }
      }
    }

    MPI_Bcast(&row_mid, sizeof(UINT32), MPI_BYTE, cur_control, MPI_COMM_WORLD);
    if(row_mid != now_r) {
      swap(dat, c, now_r, row_mid);
      swp++;
    }
    if(cur_control == id) {
      for(UINT32 j = now_r; j < r; j++) {
        sending[j] = dat[cur_index][j] / dat[cur_index][now_r];
      }
    }
    MPI_Bcast(sending.data(), r, MPI_DOUBLE, cur_control, MPI_COMM_WORLD);
    for(UINT32 j = 0; j < dat.size(); j++) {
      for(UINT32 k = now_r + 1; k < r; k++) {
        dat[j][k] -= dat[j][now_r] * sending[k];
      }
    }
    if(cur_control == id) {
      determ = determ * dat[cur_index][now_r];
      cur_index++;
    }
    cur_control++;
    if(cur_control == size) {
      cur_control = 0;
    }
    now_r++;
  }
  MPI_Reduce(&determ, &d, 1, MPI_DOUBLE, MPI_PROD, 0, MPI_COMM_WORLD);
  if(swp % 2) {
    d = -d;
  }
}

IVEC mpi_init(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  int rank;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  return std::vector<int> ({rank, size});
}

auto Send_data_to_nodes(UINT32 rows, UINT32 cols, int rank, int size) {
  std::vector<DVEC> data(cols, DVEC(rows, 0.0));
  DVEC send_buffer(rows);
  DVEC recv_buffer(cols);
  DVEC file_buffer(rows);
  // Scatter the data.
  for(UINT32 i = 0; i < rows; i++) {
    if(rank == 0) {
      for(auto& val : file_buffer) {
        val = rand();
      }
      sort(file_buffer, send_buffer, size);
    }
    MPI_Scatter(send_buffer.data(), cols, MPI_DOUBLE, recv_buffer.data(), cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    for(UINT32 j = 0; j < cols; j++) {
      data[j][i] = recv_buffer[j];
    }
  }
  return data;
}

auto& newl(std::ostream& out) {
  out << '\n';
  return out;
}

int main(int argc, char* argv[]) {
  IVEC mpvec = mpi_init(argc, argv);
  int rank = mpvec[0];
  int size = mpvec[1];
  if(argc < 2) {
    if(rank == 0) {
      printf("No input file given.\n");
    }
    MPI_Finalize();
    return 0;
  }
  UINT32 rows = 100;
  UINT32 cols = rows / static_cast<UINT32>(size);
  if(rank == 0) {
    printf("Start timer\n");
  };
  MyTTimer timer;
  timer.start();
  auto data = Send_data_to_nodes(rows, cols, rank, size);
  timer.stop();
  if(rank == 0) {
    printf("Send: %lf sec.\n", timer.duration().count());
  }
  Gauss gaussian(std::move(data), rows, cols, rank, size);
  MPI_Barrier(MPI_COMM_WORLD);
  auto begin_time = MPI_Wtime();
  timer.start();
  gaussian();
  MPI_Barrier(MPI_COMM_WORLD);
  auto end_time = MPI_Wtime();
  timer.stop();
  auto total_time = end_time - begin_time;
  // If root node, output the runtime.
  if(rank == 0) {
    printf("MPI spent: %lf sec.\n", total_time);
  }
  // Finalize and exit.
  MPI_Finalize();
  return 0;
}
