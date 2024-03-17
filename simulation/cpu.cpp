#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>

#define N 1048576

using uint = unsigned int;

inline void VADD (uint *A, uint *B, uint *C)
{
    for (int i = 0; i < N; ++i) {
        C[i] = A[i] + B[i];
    }
}

inline void VMUL (uint *A, uint *B, uint *C)
{
    for (int i = 0; i < N; ++i) {
        C[i] = A[i] + B[i];
    }
}

inline void VDIV (uint *A, uint *B, uint *C)
{
    for (int i = 0; i < N; ++i) {
        C[i] = A[i] + B[i];
    }
}

int main()
{
    srand((unsigned int)time(NULL));
    uint *A, *B, *C;

    A = new uint[N];
    B = new uint[N];
    C = new uint[N];


    for (int i = 0; i < N; ++i) {
        A[i] = (unsigned int)rand() % (RAND_MAX-1) + 1;
        B[i] = (unsigned int)rand() % (RAND_MAX-1) + 1;
    }

    VADD(A,B,C);
    VMUL(A,B,C);
    VDIV(A,B,C);

    auto t0 = std::chrono::high_resolution_clock::now();
    ///< Add
    for (int i = 0; i < 4000; ++i) {
        VADD(A,B,C);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    ///< Mul
    for (int i = 0; i < 4000; ++i) {
        VMUL(A,B,C);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    ///< Div
    for (int i = 0; i < 4000; ++i) {
        VDIV(A,B,C);
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    auto dtadd = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
    auto dtmul = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    auto dtdiv = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2);
    
    std::cout << "vadd: " << dtadd.count() << std::endl;
    std::cout << "vmul: " << dtmul.count() << std::endl;
    std::cout << "vdiv: " << dtdiv.count() << std::endl;
    delete A;
    delete B;
    delete C;
    return 0;
}
