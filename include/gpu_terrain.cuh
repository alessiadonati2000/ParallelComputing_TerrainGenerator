#ifndef GPU_TERRAIN_CUH
#define GPU_TERRAIN_CUH

/**
 * @brief Wrapper function: prepara la memoria GPU, lancia il kernel, recupera i risultati.
 * h_map: puntatore alla memoria HOST (RAM del PC) dove salveremo il risultato.
*/
void gpuGenerateTerrain(float* h_map, int width, int height, int seed, int blockSizeX, int blockSizeY);

#endif // GPU_TERRAIN_CUH