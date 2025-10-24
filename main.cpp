#include <iostream>
#include <chrono>

#include "matrix.hpp"
#include "thread.hpp"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "not enough args, needed threads";
        return 1;
    }
    int threads_count = std::stoi(argv[1]);
    int rows_a;
    int cols_a;
    int rows_b;
    int cols_b;

    std::cout << "Введите размеры матрицы А\n";
    std::cin >> rows_a >> cols_a;
    std:: cout << "введите размеры матрицы В\n";
    std::cin >> rows_b >> cols_b;
    Matrix A = create_matrix(rows_a, cols_a);
    Matrix B = create_matrix(rows_b, cols_b);
    
    auto begin = std::chrono::steady_clock::now();
    Matrix result = multiply_matrices(A, B, threads_count);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    std::cout << duration << "ms\n";
}