#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <iomanip>
#include <filesystem>
#include "cpu_terrain.h"
#include "gpu_terrain.cuh"
#include "utils.h"

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "./"
#endif

// ========================================================
// 1. PARAMETRI DEL TEST
// ========================================================
const int SEED = 999;
const float WATER_PCT = 40.0f;
const float PLAIN_PCT = 30.0f;
const float MOUNT_PCT = 30.0f;

struct MapConfig {
    int width;
    int height;
    std::string label;
};

struct BlockConfig {
    int x;
    int y;
    int totalThreads;
};

int main() {
    // ========================================================
    // 2. PREPARAZIONE AMBIENTE E TEST
    // ========================================================
    std::string outputDir = std::string(PROJECT_ROOT_DIR) + "/results/";
    std::string reportFile = outputDir + "benchmark_report.txt";

    if (!std::filesystem::exists(outputDir)) {
        std::filesystem::create_directories(outputDir);
    }

    std::ofstream rpt(reportFile);
    if (!rpt.is_open()) {
        std::cerr << "ERRORE CRITICO: Impossibile scrivere su " << reportFile << std::endl;
        return 1;
    }

    // Calcolo soglie colori
    float waterLevel = WATER_PCT / 100.0f;
    float mountainLevel = waterLevel + (PLAIN_PCT / 100.0f);

    // Definizione dei Casi di Test
    std::vector<MapConfig> mapSizes = {
        {1024, 1024, "PICCOLA (1K)"},
        {2048, 2048, "MEDIA   (2K)"},
        {4096, 4096, "GRANDE  (4K)"}
    };

    // Configurazioni Blocchi (Warp size varianti)
    std::vector<BlockConfig> blockConfs = {
        {8, 4, 32},     // Minimo
        {8, 8, 64},     // Medio-Basso
        {16, 8, 128},   // Medio-Alto
        {16, 16, 256}   // Standard CUDA
    };

    // --- INIZIO BENCHMARK ---
    std::cout << "=== GPU TERRAIN GENERATOR: BENCHMARK SUITE ===" << std::endl;
    std::cout << "Seed: " << SEED << " | Output: " << outputDir << "\n" << std::endl;

    rpt << "========================================================================================\n";
    rpt << " REPORT GENERAZIONE TERRENO - PARALLEL COMPUTING\n";
    rpt << "========================================================================================\n";
    rpt << " Seed: " << SEED << "\n";
    rpt << " Configurazione: Acqua " << WATER_PCT << "%, Pianura " << PLAIN_PCT << "%, Montagna " << MOUNT_PCT << "%\n";
    rpt << "========================================================================================\n\n";

    for (const auto& mapConf : mapSizes) {

        std::cout << ">>> Elaborazione Mappa: " << mapConf.label
                  << " (" << mapConf.width << "x" << mapConf.height << ")" << std::endl;

        // Allocazione memoria host
        size_t numPixels = mapConf.width * mapConf.height;
        float* h_cpu = new float[numPixels];
        float* h_gpu = new float[numPixels];

        // ========================================================
        // 3. BASELINE CPU
        // ========================================================
        std::cout << "    [CPU] Generazione Baseline in corso... ";
        auto startCPU = std::chrono::high_resolution_clock::now();
            cpuGenerateTerrain(h_cpu, mapConf.width, mapConf.height, SEED);
        auto endCPU = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> timeCPU = endCPU - startCPU;
        std::cout << "Fatto (" << std::fixed << std::setprecision(4) << timeCPU.count() << "s)" << std::endl;

        // Salvataggio CPU
        std::string cpuFilename = outputDir + "terrain_" + std::to_string(mapConf.width) + "_CPU.bmp";
        saveToBMP(h_cpu, mapConf.width, mapConf.height, cpuFilename, waterLevel, mountainLevel);

        rpt << "----------------------------------------------------------------------------------------\n";
        rpt << " MAPPA: " << mapConf.label << " [" << mapConf.width << "x" << mapConf.height << "]\n";
        rpt << " TEMPO CPU (Baseline): " << std::fixed << std::setprecision(5) << timeCPU.count() << " secondi\n";
        rpt << "----------------------------------------------------------------------------------------\n";
        rpt << std::left << std::setw(20) << "BLOCK DIM (XxY)"
            << std::setw(15) << "THREADS"
            << std::setw(20) << "TEMPO GPU (s)"
            << std::setw(15) << "SPEEDUP (x)" << "\n";
        rpt << "----------------------------------------------------------------------------------------\n";

        // ========================================================
        // 3. VERSIONE GPU
        // ========================================================
        for (const auto& block : blockConfs) {
            std::cout << "    [GPU] Test BlockSize " << block.totalThreads
                      << " (" << block.x << "x" << block.y << ")... ";


            auto startGPU = std::chrono::high_resolution_clock::now();
                gpuGenerateTerrain(h_gpu, mapConf.width, mapConf.height, SEED, block.x, block.y);
            auto endGPU = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> timeGPU = endGPU - startGPU;
            double speedup = timeCPU.count() / timeGPU.count();

            std::cout << "Fatto (" << timeGPU.count() << "s, " << speedup << "x)" << std::endl;
            std::string gpuFilename = outputDir + "terrain_" + std::to_string(mapConf.width) +
                                      "_GPU_bs" + std::to_string(block.totalThreads) + ".bmp";
            saveToBMP(h_gpu, mapConf.width, mapConf.height, gpuFilename, waterLevel, mountainLevel);

            std::string dimStr = std::to_string(block.x) + "x" + std::to_string(block.y);
            rpt << std::left << std::setw(20) << dimStr
                << std::setw(15) << block.totalThreads
                << std::setw(20) << std::fixed << std::setprecision(5) << timeGPU.count()
                << std::setw(15) << std::fixed << std::setprecision(2) << speedup << "\n";
        }
        rpt << "\n";

        delete[] h_cpu;
        delete[] h_gpu;
        std::cout << std::endl;
    }

    std::cout << "=== BENCHMARK COMPLETATO ===" << std::endl;
    std::cout << "Report salvato in: " << reportFile << std::endl;
    rpt.close();

    return 0;
}