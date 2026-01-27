#ifndef CPU_TERRAIN_H
#define CPU_TERRAIN_H

/**
 * @brief Riempie l'array 'map' con valori di altezza tra 0.0 e 1.0.
* width, height: dimensioni
* seed: un numero intero per variare la mappa generata
 */
void cpuGenerateTerrain(float* map, int width, int height, int seed);

#endif // CPU_TERRAIN_H