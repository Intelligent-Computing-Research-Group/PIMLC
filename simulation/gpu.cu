#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>

#define N 1048576

using uint = unsigned int;

// CUDA核函数，负责向量加法
__global__ void vectorAddition(const uint* A, const uint* B, uint* C, int size) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < size) {
        C[tid] = A[tid] + B[tid];
    }
}

int main() {
    // 分配向量在主机内存上
    uint* h_A = new uint[N];
    uint* h_B = new uint[N];
    uint* h_C = new uint[N];

    // 初始化向量数据
    for (int i = 0; i < N; ++i) {
        h_A[i] = i;
        h_B[i] = 2 * i;
    }

    // 分配GPU上的设备内存
    uint* d_A, *d_B, *d_C;
    cudaMalloc((void**)&d_A, N * sizeof(uint));
    cudaMalloc((void**)&d_B, N * sizeof(uint));
    cudaMalloc((void**)&d_C, N * sizeof(uint));

    // 将向量数据从主机内存复制到设备内存
    cudaMemcpy(d_A, h_A, N * sizeof(uint), cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, N * sizeof(uint), cudaMemcpyHostToDevice);

    // 定义网格和块的大小
    int blockSize = 256;
    int gridSize = (N + blockSize - 1) / blockSize;

    // 调用CUDA核函数进行向量加法
    for (int i = 0; i < 4000; ++i) {
        vectorAddition<<<gridSize, blockSize>>>(d_A, d_B, d_C, N);
    }

    // 将结果从设备内存复制回主机内存
    cudaMemcpy(h_C, d_C, N * sizeof(int), cudaMemcpyDeviceToHost);

    // 释放内存
    delete[] h_A;
    delete[] h_B;
    delete[] h_C;
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return 0;
}
