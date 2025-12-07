#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "mathlib.h"

int main() {
    
    std::string line;
    while (true) {
        std::getline(std::cin, line);
        
        if (line.empty())
            continue;
        
        if (line == "exit") {
            std::cout << "Выход из программы" << std::endl;
            break;
        }
        
        if (line == "0") {
            continue;
        }
        
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "1") {
            float A, B, e;
            if (iss >> A >> B >> e) {
                if (e <= 0) {
                    std::cout << "отр шаг" << std::endl;
                    continue;
                }
                
                float result = SinIntegral(A, B, e);
                std::cout << "интеграл sin(x)dx от " << A << " до " << B 
                         << " с шагом " << e << " = " << result << std::endl;
            } else {
                std::cout << "Используйте: 1 A B e" << std::endl;
            }
        }
        else if (cmd == "2") {
            int A, B;
            if (iss >> A >> B) {
                if (A <= 0 || B <= 0) {
                    std::cout << "ненатурал числа" << std::endl;
                    continue;
                }
                
                int result = GCF(A, B);
                std::cout << "НОД(" << A << ", " << B << ") = " << result << std::endl;
            } else {
                std::cout << "Используйте: 2 A B" << std::endl;
            }
        }
        else {
            std::cout << "такого нет" << std::endl;
        }
    }
    
    return 0;
}