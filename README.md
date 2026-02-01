# CUDA Terrain Generator: CPU vs GPU Benchmark

Analisi prestazionale e implementazione di un generatore procedurale di terreni 3D basato sull'algoritmo **Fractal Brownian Motion (fBm)**. Il progetto confronta l'efficienza di un'esecuzione sequenziale su **CPU** rispetto a un'accelerazione massiva su **GPU** utilizzando la piattaforma **NVIDIA CUDA**.

## 📌 Descrizione del Progetto
L'obiettivo è generare Heightmaps (mappe di altezza) realistiche simulando l'erosione naturale e la casualità topografica. Il sistema calcola il valore di altezza per ogni pixel $(x, y)$ sommando diverse ottave di **Value Noise** interpolato.

L'algoritmo implementa la funzione matematica frattale:
$$f(p) = \sum_{i=0}^{n} A_i \cdot \text{noise}(p \cdot F_i)$$
Dove:
* $A$ (Ampiezza) dimezza a ogni iterazione (Persistance).
* $F$ (Frequenza) raddoppia a ogni iterazione (Lacunarity).
* Il risultato viene mappato su una palette cromatica dinamica (Acqua, Pianura, Montagna) e salvato in formato BMP.

## 🛠️ Strategie di Ottimizzazione
Trattandosi di un problema **Compute-Bound** (alta intensità aritmetica per pixel), l'approccio parallelo è stato ottimizzato come segue:
* **Massive Parallelism**: Ogni pixel della mappa è gestito da un thread indipendente, eliminando i cicli nidificati della CPU.
* **Intrinsic Math Functions**: Utilizzo di funzioni `__device__` ottimizzate (`sinf`, `floorf`, `fabsf`) per sfruttare le unità di calcolo in virgola mobile (SFU) della GPU.
* **Coordinate Mapping**: Mappatura diretta da coordinate 2D (Blocco/Thread) a memoria lineare 1D per garantire accessi coalescenti alla memoria globale (Global Memory Coalescing).
* **Occupancy Tuning**: Il benchmark testa diverse configurazioni di `BlockSize` per identificare il bilanciamento ideale tra numero di thread e utilizzo dei registri per SM (Streaming Multiprocessor).

## 📊 Performance & Risultati
I test sono stati condotti confrontando un'esecuzione single-thread su CPU rispetto a diverse configurazioni kernel su GPU, processando mappe fino a risoluzione **4K (4096 x 4096 pixel)**, per un totale di 16 milioni di punti calcolati.

| Configurazione (Mappa 4K) | Tempo Medio (s) | Speedup | Note |
| :--- | :---: | :---: | :--- |
| **Sequenziale (CPU)** | 3.9908 | **1.0x** | Baseline Lineare |
| **CUDA (BS: 32)** | 0.2059 | **19.3x** | Sottoutilizzo (Warp size) |
| **CUDA (BS: 128)** | 0.0741 | **53.8x** | **Best Performance** |
| **CUDA (BS: 256)** | 0.1342 | **29.7x** | Register Pressure |

### Analisi dello Speedup
Il sistema raggiunge uno speedup massimo di **~54x**. Mentre la CPU scala linearmente ($O(N)$ puro) impiegando circa 4 secondi per la mappa 4K, la GPU abbatte il tempo a soli **74 millisecondi**, permettendo la generazione in real-time.
È stato osservato che configurazioni con troppi thread per blocco (256) causano un degrado delle prestazioni (da 0.07s a 0.13s) dovuto alla **Register Pressure**, che limita l'occupancy e forza lo swapping in memoria globale.

## 🚀 Getting Started

### Requisiti
* **CUDA Toolkit** 11.x o superiore
* Compilatore C++17 compatibile (MSVC o GCC)
* **CMake** 3.28 o superiore
* GPU NVIDIA con Compute Capability appropriata

### Build & Run
1.  Clona il repo: `git clone https://github.com/tuo-username/CUDA-Terrain-Generator.git`
2.  Crea la cartella build: `mkdir build && cd build`
3.  Genera i file con CMake: `cmake ..`
4.  Compila il progetto: `cmake --build . --config Release`
5.  Esegui l'applicativo:
    * Seleziona **Mode 1** per generazione singola.
    * Seleziona **Mode 2** per avviare la suite di benchmark automatica.

## 📂 Struttura del Repository
* `src/main.cpp`: Orchestratore del benchmark e gestione I/O.
* `src/gpu_terrain.cu`: Kernel CUDA e wrapper host per la gestione device.
* `src/cpu_terrain.cpp`: Implementazione sequenziale di riferimento (Gold Standard).
* `src/utils.cpp`: Gestione export immagini BMP e buffer binari.
* `include/constants.h`: Parametri condivisi per l'algoritmo di hashing procedurale.
* `results/`: Output automatico delle immagini generate e report `benchmark_report.txt`.

---
**Autore:** [Tuo Nome]
*Progetto per il corso di Parallel Computing.*
