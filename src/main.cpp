#include "App.h"
#include <iostream>

int main() {
    std::cout << "=== QuadTree – Simulador 2D de Particulas ===\n";
    std::cout << "CS2023 Algoritmos y Estructuras de Datos\n\n";
    std::cout << "Controles:\n";
    std::cout << "  SPACE    : Pausar/Reanudar simulacion\n";
    std::cout << "  S        : Modo simulacion\n";
    std::cout << "  Q        : Modo consulta rectangular (drag)\n";
    std::cout << "  C        : Modo consulta circular (drag)\n";
    std::cout << "  B        : Ejecutar benchmark completo\n";
    std::cout << "  1/2/3    : Cambiar distribucion (uniforme/clusters/densa)\n";
    std::cout << "  UP/DOWN  : +/- particulas\n";
    std::cout << "  LEFT/RIGHT: capacidad del QuadTree\n";
    std::cout << "  R        : Reiniciar\n";
    std::cout << "  ESC      : Salir\n\n";

    try {
        App app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
