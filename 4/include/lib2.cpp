#include <cmath>

#include "mathlib.h"

#ifdef _WIN32
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

extern "C" {
    EXPORT_API float SinIntegral(float A, float B, float e) {
        float integral = 0.0f;
        float x = A;
        int n = static_cast<int>((B - A) / e);
        
        for (int i = 0; i < n; ++i) {
            integral += (std::sin(x) + std::sin(x + e)) * e / 2.0f;
            x += e;
        }
        
        return integral;
    }
    
    EXPORT_API int GCF(int A, int B) {
        int gcd = 1;
        int min_val = (A < B) ? A : B;
        
        for (int i = 1; i <= min_val; ++i) {
            if (A % i == 0 && B % i == 0) {
                gcd = i;
            }
        }
        
        return gcd;
    }
}