#include <iostream>
#include <iomanip>
#include <vector>

#include "matrix.hpp"

Matrix create_matrix(int rows, int columns) {
    Matrix m(rows, std::vector<Complex>(columns));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            m[i][j] = Complex(i + j, i - j);
        }
    }
    return m;
}

void* multiply_thread(void* arg) {
    MatrixThreadData* data = (MatrixThreadData*)arg;
    const Matrix& a = *(data->a);
    const Matrix& b = *(data->b);
    Matrix& result = *(data->result);
    
    int cols_b = b[0].size();
    int cols_a = a[0].size(); 

    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < cols_b; j++) {
            result[i][j] = 0;
            for (int k = 0; k < cols_a; k++) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    
    return nullptr;
}

Matrix multiply_matrices(const Matrix& a, const Matrix& b, int num_threads) {
    int rows_a = a.size();
    int cols_b = b[0].size();
    
    if (a.empty() || b.empty() || a[0].size() != b.size()) {
        throw std::runtime_error("invalid matrix size");
    }

    Matrix result(rows_a, std::vector<Complex>(cols_b, 0));

    if (num_threads == 1 || rows_a < num_threads) {
        MatrixThreadData data{&a, &b, &result, 0, rows_a};
        multiply_thread(&data);
        return result;
    }

    num_threads = std::min(num_threads, rows_a);

    std::vector<thread::Thread> threads;
    std::vector<MatrixThreadData> thread_data(num_threads);
    int rows_per_thread = rows_a / num_threads;
    int current_row = 0;

    for (int i = 0; i < num_threads; i++) {
        int start = current_row;
        int end = (i == num_threads - 1) ? rows_a : current_row + rows_per_thread;
        thread_data[i] = {&a, &b, &result, start, end};
        threads.emplace_back(multiply_thread);
        threads.back().Run(&thread_data[i]);
        current_row = end;
    }

    for (int i = 0; i < num_threads; i++) {
        threads[i].Join();
    }
    
    return result;
}

void print_matrix(const Matrix& m) {
    for (const auto& row : m) {
        for (const auto& elem : row) {
            std::cout << std::setw(8) << std::fixed << std::setprecision(2)
                      << elem.real() << (elem.imag() >= 0 ? "+" : "") 
                      << elem.imag() << "i ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}