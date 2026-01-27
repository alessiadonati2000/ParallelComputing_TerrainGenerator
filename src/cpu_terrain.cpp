#include "cpu_terrain.h"
#include "constants.h"
#include <cmath>
#include <algorithm>

/**
* @brief Generatore pseudo-casuale deterministico.
* Dato un input (x,y), restituisce sempre lo stesso float tra 0.0 e 1.0.
* Usa operazioni matematiche pesanti (seno, dot product) per simulare carico di lavoro.
*/
float random2D(float x, float y, int seed) {
    // Modifichiamo le costanti col seed per variare il terreno.
    float dot_product = x * (NOISE_DOT_X + seed) + y * (NOISE_DOT_Y - seed);
    
    // sin(dot) crea un'onda, fract prende solo la parte decimale (il "caos")
    return std::abs(std::sin(dot_product) * NOISE_SIN_MUL) - std::floor(std::abs(std::sin(dot_product) * NOISE_SIN_MUL));
}

/**
 * @brief 2. Interpolazione Lineare (mix)
 * Serve per sfumare tra due valori a e b in base a una percentuale t (0..1)
*/
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

/**
 * @brief Prende una coordinata float, trova i 4 punti interi vicini,
 * genera un valore casuale per loro e interpola il risultato.
*/
float valueNoise(float x, float y, int seed) {
    // Coordinate della cella intera (il "quadrato" della griglia)
    float i_x = std::floor(x);
    float i_y = std::floor(y);
    
    // Coordinate frazionarie (dove siamo dentro il quadrato)
    float f_x = x - i_x;
    float f_y = y - i_y;
    
    // Smoothstep: rende l'interpolazione meno spigolosa (curva a S)
    float u_x = f_x * f_x * (3.0f - 2.0f * f_x);
    float u_y = f_y * f_y * (3.0f - 2.0f * f_y);
    
    // Calcoliamo il valore casuale ai 4 angoli del quadrato
    // a---b
    // |   |
    // c---d
    float a = random2D(i_x,       i_y,       seed);
    float b = random2D(i_x + 1.0f, i_y,       seed);
    float c = random2D(i_x,       i_y + 1.0f, seed);
    float d = random2D(i_x + 1.0f, i_y + 1.0f, seed);
    
    // Interpoliamo orizzontalmente e poi verticalmente
    return lerp(lerp(a, b, u_x), lerp(c, d, u_x), u_y);
}

/**
 * @brief Fractal Brownian Motion (fBm)
 * Somma diversi strati di rumore (ottave) per creare dettagli.
*/
float fbm(float x, float y, int seed) {
    float value = 0.0f;
    float amplitude = 0.5f;
    float frequency = 1.0f;
    
    // 6 Ottave: facciamo il ciclo 6 volte. 
    // Ogni volta il dettaglio è più fine (frequenza alta) ma conta meno (ampiezza bassa).
    for (int i = 0; i < 6; i++) {
        value += amplitude * valueNoise(x * frequency, y * frequency, seed);
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }
    return value;
}

void cpuGenerateTerrain(float* map, int width, int height, int seed) {
    // Loop sequenziale classico: riga per riga, colonna per colonna
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            
            // Normalizziamo le coordinate:
            // Dividendo per width/height otteniamo valori piccoli (es. 0.001)
            // Moltiplichiamo per 5.0f per fare "zoom out" e vedere più terreno.
            float nx = (float)x / width * 5.0f;
            float ny = (float)y / height * 5.0f;
            
            // Calcoliamo l'altezza col Fractal Noise
            float h = fbm(nx, ny, seed);
            
            // Salviamo nell'array lineare
            map[y * width + x] = h;
        }
    }
}