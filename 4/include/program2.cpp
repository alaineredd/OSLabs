#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "library_loader.h"

using SinIntegralFunc = float(*)(float, float, float);
using GCFFunc = int(*)(int, int);



int main() {
    LibraryLoader loader;
    int currentLib = 1; // 1 - lib1, 2 - lib2
    
    if (!loader.load("gcd_sin1")) {
        std::cerr << "Фатальная ошибка: не удалось загрузить библиотеку lib1" << std::endl;
        return 1;
    }
    
    auto sinIntegral = loader.getFunction<SinIntegralFunc>("SinIntegral");
    auto gcf = loader.getFunction<GCFFunc>("GCF");
    
    if (!sinIntegral || !gcf) {
        std::cerr << "Фатальная ошибка: не удалось получить функции из библиотеки" << std::endl;
        return 1;
    }
    
    std::string line;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);
        
        if (line.empty()) continue;
        
        if (line == "exit") {
            std::cout << "выход" << std::endl;
            break;
        }
        
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "0") {
            if (currentLib == 1) {
                std::cout << "lib1->lib2" << std::endl;
                if (!loader.load("gcd_sin2")) {
                    std::cerr << "не удалось загрузить lib2" << std::endl;
                    continue;
                }
                currentLib = 2;
            } else {
                std::cout << "lib2->lib1" << std::endl;
                if (!loader.load("gcd_sin1")) {
                    std::cerr << "не удалось загрузить lib1" << std::endl;
                    continue;
                }
                currentLib = 1;
            }
            
            sinIntegral = loader.getFunction<SinIntegralFunc>("SinIntegral");
            gcf = loader.getFunction<GCFFunc>("GCF");
            
            if (!sinIntegral || !gcf) {
                std::cerr << "не удалось получить функции из новой библиотеки" << std::endl;
                continue;
            }
        }
        else if (cmd == "1") {
            float A, B, e;
            if (iss >> A >> B >> e) {
                if (e <= 0) {
                    std::cout << "отр шаг" << std::endl;
                    continue;
                }
                
                float result = sinIntegral(A, B, e);
                std::cout << "интеграл sin(x)dx от " << A << " до " << B 
                         << " с шагом " << e << " = " << result << std::endl;
                std::cout << "Метод: " << (currentLib == 1 ? "прямоугольники" : "трапеции") << std::endl;
            } else {
                std::cout << "Ошибка: неверный формат команды" << std::endl;
                std::cout << "Используйте: 1 A B e" << std::endl;
            }
        }
        else if (cmd == "2") {
            int A, B;
            if (iss >> A >> B) {
                if (A <= 0 || B <= 0) {
                    std::cout << "ненатуральные числа" << std::endl;
                    continue;
                }
                
                int result = gcf(A, B);
                std::cout << "НОД(" << A << ", " << B << ") = " << result << std::endl;
                std::cout << "Алгоритм: " << (currentLib == 1 ? "Евклида" : "наивный") << std::endl;
            } else {
                std::cout << "Используйте: 2 A B" << std::endl;
            }
        }
        else {
            std::cout << "Неизвестная команда" << std::endl;
        }
    }
    
    return 0;
}