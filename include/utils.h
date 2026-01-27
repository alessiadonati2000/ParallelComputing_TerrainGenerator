#ifndef UTILS_H
#define UTILS_H

#include <string>

/**
 * @brief Funzione per salvare l'array di float in un'immagine .BMP
 * map: dati altezza (0.0 - 1.0)
 * filename: percorso file
 * waterLevel: soglia sotto la quale è acqua (0.0 - 1.0)
 * mountainLevel: soglia sopra la quale inizia la montagna (0.0 - 1.0)
 */
void saveToBMP(const float* map, int width, int height, const std::string& filename, float waterLevel, float mountainLevel);

#endif // UTILS_H