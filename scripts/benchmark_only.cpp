/**
 * benchmark_only.cpp
 * Benchmark sin interfaz gráfica – genera tabla y CSV.
 * Compilar: g++ -std=c++17 -O2 -Iinclude src/QuadTree.cpp src/Simulation.cpp scripts/benchmark_only.cpp -o benchmark_only
 */
#include "Simulation.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;

struct Result {
    int    n;
    int    dist;
    std::string distName;
    double qtAvgMs;
    double bfMs;
    double qtAvgComp;
    double bfComp;
    double ratio;
};

Result runOne(int n, int dist, int frames = 60) {
    SimConfig cfg;
    cfg.numParticles = n;
    cfg.distribution = dist;
    cfg.worldW = 960;
    cfg.worldH = 720;
    cfg.qtCapacity = 4;

    Simulation sim(cfg);

    double sumQt = 0, sumQtComp = 0;
    const double DT = 1.0 / 60.0;

    for (int f = 0; f < frames; f++) {
        sim.update(DT);
        sumQt     += sim.lastStats().frameTimeMs;
        sumQtComp += sim.lastStats().qtComparisons;
    }

    // BF real
    int bfComp = 0;
    auto t0 = Clock::now();
    BruteForce::detectCollisions(sim.particles(), bfComp);
    double bfMs = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();

    Result r;
    r.n        = n;
    r.dist     = dist;
    r.distName = (dist == 0 ? "Uniforme" : dist == 1 ? "Clusters" : "ZonaDensa");
    r.qtAvgMs  = sumQt / frames;
    r.bfMs     = bfMs;
    r.qtAvgComp= sumQtComp / frames;
    r.bfComp   = (double)bfComp;
    r.ratio    = (r.qtAvgComp > 0) ? r.bfComp / r.qtAvgComp : 0;
    return r;
}

int main() {
    std::cout << "\n=== BENCHMARK QUADTREE vs FUERZA BRUTA ===\n";
    std::cout << "CS2023 – Algoritmos y Estructuras de Datos\n\n";

    std::vector<int> sizes = {1000, 5000, 10000};
    std::vector<int> dists = {0, 1, 2};

    std::vector<Result> results;

    // Cabecera
    std::cout << std::left
              << std::setw(8)  << "N"
              << std::setw(12) << "Dist"
              << std::setw(12) << "QT(ms)"
              << std::setw(12) << "BF(ms)"
              << std::setw(14) << "Comp.QT"
              << std::setw(14) << "Comp.BF"
              << "Ratio\n";
    std::cout << std::string(86, '-') << "\n";

    for (int n : sizes) {
        for (int d : dists) {
            std::cout << "Corriendo n=" << n << " dist=" << d << "... " << std::flush;
            auto r = runOne(n, d, 60);
            results.push_back(r);

            std::cout << std::left
                      << std::setw(8)  << r.n
                      << std::setw(12) << r.distName
                      << std::setw(12) << std::fixed << std::setprecision(2) << r.qtAvgMs
                      << std::setw(12) << r.bfMs
                      << std::setw(14) << (long long)r.qtAvgComp
                      << std::setw(14) << (long long)r.bfComp
                      << std::setprecision(1) << r.ratio << "x\n";
        }
    }

    // Guardar CSV
    std::ofstream csv("benchmark_results.csv");
    csv << "N,Distribucion,QT_AvgFrameMs,BF_FrameMs,QT_AvgComparisons,BF_Comparisons,Ratio\n";
    for (auto& r : results) {
        csv << r.n << "," << r.distName << ","
            << r.qtAvgMs << "," << r.bfMs << ","
            << (long long)r.qtAvgComp << "," << (long long)r.bfComp << ","
            << r.ratio << "\n";
    }
    csv.close();

    std::cout << "\nResultados guardados en benchmark_results.csv\n";

    // Interpretación
    std::cout << "\n=== INTERPRETACION ===\n";
    for (int i = 0; i < 3; i++) {
        // usar resultado uniforme de cada tamaño
        auto& r = results[i * 3];
        std::cout << "n=" << r.n << ": QuadTree es ~" 
                  << std::fixed << std::setprecision(1) << r.ratio 
                  << "x mas rapido que BF (" << r.distName << ")\n";
    }

    return 0;
}
