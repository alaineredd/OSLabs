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
            integral += std::sin(x) * e;
            x += e;
        }
        
        return integral;
    }
    
    EXPORT_API int GCF(int A, int B) {
        while (B != 0) {
            int temp = B;
            B = A % B;
            A = temp;
        }
        return A;
    }
}