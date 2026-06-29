#pragma once
#include "QuadTree.h"
#include <vector>
#include <string>
#include <random>
#include <chrono>

// ─────────────────────────────────────────────
//  Configuración de la simulación
// ─────────────────────────────────────────────

struct SimConfig {
    int    numParticles  = 500;
    double worldW        = 800.0;
    double worldH        = 600.0;
    int    qtCapacity    = 4;
    double minRadius     = 3.0;
    double maxRadius     = 6.0;
    double minSpeed      = 30.0;
    double maxSpeed      = 80.0;
    // 0 = uniforme, 1 = clusters, 2 = zona densa
    int    distribution  = 0;
    bool   bruteForce    = false;  // si true, usar fuerza bruta para colisiones
    unsigned int seed    = 42;     // semilla para aleatoriedad reproducible
};

// ─────────────────────────────────────────────
//  Métricas por frame
// ─────────────────────────────────────────────

struct FrameStats {
    double frameTimeMs    = 0.0;
    int    qtComparisons  = 0;
    int    bfComparisons  = 0;
    int    qtNodesVisited = 0;
    int    collisions     = 0;
    int    numParticles   = 0;
};

// ─────────────────────────────────────────────
//  Resultado de consulta (rect o círculo)
// ─────────────────────────────────────────────

struct QueryResult {
    std::vector<Particle*> qtFound;
    std::vector<Particle*> bfFound;
    int qtNodesVisited  = 0;
    int bfComparisons   = 0;
    double qtTimeUs     = 0.0;
    double bfTimeUs     = 0.0;
    AABB   rectQuery;
    Vec2   circleCenter;
    double circleRadius = 0.0;
    bool   isRect       = true;
    bool   isKNN        = false;
};

// ─────────────────────────────────────────────
//  Datos de experimento benchmark
// ─────────────────────────────────────────────

struct BenchmarkEntry {
    int    n;
    double qtAvgFrameMs;
    double bfAvgFrameMs;
    double qtAvgComparisons;
    double bfAvgComparisons;
    double qtAvgNodesVisited;
    int    distribution;
    std::string distName;
};

// ─────────────────────────────────────────────
//  Simulación
// ─────────────────────────────────────────────

class Simulation {
public:
    explicit Simulation(const SimConfig& cfg);

    void reset(const SimConfig& cfg);
    void update(double dt);          // avanza un frame

    // Consultas manuales
    QueryResult queryRect(const AABB& range);
    QueryResult queryCircle(const Vec2& center, double radius);
    QueryResult queryKNN(const Vec2& center, int k);

    // Modificar simulación
    void addParticle(double x, double y);

    // Benchmark automático
    BenchmarkEntry runBenchmark(int n, int dist, int frames = 60);

    // Getters
    const std::vector<Particle>& particles() const { return particles_; }
    std::vector<Particle>&       particles()       { return particles_; }
    const QuadTree&              quadTree()  const { return *qt_; }
    const FrameStats&            lastStats() const { return lastStats_; }
    const SimConfig&             config()    const { return cfg_; }

    // Nodos del QuadTree para dibujar
    std::vector<const QuadTree::Node*> getNodes() const;

private:
    SimConfig cfg_;
    std::vector<Particle> particles_;
    std::unique_ptr<QuadTree> qt_;
    FrameStats lastStats_;
    std::mt19937 rng_;

    void generateParticles();
    void wrapBounds(Particle& p);
    void rebuildQT();
    int  countQTComparisons();  // aproximado
};
