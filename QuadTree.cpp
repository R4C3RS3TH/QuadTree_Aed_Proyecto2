#include "QuadTree.h"

// ══════════════════════════════════════════════
//  QuadTree::Node
// ══════════════════════════════════════════════

bool QuadTree::Node::insert(Particle* p) {
    if (!boundary.contains(p->pos())) return false;

    if ((int)particles.size() < capacity || depth >= MAX_DEPTH) {
        particles.push_back(p);
        return true;
    }

    if (!divided) subdivide();

    for (auto& child : children)
        if (child->insert(p)) return true;

    // Fallback: insertar aquí si no cabe en ningún hijo
    particles.push_back(p);
    return true;
}

void QuadTree::Node::subdivide() {
    double hw = boundary.w / 2.0;
    double hh = boundary.h / 2.0;

    children[0] = std::make_unique<Node>(AABB(boundary.x - hw, boundary.y - hh, hw, hh), capacity, depth + 1); // SW
    children[1] = std::make_unique<Node>(AABB(boundary.x + hw, boundary.y - hh, hw, hh), capacity, depth + 1); // SE
    children[2] = std::make_unique<Node>(AABB(boundary.x - hw, boundary.y + hh, hw, hh), capacity, depth + 1); // NW
    children[3] = std::make_unique<Node>(AABB(boundary.x + hw, boundary.y + hh, hw, hh), capacity, depth + 1); // NE

    divided = true;

    // Redistribuir partículas actuales a hijos
    std::vector<Particle*> remaining;
    for (auto* p : particles) {
        bool placed = false;
        for (auto& child : children) {
            if (child->insert(p)) { placed = true; break; }
        }
        if (!placed) remaining.push_back(p);
    }
    particles = remaining;
}

void QuadTree::Node::query(const AABB& range, std::vector<Particle*>& found, int& nodesVisited) const {
    nodesVisited++;
    if (!boundary.intersects(range)) return;

    for (auto* p : particles)
        if (range.contains(p->pos()))
            found.push_back(p);

    if (divided)
        for (auto& child : children)
            child->query(range, found, nodesVisited);
}

void QuadTree::Node::queryCircle(const Vec2& center, double radius,
                                  std::vector<Particle*>& found, int& nodesVisited) const {
    nodesVisited++;
    // AABB del círculo
    AABB circleBB(center.x, center.y, radius, radius);
    if (!boundary.intersects(circleBB)) return;

    for (auto* p : particles)
        if (p->pos().distTo(center) <= radius)
            found.push_back(p);

    if (divided)
        for (auto& child : children)
            child->queryCircle(center, radius, found, nodesVisited);
}

void QuadTree::Node::clear() {
    particles.clear();
    if (divided) {
        for (auto& c : children) { c->clear(); c.reset(); }
        divided = false;
    }
}

void QuadTree::Node::collectNodes(std::vector<const Node*>& out) const {
    out.push_back(this);
    if (divided)
        for (auto& c : children)
            c->collectNodes(out);
}

// ══════════════════════════════════════════════
//  QuadTree
// ══════════════════════════════════════════════

QuadTree::QuadTree(AABB boundary, int capacity)
    : capacity_(capacity) {
    root_ = std::make_unique<Node>(boundary, capacity, 0);
}

void QuadTree::insert(Particle* p) {
    root_->insert(p);
}

void QuadTree::clear() {
    root_->clear();
}

void QuadTree::rebuild(const std::vector<Particle>& particles) {
    root_->clear();
    // Necesitamos punteros válidos; se usa vector externo
    // Esta función es llamada con el vector real de la simulación
    // para evitar dangling pointers usamos const_cast (los punteros
    // son a elementos del vector original que persiste en Simulation)
    for (auto& p : particles)
        root_->insert(const_cast<Particle*>(&p));
}

std::vector<Particle*> QuadTree::query(const AABB& range, int& nodesVisited) const {
    std::vector<Particle*> found;
    nodesVisited = 0;
    root_->query(range, found, nodesVisited);
    return found;
}

std::vector<Particle*> QuadTree::queryCircle(const Vec2& center, double radius, int& nodesVisited) const {
    std::vector<Particle*> found;
    nodesVisited = 0;
    root_->queryCircle(center, radius, found, nodesVisited);
    return found;
}

int QuadTree::detectCollisions(std::vector<Particle>& particles) {
    int count = 0;
    for (auto& p : particles) p.colliding = false;

    for (auto& p : particles) {
        int visited = 0;
        AABB searchArea(p.x, p.y, p.radius * 2 + 2, p.radius * 2 + 2);
        auto candidates = query(searchArea, visited);

        for (auto* q : candidates) {
            if (q->id <= p.id) continue;
            double dist = p.pos().distTo(q->pos());
            if (dist < p.radius + q->radius) {
                p.colliding = true;
                q->colliding = true;
                count++;
            }
        }
    }
    return count;
}

void QuadTree::collectNodes(std::vector<const Node*>& out) const {
    root_->collectNodes(out);
}

// ══════════════════════════════════════════════
//  BruteForce
// ══════════════════════════════════════════════

std::vector<Particle*> BruteForce::queryRect(
    std::vector<Particle>& particles,
    const AABB& range,
    int& comparisons) {
    std::vector<Particle*> found;
    comparisons = 0;
    for (auto& p : particles) {
        comparisons++;
        if (range.contains(p.pos()))
            found.push_back(&p);
    }
    return found;
}

std::vector<Particle*> BruteForce::queryCircle(
    std::vector<Particle>& particles,
    const Vec2& center, double radius,
    int& comparisons) {
    std::vector<Particle*> found;
    comparisons = 0;
    for (auto& p : particles) {
        comparisons++;
        if (p.pos().distTo(center) <= radius)
            found.push_back(&p);
    }
    return found;
}

int BruteForce::detectCollisions(std::vector<Particle>& particles, int& comparisons) {
    int count = 0;
    comparisons = 0;
    for (auto& p : particles) p.colliding = false;

    for (size_t i = 0; i < particles.size(); i++) {
        for (size_t j = i + 1; j < particles.size(); j++) {
            comparisons++;
            double dist = particles[i].pos().distTo(particles[j].pos());
            if (dist < particles[i].radius + particles[j].radius) {
                particles[i].colliding = true;
                particles[j].colliding = true;
                count++;
            }
        }
    }
    return count;
}
