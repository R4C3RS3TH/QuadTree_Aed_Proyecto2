#pragma once
#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>
#include <memory>

// ─────────────────────────────────────────────
//  Tipos geométricos básicos
// ─────────────────────────────────────────────

struct Vec2 {
    double x, y;
    Vec2(double x = 0, double y = 0) : x(x), y(y) {}
    double distTo(const Vec2& o) const {
        double dx = x - o.x, dy = y - o.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

struct AABB {
    double x, y, w, h; // x,y = center; w,h = half-widths
    AABB(double x = 0, double y = 0, double w = 0, double h = 0)
        : x(x), y(y), w(w), h(h) {}

    bool contains(const Vec2& p) const {
        return p.x >= x - w && p.x <= x + w &&
               p.y >= y - h && p.y <= y + h;
    }

    bool intersects(const AABB& o) const {
        return !(o.x - o.w > x + w || o.x + o.w < x - w ||
                 o.y - o.h > y + h || o.y + o.h < y - h);
    }
    // Esquinas (minX, minY, maxX, maxY)
    double minX() const { return x - w; }
    double minY() const { return y - h; }
    double maxX() const { return x + w; }
    double maxY() const { return y + h; }
};

// ─────────────────────────────────────────────
//  Partícula
// ─────────────────────────────────────────────

struct Particle {
    int   id;
    double x, y;
    double vx, vy;
    double radius;
    bool  colliding = false;

    Particle() : id(-1), x(0), y(0), vx(0), vy(0), radius(1.0) {}
    Particle(int id, double x, double y, double vx, double vy, double r)
        : id(id), x(x), y(y), vx(vx), vy(vy), radius(r) {}

    Vec2 pos() const { return {x, y}; }
};

// ─────────────────────────────────────────────
//  Nodo del QuadTree
// ─────────────────────────────────────────────

class QuadTree {
public:
    static const int MAX_DEPTH = 8;

    struct Node {
        AABB boundary;
        int  capacity;
        int  depth;
        bool divided = false;

        std::vector<Particle*> particles;

        // Hijos: NW, NE, SW, SE
        std::unique_ptr<Node> children[4];


        Node(AABB b, int cap, int dep)
            : boundary(b), capacity(cap), depth(dep) {}

        bool insert(Particle* p);
        void subdivide();
        void query(const AABB& range, std::vector<Particle*>& found, int& nodesVisited) const;
        void queryCircle(const Vec2& center, double radius,
                         std::vector<Particle*>& found, int& nodesVisited) const;
        void queryKNN(const Vec2& target, int k,
                      std::vector<Particle*>& best, int& nodesVisited) const;
        void clear();
        void collectNodes(std::vector<const Node*>& out) const;
    };

    // ─── API pública ───
    QuadTree(AABB boundary, int capacity = 4);

    void insert(Particle* p);
    void clear();

    // Consulta rectangular
    std::vector<Particle*> query(const AABB& range, int& nodesVisited) const;

    // Consulta circular (vecinos en radio r)
    std::vector<Particle*> queryCircle(const Vec2& center, double radius, int& nodesVisited) const;

    // Consulta KNN (K vecinos más cercanos)
    std::vector<Particle*> queryKNN(const Vec2& target, int k, int& nodesVisited) const;

    // Detección de colisiones (todos contra todos con QuadTree)
    int detectCollisions(std::vector<Particle>& particles);

    // Acceso a nodos para visualización
    void collectNodes(std::vector<const Node*>& out) const;

    const Node* root() const { return root_.get(); }

private:
    std::unique_ptr<Node> root_;
    int capacity_;
};

// ─────────────────────────────────────────────
//  Solución de fuerza bruta (para comparar)
// ─────────────────────────────────────────────

struct BruteForce {
    // Devuelve cuántas comparaciones hizo
    static std::vector<Particle*> queryRect(
        std::vector<Particle>& particles,
        const AABB& range,
        int& comparisons);

    static std::vector<Particle*> queryCircle(
        std::vector<Particle>& particles,
        const Vec2& center, double radius,
        int& comparisons);

    static std::vector<Particle*> queryKNN(
        std::vector<Particle>& particles,
        const Vec2& target, int k,
        int& comparisons);

    static int detectCollisions(std::vector<Particle>& particles, int& comparisons);
};
