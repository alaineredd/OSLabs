#pragma once

#include <vector>
#include <complex>

#include "thread.hpp"

using Complex = std::complex<double>;
using Matrix = std::vector<std::vector<Complex>>;

struct MatrixThreadData {
    const Matrix* a;
    const Matrix* b;
    Matrix* result;
    int start_row;
    int end_row;
};

Matrix create_matrix(int rows, int cols);

void* multiply_thread(void* arg);

Matrix multiply_matrices(const Matrix& a, const Matrix& b, int num_threads = 2);

void print_matrix(const Matrix& m);