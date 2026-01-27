#include "utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>

void saveToBMP(const float* map, int width, int height, const std::string& filename, float waterLevel, float mountainLevel) {

    // Creazione della cartella
    std::filesystem::path filepath(filename);
    if (filepath.has_parent_path()) {
        if (!std::filesystem::exists(filepath.parent_path())) {
            std::filesystem::create_directories(filepath.parent_path());
        }
    }

    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Errore: Impossibile creare il file " << filename << std::endl;
        return;
    }

    // Calcolo header BPM
    // In BMP ogni riga deve essere multipla di 4 byte.
    const int paddingAmount = ((4 - (width * 3) % 4) % 4);
    const int fileHeaderSize = 14;
    const int informationHeaderSize = 40;
    const int fileSize = fileHeaderSize + informationHeaderSize + (width * 3 + paddingAmount) * height;

    unsigned char fileHeader[fileHeaderSize];
    // File Type BM
    fileHeader[0] = 'B'; fileHeader[1] = 'M';
    // File Size
    fileHeader[2] = fileSize; fileHeader[3] = fileSize >> 8; fileHeader[4] = fileSize >> 16; fileHeader[5] = fileSize >> 24;
    // Reserved
    fileHeader[6] = 0; fileHeader[7] = 0; fileHeader[8] = 0; fileHeader[9] = 0;
    // Offset data
    fileHeader[10] = fileHeaderSize + informationHeaderSize; fileHeader[11] = 0; fileHeader[12] = 0; fileHeader[13] = 0;

    unsigned char informationHeader[informationHeaderSize];
    // Header size
    informationHeader[0] = informationHeaderSize; informationHeader[1] = 0; informationHeader[2] = 0; informationHeader[3] = 0;
    // Width
    informationHeader[4] = width; informationHeader[5] = width >> 8; informationHeader[6] = width >> 16; informationHeader[7] = width >> 24;
    // Height
    informationHeader[8] = height; informationHeader[9] = height >> 8; informationHeader[10] = height >> 16; informationHeader[11] = height >> 24;
    // Planes
    informationHeader[12] = 1; informationHeader[13] = 0;
    // Bits per pixel (RGB)
    informationHeader[14] = 24; informationHeader[15] = 0;
    // Compression etc (all 0)
    for (int i = 16; i < 40; i++) informationHeader[i] = 0;

    file.write(reinterpret_cast<char*>(fileHeader), fileHeaderSize);
    file.write(reinterpret_cast<char*>(informationHeader), informationHeaderSize);

    // Scrittura dei pixel
    unsigned char padding[3] = { 0, 0, 0 };

    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {

            float val = map[y * width + x];
            if (val < 0.0f) val = 0.0f;
            if (val > 1.0f) val = 1.0f;

            unsigned char r, g, b;

            // Colori dinamici
            if (val < waterLevel) {
                // Acqua (Blu)
                r = 0; g = 105; b = 148;
            }
            else if (val < mountainLevel) {
                // Pianura (Verde)
                // Normalizziamo t tra 0 e 1 all'interno della fascia pianura
                float range = mountainLevel - waterLevel;
                float t = (range > 0.0001f) ? (val - waterLevel) / range : 0.0f;

                r = static_cast<unsigned char>(34 + t * 20);
                g = static_cast<unsigned char>(139 + t * 20);
                b = 34;
            }
            else {
                // Montagna (Marrone -> Bianco)
                float range = 1.0f - mountainLevel;
                float t = (range > 0.0001f) ? (val - mountainLevel) / range : 0.0f;

                r = static_cast<unsigned char>(139 + t * (255 - 139));
                g = static_cast<unsigned char>(69 + t * (255 - 69));
                b = static_cast<unsigned char>(19 + t * (255 - 19));
            }

            // BMP scrive in formato BGR
            unsigned char color[3] = { b, g, r };
            file.write(reinterpret_cast<char*>(color), 3);
        }
        // Scrive il padding a fine riga
        file.write(reinterpret_cast<char*>(padding), paddingAmount);
    }

    file.close();
    std::cout << "Mappa salvata: " << filename << std::endl;
}