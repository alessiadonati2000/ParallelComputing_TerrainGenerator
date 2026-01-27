#include "gpu_terrain.cuh"
#include "constants.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cstdio>

// --- FUNZIONI DEVICE (Eseguite dai thread della GPU, chiamata dalla GPU, eseguita sulla GPU) ---

__device__ float gpuRandom2D(float x, float y, int seed) {
    float dot_product = x * (NOISE_DOT_X + seed) + y * (NOISE_DOT_Y - seed);
    // Usiamo le versioni 'f' (sinf, floorf) ottimizzate per float su GPU
    return fabsf(sinf(dot_product) * NOISE_SIN_MUL) - floorf(fabsf(sinf(dot_product) * NOISE_SIN_MUL));
}

__device__ float gpuLerp(float a, float b, float t) {
    return a + t * (b - a);
}

__device__ float gpuValueNoise(float x, float y, int seed) {
    float i_x = floorf(x);
    float i_y = floorf(y);
    float f_x = x - i_x;
    float f_y = y - i_y;

    float u_x = f_x * f_x * (3.0f - 2.0f * f_x);
    float u_y = f_y * f_y * (3.0f - 2.0f * f_y);

    float a = gpuRandom2D(i_x,       i_y,       seed);
    float b = gpuRandom2D(i_x + 1.0f, i_y,       seed);
    float c = gpuRandom2D(i_x,       i_y + 1.0f, seed);
    float d = gpuRandom2D(i_x + 1.0f, i_y + 1.0f, seed);

    return gpuLerp(gpuLerp(a, b, u_x), gpuLerp(c, d, u_x), u_y);
}

__device__ float gpuFbm(float x, float y, int seed) {
    float value = 0.0f;
    float amplitude = 0.5f;
    float frequency = 1.0f;

    // Stessi parametri della CPU per avere lo stesso disegno
    for (int i = 0; i < 6; i++) {
        value += amplitude * gpuValueNoise(x * frequency, y * frequency, seed);
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }
    return value;
}

// --- KERNEL CUDA (Chiamato da Host, eseguito da GPU) ---

__global__ void generateTerrainKernel(float* d_map, int width, int height, int seed) {
    // 1. Calcolo coordinate globali del thread nel piano 2D
    int idx = blockIdx.x * blockDim.x + threadIdx.x; // Colonna (x)
    int idy = blockIdx.y * blockDim.y + threadIdx.y; // Riga (y)

    // 2. Controllo bordi (fondamentale: i thread totali sono spesso più dei pixel)
    if (idx >= width || idy >= height) return;

    // 3. Logica identica alla CPU
    float nx = (float)idx / width * 5.0f;
    float ny = (float)idy / height * 5.0f;

    float h = gpuFbm(nx, ny, seed);

    // 4. Scrittura in memoria globale
    // Mappiamo 2D (x,y) su 1D
    d_map[idy * width + idx] = h;
}

// --- FUNZIONE HOST (Wrapper) ---

void gpuGenerateTerrain(float* h_map, int width, int height, int seed, int blockSizeX, int blockSizeY) {
    size_t size = width * height * sizeof(float);
    float* d_map;

    cudaMalloc((void**)&d_map, size);

    // 1. Allocazione memoria sulla GPU
    cudaError_t err = cudaMalloc((void**)&d_map, size);
    if (err != cudaSuccess) {
        printf("CUDA Error Malloc: %s\n", cudaGetErrorString(err));
        return;
    }

    // 2. Definizione della griglia di lancio
    // Usiamo blocchi 16x16 (256 thread per blocco). È una dimensione standard efficiente.
    dim3 threadsPerBlock(blockSizeX, blockSizeY);
    
    // Calcoliamo quanti blocchi servono per coprire l'immagine.
    // La formula (N + block - 1) / block serve per arrotondare per eccesso (ceiling)
    dim3 numBlocks((width + threadsPerBlock.x - 1) / threadsPerBlock.x,
                   (height + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // 3. Lancio del Kernel
    generateTerrainKernel<<<numBlocks, threadsPerBlock>>>(d_map, width, height, seed);

    // Controllo errori lancio kernel
    err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("CUDA Error Kernel Launch: %s\n", cudaGetErrorString(err));
        cudaFree(d_map);
        return;
    }

    cudaDeviceSynchronize();
    // 4. Copia del risultato dalla GPU alla RAM (Host)
    // Questa funzione è bloccante: aspetta che la GPU finisca.
    cudaMemcpy(h_map, d_map, size, cudaMemcpyDeviceToHost);

    // 5. Pulizia
    cudaFree(d_map);
}