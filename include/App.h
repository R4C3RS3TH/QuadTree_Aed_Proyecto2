#pragma once
#include <SFML/Graphics.hpp>
#include "Simulation.h"
#include <string>
#include <vector>
#include <deque>

// Panel de UI para stats
struct UIPanel {
    sf::RectangleShape bg;
    std::vector<sf::Text> lines;
};

// Modo de la app
enum class AppMode {
    SIMULATE,       // simulación en tiempo real
    QUERY_RECT,     // el usuario dibuja un rectángulo
    QUERY_CIRCLE,   // el usuario define un círculo
    BENCHMARK,      // corriendo experimentos automáticos
    PAUSED
};

// ─────────────────────────────────────────────
//  Renderer / App principal
// ─────────────────────────────────────────────

class App {
public:
    App();
    void run();

private:
    // SFML
    sf::RenderWindow window_;
    sf::Font         font_;
    sf::Clock        clock_;

    // Simulación
    SimConfig        cfg_;
    Simulation       sim_;
    AppMode          mode_ = AppMode::SIMULATE;

    // Interacción
    bool   dragging_   = false;
    sf::Vector2f dragStart_, dragEnd_;
    sf::Vector2f mousePos_;
    double queryRadius_ = 60.0;

    // Resultados de consulta
    QueryResult lastQuery_;
    bool        hasQuery_ = false;

    // Benchmark results
    std::vector<BenchmarkEntry> benchResults_;
    bool benchDone_ = false;
    int  benchStep_ = 0;  // 0..total

    // Graph data (historial de frame time)
    std::deque<float> qtFrameTimes_, bfFrameTimes_;
    static const int  GRAPH_HISTORY = 120;

    // ─── métodos ───
    void processEvents();
    void update();
    void render();

    void drawWorld();
    void drawQuadTreeNodes(const std::vector<const QuadTree::Node*>& nodes);
    void drawParticles();
    void drawQueryOverlay();
    void drawHUD();
    void drawBenchmarkPanel();
    void drawGraph();


    void runBenchmarkAll();

    sf::Text makeText(const std::string& s, unsigned sz, sf::Color col, float x, float y);
    void drawTextShadow(sf::RenderTarget& rt, sf::Text& t);
    sf::Color particleColor(const Particle& p) const;

    // Helpers coordenadas: mundo <-> pantalla
    sf::Vector2f worldToScreen(double wx, double wy) const;
    Vec2 screenToWorld(float sx, float sy) const;

    // Offset de la UI lateral
    static constexpr float UI_W = 320.f;
    static constexpr float WIN_W = 1280.f;
    static constexpr float WIN_H = 720.f;
    float worldOffX() const { return 0.f; }
    float worldW()    const { return WIN_W - UI_W; }
    float worldH()    const { return WIN_H; }
    float scaleX()    const { return worldW() / (float)cfg_.worldW; }
    float scaleY()    const { return worldH() / (float)cfg_.worldH; }
};
