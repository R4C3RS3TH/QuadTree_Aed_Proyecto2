#include "Simulation.h"
#include <cmath>
#include <numeric>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;

static double nowUs() {
    return std::chrono::duration<double, std::micro>(
        Clock::now().time_since_epoch()).count();
}

// ══════════════════════════════════════════════
//  Construcción / Reset
// ══════════════════════════════════════════════

Simulation::Simulation(const SimConfig& cfg)
    : cfg_(cfg), rng_(48) {
    reset(cfg);
}

void Simulation::reset(const SimConfig& cfg) {
    cfg_ = cfg;
    rng_.seed(48);
    generateParticles();
    AABB world(cfg_.worldW / 2, cfg_.worldH / 2, cfg_.worldW / 2, cfg_.worldH / 2);
    qt_ = std::make_unique<QuadTree>(world, cfg_.qtCapacity);
    rebuildQT();
}

// ─── Generadores de distribución ───

static void assignRandomVelocity(Particle& p, const SimConfig& cfg, std::mt19937& rng) {
    double spd = cfg.minSpeed + (rng() % 1000) / 1000.0 * (cfg.maxSpeed - cfg.minSpeed);
    double ang = (rng() % 1000) / 1000.0 * 2.0 * 3.14159265;
    p.vx = spd * std::cos(ang);
    p.vy = spd * std::sin(ang);
}

static void uniformDist(std::vector<Particle>& ps, const SimConfig& cfg, std::mt19937& rng) {
    std::uniform_real_distribution<double> rx(0, cfg.worldW);
    std::uniform_real_distribution<double> ry(0, cfg.worldH);
    std::uniform_real_distribution<double> rr(cfg.minRadius, cfg.maxRadius);

    for (int i = 0; i < cfg.numParticles; i++) {
        ps.emplace_back(i, rx(rng), ry(rng), 0, 0, rr(rng));
        assignRandomVelocity(ps.back(), cfg, rng);
    }
}

static void clusterDist(std::vector<Particle>& ps, const SimConfig& cfg, std::mt19937& rng) {
    const int NUM_CLUSTERS = 5;
    std::uniform_real_distribution<double> cx(cfg.worldW * 0.1, cfg.worldW * 0.9);
    std::uniform_real_distribution<double> cy(cfg.worldH * 0.1, cfg.worldH * 0.9);
    std::normal_distribution<double> spread(0, cfg.worldW * 0.06);
    std::uniform_real_distribution<double> rr(cfg.minRadius, cfg.maxRadius);

    std::vector<Vec2> centers;
    for (int c = 0; c < NUM_CLUSTERS; c++)
        centers.push_back({cx(rng), cy(rng)});

    for (int i = 0; i < cfg.numParticles; i++) {
        Vec2& ctr = centers[i % NUM_CLUSTERS];
        double x = std::clamp(ctr.x + spread(rng), 0.0, cfg.worldW);
        double y = std::clamp(ctr.y + spread(rng), 0.0, cfg.worldH);
        ps.emplace_back(i, x, y, 0, 0, rr(rng));
        assignRandomVelocity(ps.back(), cfg, rng);
    }
}

static void denseDist(std::vector<Particle>& ps, const SimConfig& cfg, std::mt19937& rng) {
    std::uniform_real_distribution<double> rr(cfg.minRadius, cfg.maxRadius);
    // 70% en zona central pequeña, 30% uniforme
    int dense = (int)(cfg.numParticles * 0.7);
    int rest  = cfg.numParticles - dense;

    double cx = cfg.worldW * 0.5, cy = cfg.worldH * 0.5;
    std::normal_distribution<double> dx(cx, cfg.worldW * 0.08);
    std::normal_distribution<double> dy(cy, cfg.worldH * 0.08);
    std::uniform_real_distribution<double> ux(0, cfg.worldW);
    std::uniform_real_distribution<double> uy(0, cfg.worldH);

    for (int i = 0; i < dense; i++) {
        double x = std::clamp(dx(rng), 0.0, cfg.worldW);
        double y = std::clamp(dy(rng), 0.0, cfg.worldH);
        ps.emplace_back(i, x, y, 0, 0, rr(rng));
        assignRandomVelocity(ps.back(), cfg, rng);
    }
    for (int i = 0; i < rest; i++) {
        ps.emplace_back(dense + i, ux(rng), uy(rng), 0, 0, rr(rng));
        assignRandomVelocity(ps.back(), cfg, rng);
    }
}

void Simulation::generateParticles() {
    particles_.clear();
    particles_.reserve(cfg_.numParticles);
    switch (cfg_.distribution) {
        case 0: uniformDist(particles_, cfg_, rng_); break;
        case 1: clusterDist(particles_, cfg_, rng_); break;
        case 2: denseDist  (particles_, cfg_, rng_); break;
        default: uniformDist(particles_, cfg_, rng_);
    }
}

// ══════════════════════════════════════════════
//  Update
// ══════════════════════════════════════════════

void Simulation::wrapBounds(Particle& p) {
    if (p.x < 0)          { p.x += cfg_.worldW; }
    if (p.x > cfg_.worldW){ p.x -= cfg_.worldW; }
    if (p.y < 0)          { p.y += cfg_.worldH; }
    if (p.y > cfg_.worldH){ p.y -= cfg_.worldH; }
}

void Simulation::rebuildQT() {
    qt_->clear();
    for (auto& p : particles_)
        qt_->insert(&p);
}

void Simulation::update(double dt) {
    auto t0 = Clock::now();

    // Mover partículas
    for (auto& p : particles_) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        wrapBounds(p);
        p.colliding = false;
    }

    // Reconstruir QuadTree
    rebuildQT();

    // Detectar colisiones con QuadTree
    int qtComp = 0;
    for (auto& p : particles_) {
        int visited = 0;
        AABB area(p.x, p.y, p.radius * 2 + 2, p.radius * 2 + 2);
        auto cands = qt_->query(area, visited);
        qtComp += visited;
        for (auto* q : cands) {
            if (q->id <= p.id) continue;
            if (p.pos().distTo(q->pos()) < p.radius + q->radius) {
                p.colliding = true;
                q->colliding = true;
                lastStats_.collisions++;
            }
        }
    }

    // Fuerza bruta para métricas
    int bfComp = 0;
    if (cfg_.bruteForce) {
        BruteForce::detectCollisions(particles_, bfComp);
    } else {
        // Estimado: n*(n-1)/2
        int n = (int)particles_.size();
        bfComp = n * (n - 1) / 2;
    }

    auto t1 = Clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    lastStats_.frameTimeMs    = ms;
    lastStats_.qtComparisons  = qtComp;
    lastStats_.bfComparisons  = bfComp;
    lastStats_.qtNodesVisited = qtComp; // aprox
    lastStats_.numParticles   = (int)particles_.size();
}

// ══════════════════════════════════════════════
//  Consultas manuales
// ══════════════════════════════════════════════

QueryResult Simulation::queryRect(const AABB& range) {
    QueryResult r;
    r.isRect    = true;
    r.rectQuery = range;

    double t0 = nowUs();
    r.qtFound = qt_->query(range, r.qtNodesVisited);
    r.qtTimeUs = nowUs() - t0;

    t0 = nowUs();
    r.bfFound = BruteForce::queryRect(particles_, range, r.bfComparisons);
    r.bfTimeUs = nowUs() - t0;

    return r;
}

QueryResult Simulation::queryCircle(const Vec2& center, double radius) {
    QueryResult r;
    r.isRect       = false;
    r.circleCenter = center;
    r.circleRadius = radius;

    double t0 = nowUs();
    r.qtFound = qt_->queryCircle(center, radius, r.qtNodesVisited);
    r.qtTimeUs = nowUs() - t0;

    t0 = nowUs();
    r.bfFound = BruteForce::queryCircle(particles_, center, radius, r.bfComparisons);
    r.bfTimeUs = nowUs() - t0;

    return r;
}

std::vector<const QuadTree::Node*> Simulation::getNodes() const {
    std::vector<const QuadTree::Node*> nodes;
    qt_->collectNodes(nodes);
    return nodes;
}

// ══════════════════════════════════════════════
//  Benchmark
// ══════════════════════════════════════════════

BenchmarkEntry Simulation::runBenchmark(int n, int dist, int frames) {
    SimConfig cfg = cfg_;
    cfg.numParticles = n;
    cfg.distribution = dist;
    cfg.bruteForce   = false; // usamos estimado
    reset(cfg);

    double sumQtMs = 0;
    double sumQtComp = 0, sumBfComp = 0, sumQtNodes = 0;
    const double DT = 1.0 / 60.0;

    for (int f = 0; f < frames; f++) {
        update(DT);
        sumQtMs    += lastStats_.frameTimeMs;
        sumQtComp  += lastStats_.qtComparisons;
        sumBfComp  += lastStats_.bfComparisons;
        sumQtNodes += lastStats_.qtNodesVisited;
    }

    // Medir BF real en un frame
    double t0 = nowUs();
    int bfComp2 = 0;
    BruteForce::detectCollisions(particles_, bfComp2);
    double bfMs = (nowUs() - t0) / 1000.0;

    BenchmarkEntry e;
    e.n                 = n;
    e.qtAvgFrameMs      = sumQtMs / frames;
    e.bfAvgFrameMs      = bfMs;
    e.qtAvgComparisons  = sumQtComp  / frames;
    e.bfAvgComparisons  = (double)bfComp2;
    e.qtAvgNodesVisited = sumQtNodes / frames;
    e.distribution      = dist;
    e.distName          = (dist == 0 ? "Uniforme" : dist == 1 ? "Clusters" : "Zona densa");
    return e;
}
