# QuadTree – Simulador 2D de Partículas
### CS2023 – Algoritmos y Estructuras de Datos  
**Opción A: QuadTree**

---

## Descripción

Aplicación interactiva que simula partículas en movimiento en un espacio 2D, usando un **QuadTree** para detectar colisiones y realizar consultas espaciales de forma eficiente, comparando contra una solución de **fuerza bruta (O(n²))**.

---

## Requisitos

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install cmake g++ libsfml-dev
```

### macOS
```bash
brew install cmake sfml
```

### Windows
- Instalar [CMake](https://cmake.org/download/)
- Instalar [SFML 2.5+](https://www.sfml-dev.org/download.php)
- Instalar MinGW o MSVC

---

## Compilación y ejecución

El proyecto se puede compilar y ejecutar directamente desde la terminal, sin necesidad de usar CMake.

### Windows (usando CMD o PowerShell)
Antes de ejecutar el script, asegúrese de editar el archivo `build.bat` (y `Makefile` si corresponde) para que las variables `SFML_INCLUDE` y `SFML_LIB` apunten a la carpeta donde tiene instalado SFML.
Luego, simplemente ejecute el script `.bat` proporcionado, el cual compilará el código y generará el ejecutable:
```cmd
build.bat
```
O compile manualmente usando `g++`:
```cmd
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude -o quadtree_sim.exe src/main.cpp src/QuadTree.cpp src/Simulation.cpp src/App.cpp -lsfml-graphics -lsfml-window -lsfml-system
quadtree_sim.exe
```

### Linux / macOS (o Windows con MinGW32-make)
Puede utilizar el `Makefile` incluido:
```bash
# Compilar
make

# Ejecutar
./quadtree_sim
```

### Benchmark sin interfaz gráfica

```bash
# Desde la raíz del proyecto
g++ -std=c++17 -O2 -Iinclude \
    src/QuadTree.cpp src/Simulation.cpp scripts/benchmark_only.cpp \
    -o benchmark_only
./benchmark_only
```

---

## Controles de la aplicación

| Tecla | Acción |
|-------|--------|
| `SPACE` | Pausar / reanudar simulación |
| `S` | Modo simulación en tiempo real |
| `Q` | Modo consulta rectangular (arrastrar para definir región) |
| `C` | Modo consulta circular (arrastrar desde centro) |
| `B` | Ejecutar benchmark completo (3 tamaños × 3 distribuciones) |
| `1` | Distribución uniforme |
| `2` | Distribución con clusters |
| `3` | Distribución con zona de alta densidad |
| `↑ / ↓` | Aumentar / disminuir número de partículas (±100) |
| `← / →` | Cambiar capacidad máxima del QuadTree |
| `R` | Reiniciar simulación |
| `ESC` | Salir |

---

## Estructura del proyecto

```
quadtree_project/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── QuadTree.h      ← Estructura de datos principal
│   ├── Simulation.h    ← Lógica de simulación y benchmark
│   └── App.h           ← Renderizador SFML
├── src/
│   ├── main.cpp        ← Punto de entrada
│   ├── QuadTree.cpp    ← Implementación del QuadTree
│   ├── Simulation.cpp  ← Simulación, distribuciones, experimentos
│   └── App.cpp         ← Interfaz gráfica completa
├── scripts/
│   └── benchmark_only.cpp  ← Benchmark sin SFML
└── docs/
    └── reporte.md      ← Reporte técnico completo
```

---

## Datasets

La aplicación genera **datos sintéticos** configurables. No se requiere dataset externo (según especificación de la Opción A). Se implementan tres distribuciones:

1. **Uniforme** – posiciones aleatorias en todo el espacio
2. **Clusters** – 5 grupos de alta densidad con distribución normal
3. **Zona densa** – 70% en región central, 30% uniforme

---

## Tecnologías

- **C++17** – implementación de la estructura
- **SFML 2.5+** – renderización 2D e interfaz gráfica
- **Make / g++** – compilación directa desde terminal mediante scripts

---

## Créditos

Proyecto final CS2023 – Algoritmos y Estructuras de Datos  
Departamento de Computer Science  
