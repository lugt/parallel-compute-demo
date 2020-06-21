//
// Created by xc5 on 6/21/20.
//

#ifndef MPIS_GAUSS_H
#define MPIS_GAUSS_H

#include <vector>
#include <mpi.h>

// Sorts the input row into chunks to be scattered two all the processors.
void sort(std::vector<double> list2, std::vector<double>& list1, size_t size);

class Gauss {
public:
    using TOTALTYPE = std::vector<std::vector<double>>;
    double determinant() const;
    void operator()();
    Gauss(TOTALTYPE data, size_t rows, size_t cols, int id, int s);
    std::vector<double> sending;
    double d{0.0};
    std::vector<std::vector<double>> dat;
    size_t r{0};
    size_t c{0};
    int id{0};
    int size{0};
};

#endif //MPIS_GAUSS_H
