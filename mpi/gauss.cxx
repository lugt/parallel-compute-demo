//
// Created by xc5 on 6/21/20.
//

// Joshua McCarville-Schueths
// Student ID: 12122858
// gaussian.cpp
//
// This program is an implementation of parallel gaussian elimination.
//

#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

#include <mpi.h>

#include "gauss.h"
#include "codetimer.h"

void sortByProcess(std::vector<double> list2, std::vector<double>& list1, size_t size) {
  size_t index = 0;
  for(size_t i = 0; i < size; i++) {
    for(size_t j = i; j < list1.size(); j += size) {
      list1[index] = list2[j];
      index++;
    }
  }
}

namespace {
    void swap(std::vector<std::vector<double>>& list, size_t count, size_t row1, size_t row2) {
      for(size_t i = 0; i < count; i++) {
        std::swap(list[i][row1], list[i][row2]);
      }
    }
}

GaussianEliminator::GaussianEliminator(GaussianEliminator::DataType data, size_t rows, size_t cols, int id, int s)
  : m_data(std::move(data)), m_rows(rows), m_cols(cols), m_id(id), size(s), m_send_buffer(rows)
{}

double GaussianEliminator::determinant() const {
  return m_determinant;
}

void GaussianEliminator::operator()() {
  int cur_control = 0;
  size_t swaps = 0;
  size_t cur_row = 0;
  double det_val = 1.0;
  size_t cur_index = 0;
  for(size_t i = 0; i < m_rows; i++) {
    // Actual Gaussian code here.
    /*Algorithm for Gaussian elimination (with pivoting):
            Start with all the numbers stored in our NxN matrix A.
            For each column p, we do the following (p=1..N)
                First make sure that a(p,p) is non-zero and preferably large:
                Look at the rows in our matrix below row p.  Look at the p'th
                term in each row.  Select the row that has the largest absolute
                value in the p'th term, and swap the p'th row with that one.
                (optionally, you can only bother to do the above step if
                a(p,p) is zero).
            If we were fortunate enough to get a non-zero value for a(p,p),
            then proceed with the following for loop:
            For each row r below p, we do the following (r=p+1..N)
                row(r) = row(r)  -  (a(r,p) / a(p,p)) * row(p)
            End For
            */
    // Find the row to swap with.
    size_t rowSwap;
    if(cur_control == m_id) {
      rowSwap = cur_row;
      double max = m_data[cur_index][cur_row];
      // Find the row to swap with.
      for(size_t j = cur_row + 1; j < m_rows; j++) {
        if(m_data[cur_index][j] > max) {
          rowSwap = j;
          max = m_data[cur_index][j];
        }
      }
    }

    // Find out if you need to swap and then act accordingly.
    MPI_Bcast(&rowSwap, sizeof(size_t), MPI_BYTE, cur_control, MPI_COMM_WORLD);
    if(rowSwap != cur_row) {
      swap(m_data, m_cols, cur_row, rowSwap);
      swaps++;
    }

    if(cur_control == m_id) {
      // Generate the coefficients.
      for(size_t j = cur_row; j < m_rows; j++) {
        m_send_buffer[j] = m_data[cur_index][j] / m_data[cur_index][cur_row];
      }
    }
    // Send and recv the coefficients.
    MPI_Bcast(m_send_buffer.data(), m_rows, MPI_DOUBLE, cur_control, MPI_COMM_WORLD);
    // Apply the coefficients to the data.
    for(size_t j = 0; j < m_data.size(); j++) {
      for(size_t k = cur_row + 1; k < m_rows; k++) {
        m_data[j][k] -= m_data[j][cur_row] * m_send_buffer[k];
      }
    }

    // Update the determinant value.
    if(cur_control == m_id) {
      det_val = det_val * m_data[cur_index][cur_row];
      cur_index++;
    }

    // Increment the row that we are looking at
    // and increment the counter that tells each process where
    // to recv from. The counter resets to zero to give us a
    // "round robin" communication pattern. Probably not very efficient,
    // but it will do for now.
    cur_control++;
    if(cur_control == size) {
      cur_control = 0;
    }
    cur_row++;
  }

  // Reduce all the determinant values from each process
  // with a multiplication operation.
  // Personally I really like the method I used to find the determinant:
  //   1. Each process just keeps multiplying the pivot value into the product.
  //   2. The reduce does a multiply on all of the individual products.
  // So there really is no extra work to find the determinant.
  MPI_Reduce(&det_val, &m_determinant, 1, MPI_DOUBLE, MPI_PROD, 0, MPI_COMM_WORLD);
  // If we did an odd number of row swaps, negate the determinant.
  if(swaps % 2) {
    m_determinant = -m_determinant;
  }
}


std::vector<int> mpi_init(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  int rank;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  return std::vector<int> ({rank, size});
}

auto scatter_data(size_t rows, size_t cols, int rank, int size, std::istream& in) {
  std::vector<std::vector<double>> data(cols, std::vector<double>(rows, 0.0));
  std::vector<double> send_buffer(rows);
  std::vector<double> recv_buffer(cols);
  std::vector<double> file_buffer(rows);
  // Scatter the data.
  for(size_t i = 0; i < rows; i++) {
    if(!rank) {
      for(auto& val : file_buffer) {
        val = int(rand() * 10);
      }
      sortByProcess(file_buffer, send_buffer, size);
    }
    // Scatters the data so that each process gets the next value for their columns.
    MPI_Scatter(send_buffer.data(), cols, MPI_DOUBLE, recv_buffer.data(), cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    for(size_t j = 0; j < cols; j++) {
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
  std::ifstream inFile;

  // Just get the initialization of the program going.
  std::vector<int> mpvec = mpi_init(argc, argv);

  int rank = mpvec[0];
  int size = mpvec[1];

  // If the input file is not given, print message and exit.
  if(argc < 2) {
    if(rank == 0) {
      std::cout << "No input file given." << newl;
    }
    MPI_Finalize();
    return 0;
  }
  // If the root node (0), then open the input file and read in the
  // number of rows.
  size_t num_rows;
  if(!rank) {
    inFile.open(argv[1]);
    inFile >> num_rows;
  }

  // Broadcasts the number of rows to each processor.
  MPI_Bcast(&num_rows, sizeof(size_t), MPI_BYTE, 0, MPI_COMM_WORLD);
  size_t num_cols = num_rows / static_cast<size_t>(size);
  if(rank == 0) {
    std::cout << "Scattering data..." << newl;
  };

  // Scatter the data to all processors
  CodeTimer timer;
  timer.start();
  auto data = scatter_data(num_rows, num_cols, rank, size, inFile);
  timer.stop();
  if(rank == 0) {
    std::cout << "Scattering completed in: " << timer.duration().count() << " sec" << newl;
    std::cout << "Running Gaussian Elimination..." << newl;
  }
  // Begin timing and gaussian elimination.
  GaussianEliminator gaussian(std::move(data), num_rows, num_cols, rank, size);
  MPI_Barrier(MPI_COMM_WORLD);
  auto sTime = MPI_Wtime();
  timer.start();
  gaussian();

  // End timing.
  MPI_Barrier(MPI_COMM_WORLD);
  auto eTime = MPI_Wtime();
  timer.stop();

  auto rTime = eTime - sTime;

  // If root node, output the runtime.
  if(!rank) {
    std::cout << "MPI spent: " << rTime << " sec" << newl;
//    std::cout << "Root node time: " << timer.duration().count() << " sec" << newl;
    std::cout << "Det[...] value: " << gaussian.determinant() << newl;
  }

  // Finalize and exit.
  MPI_Finalize();
  return 0;
}