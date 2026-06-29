#include "App.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>

// ══════════════════════════════════════════════
//  Colores del tema
// ══════════════════════════════════════════════
namespace Theme {
    const sf::Color BG        {10,  12,  20,  255};
    const sf::Color GRID      {30,  35,  55,  255};
    const sf::Color QT_LINE   {40, 120, 200,  80};
    const sf::Color QT_FILL   {20,  60, 120,  15};
    const sf::Color PARTICLE  {80, 200, 120, 220};
    const sf::Color COLLIDING {255, 80,  80, 255};
    const sf::Color QUERIED   {255,220,  50, 255};
    const sf::Color QUERY_BOX {255,200,  30,  80};
    const sf::Color QUERY_BRD {255,220,  80, 200};
    const sf::Color PANEL_BG  {18,  22,  36, 220};
    const sf::Color ACCENT    {80, 200, 120, 255};
    const sf::Color ACCENT2   {80, 160, 255, 255};
    const sf::Color TEXT      {210,215,230, 255};
    const sf::Color DIM       {100,110,130, 255};
    const sf::Color QT_BAR    {80, 200, 120, 200};
    const sf::Color BF_BAR    {255, 90,  90, 200};
}

// ══════════════════════════════════════════════
//  Constructor
// ══════════════════════════════════════════════

App::App()
    : window_(sf::VideoMode((unsigned)WIN_W, (unsigned)WIN_H),
              "QuadTree – Simulador 2D de Partículas | CS2023",
              sf::Style::Titlebar | sf::Style::Close),
      sim_(cfg_)
{
    window_.setFramerateLimit(60);

    if (!font_.loadFromFile("assets/arial.ttf") &&
        !font_.loadFromFile("C:/Windows/Fonts/arial.ttf") &&
        !font_.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") &&
        !font_.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf") &&
        !font_.loadFromFile("/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf") &&
        !font_.loadFromFile("/usr/share/fonts/opentype/noto/NotoSans-Regular.ttf")) {
        // Intentar cualquier fuente disponible
        std::cerr << "Advertencia: no se encontro fuente TTF; el texto puede no renderizar.\n";
    }

    cfg_.worldW = (double)worldW();
    cfg_.worldH = (double)WIN_H;
    cfg_.numParticles = 300;
    sim_.reset(cfg_);
}

// ══════════════════════════════════════════════
//  Helpers
// ══════════════════════════════════════════════

sf::Vector2f App::worldToScreen(double wx, double wy) const {
    return {(float)(wx * scaleX() + worldOffX()), (float)(wy * scaleY())};
}

Vec2 App::screenToWorld(float sx, float sy) const {
    return {(double)((sx - worldOffX()) / scaleX()), (double)(sy / scaleY())};
}

sf::Text App::makeText(const std::string& s, unsigned sz, sf::Color col, float x, float y) {
    sf::Text t;
    t.setFont(font_);
    t.setString(s);
    t.setCharacterSize(sz);
    t.setFillColor(col);
    t.setPosition(x, y);
    return t;
}

void App::drawTextShadow(sf::RenderTarget& rt, sf::Text& t) {
    auto pos = t.getPosition();
    auto col = t.getFillColor();
    t.setFillColor({0,0,0,120});
    t.setPosition(pos.x+1, pos.y+1);
    rt.draw(t);
    t.setFillColor(col);
    t.setPosition(pos);
    rt.draw(t);
}

sf::Color App::particleColor(const Particle& p) const {
    if (p.colliding) return Theme::COLLIDING;
    // color por velocidad
    double spd = std::sqrt(p.vx*p.vx + p.vy*p.vy);
    double t   = std::min(spd / cfg_.maxSpeed, 1.0);
    sf::Uint8 r = (sf::Uint8)(80  + t * 120);
    sf::Uint8 g = (sf::Uint8)(200 - t * 80);
    sf::Uint8 b = (sf::Uint8)(120 - t * 60);
    return {r, g, b, 220};
}

// ══════════════════════════════════════════════
//  Eventos
// ══════════════════════════════════════════════

void App::processEvents() {
    sf::Event ev;
    while (window_.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) window_.close();

        // Teclado
        if (ev.type == sf::Event::KeyPressed) {
            switch (ev.key.code) {
                case sf::Keyboard::Space:
                    mode_ = (mode_ == AppMode::PAUSED) ? AppMode::SIMULATE : AppMode::PAUSED;
                    break;
                case sf::Keyboard::R:
                    sim_.reset(cfg_);
                    hasQuery_ = false;
                    mode_     = AppMode::SIMULATE;
                    qtFrameTimes_.clear(); bfFrameTimes_.clear();
                    break;
                case sf::Keyboard::Num1:
                    cfg_.distribution = 0; sim_.reset(cfg_); hasQuery_ = false;
                    break;
                case sf::Keyboard::Num2:
                    cfg_.distribution = 1; sim_.reset(cfg_); hasQuery_ = false;
                    break;
                case sf::Keyboard::Num3:
                    cfg_.distribution = 2; sim_.reset(cfg_); hasQuery_ = false;
                    break;
                case sf::Keyboard::Q:
                    mode_ = AppMode::QUERY_RECT;
                    hasQuery_ = false;
                    break;
                case sf::Keyboard::C:
                    mode_ = AppMode::QUERY_CIRCLE;
                    hasQuery_ = false;
                    break;
                case sf::Keyboard::K:
                    mode_ = AppMode::QUERY_KNN;
                    hasQuery_ = false;
                    break;
                case sf::Keyboard::S:
                    mode_ = AppMode::SIMULATE;
                    hasQuery_ = false;
                    break;
                case sf::Keyboard::B:
                    runBenchmarkAll();
                    break;
                case sf::Keyboard::Up:
                    cfg_.numParticles = std::min(cfg_.numParticles + 100, 10000);
                    sim_.reset(cfg_); hasQuery_ = false;
                    break;
                case sf::Keyboard::Down:
                    cfg_.numParticles = std::max(cfg_.numParticles - 100, 50);
                    sim_.reset(cfg_); hasQuery_ = false;
                    break;
                case sf::Keyboard::Left:
                    cfg_.qtCapacity = std::max(cfg_.qtCapacity - 1, 1);
                    sim_.reset(cfg_);
                    break;
                case sf::Keyboard::Right:
                    cfg_.qtCapacity = std::min(cfg_.qtCapacity + 1, 16);
                    sim_.reset(cfg_);
                    break;
                case sf::Keyboard::Escape:
                    window_.close();
                    break;
                default: break;
            }
        }

        // Mouse
        if (ev.type == sf::Event::MouseButtonPressed) {
            float mx = (float)ev.mouseButton.x, my = (float)ev.mouseButton.y;
            if (mx < worldW()) { // dentro del mundo
                if (ev.mouseButton.button == sf::Mouse::Left) {
                    dragging_  = true;
                    dragStart_ = {mx, my};
                    dragEnd_   = {mx, my};
                } else if (ev.mouseButton.button == sf::Mouse::Right) {
                    Vec2 wpos = screenToWorld(mx, my);
                    sim_.addParticle(wpos.x, wpos.y);
                }
            }
        }

        if (ev.type == sf::Event::MouseMoved) {
            mousePos_ = {(float)ev.mouseMove.x, (float)ev.mouseMove.y};
            if (dragging_) dragEnd_ = mousePos_;
        }

        if (ev.type == sf::Event::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left) {
            if (dragging_ && dragStart_.x < worldW()) {
                dragging_ = false;
                float mx = (float)ev.mouseButton.x, my = (float)ev.mouseButton.y;
                dragEnd_ = {mx, my};

                Vec2 wa = screenToWorld(std::min(dragStart_.x, dragEnd_.x),
                                        std::min(dragStart_.y, dragEnd_.y));
                Vec2 wb = screenToWorld(std::max(dragStart_.x, dragEnd_.x),
                                        std::max(dragStart_.y, dragEnd_.y));

                if (mode_ == AppMode::QUERY_RECT) {
                    double cx = (wa.x + wb.x) / 2, cy = (wa.y + wb.y) / 2;
                    double hw = (wb.x - wa.x) / 2, hh = (wb.y - wa.y) / 2;
                    if (hw > 1 && hh > 1) {
                        lastQuery_ = sim_.queryRect(AABB(cx, cy, hw, hh));
                        hasQuery_  = true;
                    }
                } else if (mode_ == AppMode::QUERY_CIRCLE) {
                    Vec2 center = screenToWorld(dragStart_.x, dragStart_.y);
                    // calcular radio en mundo
                    float dsx = dragEnd_.x - dragStart_.x;
                    float dsy = dragEnd_.y - dragStart_.y;
                    double screenR = std::sqrt(dsx*dsx + dsy*dsy);
                    double worldR  = screenR / scaleX();
                    if (worldR > 1) {
                        lastQuery_ = sim_.queryCircle(center, worldR);
                        hasQuery_  = true;
                    }
                } else if (mode_ == AppMode::QUERY_KNN) {
                    Vec2 center = screenToWorld(dragStart_.x, dragStart_.y);
                    lastQuery_ = sim_.queryKNN(center, queryK_);
                    hasQuery_  = true;
                }
            }
        }

        if (ev.type == sf::Event::MouseWheelScrolled) {
            if (mode_ == AppMode::QUERY_KNN) {
                queryK_ = std::max(1, queryK_ + (int)ev.mouseWheelScroll.delta);
            } else {
                queryRadius_ = std::clamp(queryRadius_ + ev.mouseWheelScroll.delta * 5.0, 10.0, 300.0);
            }
        }
    }
}

// ══════════════════════════════════════════════
//  Update
// ══════════════════════════════════════════════

void App::update() {
    float dt = clock_.restart().asSeconds();
    dt = std::min(dt, 0.05f);

    if (mode_ == AppMode::SIMULATE) {
        sim_.update((double)dt);
        auto& s = sim_.lastStats();
        if ((int)qtFrameTimes_.size() >= GRAPH_HISTORY) qtFrameTimes_.pop_front();
        if ((int)bfFrameTimes_.size() >= GRAPH_HISTORY) bfFrameTimes_.pop_front();
        // Estimado BF en ms
        int n = s.numParticles;
        double bfMs = (double)(n*(n-1)/2) * 0.000001; // ~1ns por comparación
        qtFrameTimes_.push_back((float)s.frameTimeMs);
        bfFrameTimes_.push_back((float)bfMs);
    }
}

// ══════════════════════════════════════════════
//  Render
// ══════════════════════════════════════════════

void App::render() {
    window_.clear(Theme::BG);

    // Clip world
    sf::View worldView(sf::FloatRect(0, 0, worldW(), WIN_H));
    worldView.setViewport({0.f, 0.f, worldW()/WIN_W, 1.f});
    window_.setView(worldView);

    drawWorld();
    drawQuadTreeNodes(sim_.getNodes());
    drawParticles();
    if (hasQuery_) drawQueryOverlay();
    if (dragging_) {
        // Preview del drag
        float minX = std::min(dragStart_.x, mousePos_.x);
        float minY = std::min(dragStart_.y, mousePos_.y);
        float dx   = std::abs(mousePos_.x - dragStart_.x);
        float dy   = std::abs(mousePos_.y - dragStart_.y);
        if (mode_ == AppMode::QUERY_RECT) {
            sf::RectangleShape r({dx, dy});
            r.setPosition(minX, minY);
            r.setFillColor(Theme::QUERY_BOX);
            r.setOutlineColor(Theme::QUERY_BRD);
            r.setOutlineThickness(1.5f);
            window_.draw(r);
        } else if (mode_ == AppMode::QUERY_CIRCLE) {
            float radius = std::sqrt(dx*dx + dy*dy);
            sf::CircleShape c(radius);
            c.setOrigin(radius, radius);
            c.setPosition(dragStart_);
            c.setFillColor({255,220,30,40});
            c.setOutlineColor({255,220,80,180});
            c.setOutlineThickness(1.5f);
            window_.draw(c);
        } else if (mode_ == AppMode::QUERY_KNN) {
            sf::CircleShape c(4.f);
            c.setOrigin(4.f, 4.f);
            c.setPosition(dragStart_);
            c.setFillColor({255,100,100,200});
            window_.draw(c);
        }
    }

    // UI panel (coordenadas absolutas)
    sf::View uiView(sf::FloatRect(0, 0, WIN_W, WIN_H));
    window_.setView(uiView);

    // Separador
    sf::RectangleShape sep({2.f, WIN_H});
    sep.setPosition(worldW(), 0);
    sep.setFillColor(Theme::GRID);
    window_.draw(sep);

    drawHUD();
    if (benchDone_) drawBenchmarkPanel();
    drawGraph();

    window_.display();
}

// ─── Fondo/grid ───
void App::drawWorld() {
    sf::RectangleShape bg({worldW(), WIN_H});
    bg.setFillColor(Theme::BG);
    window_.draw(bg);

    // Grid sutil
    int step = 80;
    sf::RectangleShape line;
    line.setFillColor(Theme::GRID);
    for (int x = 0; x < (int)worldW(); x += step) {
        line.setSize({1.f, WIN_H});
        line.setPosition((float)x, 0);
        window_.draw(line);
    }
    for (int y = 0; y < (int)WIN_H; y += step) {
        line.setSize({worldW(), 1.f});
        line.setPosition(0, (float)y);
        window_.draw(line);
    }
}

// ─── Nodos del QuadTree ───
void App::drawQuadTreeNodes(const std::vector<const QuadTree::Node*>& nodes) {
    for (auto* nd : nodes) {
        auto& b = nd->boundary;
        auto  tl = worldToScreen(b.minX(), b.minY());
        float w  = (float)(b.w * 2 * scaleX());
        float h  = (float)(b.h * 2 * scaleY());

        sf::RectangleShape rect({w, h});
        rect.setPosition(tl);
        rect.setFillColor(Theme::QT_FILL);
        rect.setOutlineColor(Theme::QT_LINE);
        rect.setOutlineThickness(0.8f);
        window_.draw(rect);
    }
}

// ─── Partículas ───
void App::drawParticles() {
    for (auto& p : sim_.particles()) {
        auto sc = worldToScreen(p.x, p.y);
        float r  = (float)(p.radius * scaleX());
        sf::CircleShape c(r);
        c.setOrigin(r, r);
        c.setPosition(sc);
        c.setFillColor(particleColor(p));

        // Halo en colisión
        if (p.colliding) {
            sf::CircleShape halo(r + 3.f);
            halo.setOrigin(r+3.f, r+3.f);
            halo.setPosition(sc);
            halo.setFillColor({255,80,80,50});
            window_.draw(halo);
        }
        window_.draw(c);
    }
}

// ─── Overlay de consulta ───
void App::drawQueryOverlay() {
    if (!hasQuery_) return;
    auto& q = lastQuery_;

    if (q.isRect) {
        auto& b = q.rectQuery;
        auto tl = worldToScreen(b.minX(), b.minY());
        float w = (float)(b.w * 2 * scaleX());
        float h = (float)(b.h * 2 * scaleY());
        sf::RectangleShape rect({w, h});
        rect.setPosition(tl);
        rect.setFillColor({255,220,30,35});
        rect.setOutlineColor({255,220,80,220});
        rect.setOutlineThickness(2.f);
        window_.draw(rect);
    } else {
        auto csc = worldToScreen(q.circleCenter.x, q.circleCenter.y);
        float r  = (float)(q.circleRadius * scaleX());
        sf::CircleShape c(r);
        c.setOrigin(r, r);
        c.setPosition(csc);
        c.setFillColor({255,220,30,35});
        c.setOutlineColor({255,220,80,220});
        c.setOutlineThickness(2.f);
        window_.draw(c);
    }

    // Destacar resultados QT
    for (auto* p : q.qtFound) {
        auto sc = worldToScreen(p->x, p->y);
        float r = (float)(p->radius * scaleX()) + 4.f;
        sf::CircleShape c(r);
        c.setOrigin(r, r);
        c.setPosition(sc);
        c.setFillColor({255,220,50,0});
        c.setOutlineColor(Theme::QUERIED);
        c.setOutlineThickness(2.5f);
        window_.draw(c);
    }
}

// ─── HUD lateral ───
void App::drawHUD() {
    float px = worldW() + 12.f;
    float py = 12.f;

    auto line = [&](const std::string& s, sf::Color c = Theme::TEXT, unsigned sz = 13) {
        auto t = makeText(s, sz, c, px, py);
        drawTextShadow(window_, t);
        py += sz + 5.f;
    };
    auto spacer = [&]() { py += 8.f; };
    auto divider = [&]() {
        sf::RectangleShape d({UI_W - 20.f, 1.f});
        d.setPosition(px, py); d.setFillColor(Theme::GRID);
        window_.draw(d); py += 10.f;
    };

    line("QUADTREE  SIMULADOR 2D", Theme::ACCENT, 16);
    divider();

    auto& s = sim_.lastStats();
    line("SIMULACION", Theme::ACCENT2, 12);
    line("Particulas : " + std::to_string(s.numParticles));
    line("Colisiones : " + std::to_string(s.collisions));

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << s.frameTimeMs;
    line("Frame QT   : " + oss.str() + " ms");

    int n = s.numParticles;
    int bfEst = n*(n-1)/2;
    line("Comp. QT   : " + std::to_string(s.qtComparisons));
    line("Comp. BF   : " + std::to_string(bfEst));

    double ratio = (s.qtComparisons > 0)
        ? (double)bfEst / s.qtComparisons : 0;
    std::ostringstream r;
    r << std::fixed << std::setprecision(1) << ratio << "x mas rapido";
    line("QT/BF ratio: " + r.str(), Theme::ACCENT);

    spacer(); divider();

    // Modo actual
    line("MODO", Theme::ACCENT2, 12);
    std::string modeStr;
    switch (mode_) {
        case AppMode::SIMULATE:     modeStr = "Simulacion (SPACE=pausa)"; break;
        case AppMode::PAUSED:       modeStr = "PAUSADO"; break;
        case AppMode::QUERY_RECT:   modeStr = "Consulta Rect (drag)"; break;
        case AppMode::QUERY_CIRCLE: modeStr = "Consulta Circulo (drag)"; break;
        case AppMode::QUERY_KNN:    modeStr = "Consulta KNN (drag/click)"; break;
        case AppMode::BENCHMARK:    modeStr = "Benchmark..."; break;
    }
    line(modeStr, Theme::QUERIED);

    spacer(); divider();

    // Distribución
    line("DISTRIBUCION", Theme::ACCENT2, 12);
    std::string distNames[] = {"[1] Uniforme", "[2] Clusters", "[3] Zona Densa"};
    for (int i = 0; i < 3; i++)
        line(distNames[i], i == cfg_.distribution ? Theme::ACCENT : Theme::DIM);

    spacer();
    line("Capacidad QT: " + std::to_string(cfg_.qtCapacity) + "  (<-/->)", Theme::DIM);

    spacer(); divider();

    // Controles
    line("CONTROLES", Theme::ACCENT2, 12);
    line("[S] Simular  [SPACE] Pausa");
    line("[Q] Consulta rect");
    line("[C] Consulta circulo");
    line("[K] Consulta KNN (rueda=k)");
    if (mode_ == AppMode::QUERY_KNN) line("    -> K actual: " + std::to_string(queryK_), Theme::QUERIED);
    line("[Right Click] Insertar particula");
    line("[B] Benchmark completo");
    line("[R] Reiniciar");
    line("[UP/DOWN] +/- particulas");
    spacer();

    if (hasQuery_) {
        divider();
        line("ULTIMA CONSULTA", Theme::ACCENT2, 12);
        line("QT encontro : " + std::to_string(lastQuery_.qtFound.size()));
        line("BF encontro : " + std::to_string(lastQuery_.bfFound.size()));

        std::ostringstream qt, bf;
        qt << std::fixed << std::setprecision(1) << lastQuery_.qtTimeUs;
        bf << std::fixed << std::setprecision(1) << lastQuery_.bfTimeUs;
        line("Tiempo QT   : " + qt.str() + " us");
        line("Tiempo BF   : " + bf.str() + " us");
        line("Nodos visit.: " + std::to_string(lastQuery_.qtNodesVisited));
        line("Comp. BF    : " + std::to_string(lastQuery_.bfComparisons));
    }
}

// ─── Panel benchmark ───
void App::drawBenchmarkPanel() {
    if (benchResults_.empty()) return;

    // Ventana flotante encima
    float bx = 20.f, by = 30.f, bw = worldW() - 40.f, bh = 300.f;
    sf::RectangleShape bg({bw, bh});
    bg.setPosition(bx, by);
    bg.setFillColor({10,12,20,230});
    bg.setOutlineColor(Theme::ACCENT);
    bg.setOutlineThickness(2.f);
    window_.draw(bg);

    float px = bx + 15.f, py = by + 12.f;
    auto line = [&](const std::string& s, sf::Color c = Theme::TEXT, unsigned sz = 12) {
        auto t = makeText(s, sz, c, px, py);
        drawTextShadow(window_, t);
        py += sz + 4.f;
    };

    line("RESULTADOS BENCHMARK – QuadTree vs Fuerza Bruta", Theme::ACCENT, 14);
    py += 4;

    std::ostringstream header;
    header << std::left
           << std::setw(8)  << "N"
           << std::setw(14) << "Dist"
           << std::setw(14) << "QT(ms)"
           << std::setw(14) << "BF(ms)"
           << std::setw(14) << "Comp.QT"
           << std::setw(14) << "Comp.BF"
           << "Ratio";
    line(header.str(), Theme::ACCENT2, 11);

    for (auto& e : benchResults_) {
        std::ostringstream row;
        double ratio = (e.qtAvgComparisons > 0) ? e.bfAvgComparisons / e.qtAvgComparisons : 0;
        row << std::left
            << std::setw(8)  << e.n
            << std::setw(14) << e.distName
            << std::setw(14) << (std::to_string((int)e.qtAvgFrameMs) + "ms")
            << std::setw(14) << (std::to_string((int)(e.bfAvgFrameMs*1000)/1000.0) + "ms")
            << std::setw(14) << (int)e.qtAvgComparisons
            << std::setw(14) << (int)e.bfAvgComparisons
            << std::fixed << std::setprecision(1) << ratio << "x";
        line(row.str(), Theme::TEXT, 11);
    }

    line("[Presiona B de nuevo para cerrar / ESC para salir]", Theme::DIM, 11);
}

// ─── Mini gráfico de rendimiento ───
void App::drawGraph() {
    if (qtFrameTimes_.empty()) return;

    float gx = worldW() + 10.f, gy = WIN_H - 90.f;
    float gw = UI_W - 20.f, gh = 75.f;

    sf::RectangleShape bg({gw, gh});
    bg.setPosition(gx, gy);
    bg.setFillColor({15,18,30,200});
    bg.setOutlineColor(Theme::GRID);
    bg.setOutlineThickness(1.f);
    window_.draw(bg);

    auto lbl = makeText("Frame time (ms)", 10, Theme::DIM, gx+2, gy+2);
    window_.draw(lbl);

    float maxVal = 0;
    for (auto v : qtFrameTimes_) maxVal = std::max(maxVal, v);
    for (auto v : bfFrameTimes_) maxVal = std::max(maxVal, v);
    if (maxVal < 0.001f) maxVal = 1.f;

    auto drawCurve = [&](const std::deque<float>& data, sf::Color col) {
        if (data.size() < 2) return;
        sf::VertexArray va(sf::LineStrip, data.size());
        for (size_t i = 0; i < data.size(); i++) {
            float xp = gx + (float)i / (GRAPH_HISTORY-1) * gw;
            float yp = gy + gh - (data[i] / maxVal) * (gh - 10.f);
            va[i].position = {xp, yp};
            va[i].color    = col;
        }
        window_.draw(va);
    };

    drawCurve(qtFrameTimes_, Theme::QT_BAR);
    drawCurve(bfFrameTimes_, Theme::BF_BAR);

    auto leg1 = makeText("QT", 10, Theme::QT_BAR, gx+4, gy+gh-16.f);
    auto leg2 = makeText("BF(est)", 10, Theme::BF_BAR, gx+30, gy+gh-16.f);
    window_.draw(leg1); window_.draw(leg2);
}

// ══════════════════════════════════════════════
//  Benchmark automático
// ══════════════════════════════════════════════

void App::runBenchmarkAll() {
    if (benchDone_) { benchDone_ = false; benchResults_.clear(); return; }

    mode_ = AppMode::BENCHMARK;
    benchResults_.clear();

    std::vector<int> sizes = {1000, 5000, 10000};
    std::vector<int> dists = {0, 1, 2};

    for (int n : sizes) {
        for (int d : dists) {
            auto entry = sim_.runBenchmark(n, d, 30);
            benchResults_.push_back(entry);
            render(); // mostrar progreso
        }
    }

    // Guardar CSV
    std::ofstream csv("benchmark_results.csv");
    csv << "N,Distribucion,QT_AvgFrameMs,BF_FrameMs,QT_AvgComparisons,BF_Comparisons,Ratio\n";
    for (auto& e : benchResults_) {
        double ratio = (e.qtAvgComparisons > 0) ? e.bfAvgComparisons / e.qtAvgComparisons : 0;
        csv << e.n << "," << e.distName << ","
            << e.qtAvgFrameMs << "," << e.bfAvgFrameMs << ","
            << (int)e.qtAvgComparisons << "," << (int)e.bfAvgComparisons << ","
            << ratio << "\n";
    }
    csv.close();
    std::cout << "Benchmark guardado en benchmark_results.csv\n";

    benchDone_ = true;
    mode_ = AppMode::SIMULATE;
    sim_.reset(cfg_);
}

// ══════════════════════════════════════════════
//  Loop principal
// ══════════════════════════════════════════════

void App::run() {
    clock_.restart();
    while (window_.isOpen()) {
        processEvents();
        update();
        render();
    }
}
