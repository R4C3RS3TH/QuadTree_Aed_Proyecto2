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


### Windows
- Instalar MinGW (para usar `g++` y `make`)
- *Nota: Las librerías y dependencias de SFML ya vienen incluidas en la carpeta `SFML/` del repositorio, por lo que no es necesario instalarlas ni configurarlas manualmente.*

---

## Compilación y ejecución

El proyecto se puede compilar y ejecutar directamente desde la terminal, sin necesidad de usar CMake.

### Windows (usando CMD o PowerShell)
Las dependencias de SFML ya están incluidas y configuradas en los scripts. Para compilar y generar el ejecutable, simplemente ejecute el script `.bat` proporcionado:
```cmd
./build.bat
```
Alternativamente, si tiene `make` de MinGW configurado en su sistema, puede compilar usando:
```cmd
make
```
Una vez compilado, ejecute el programa con:
```cmd
./quadtree_sim.exe
```

### Linux
En entornos Unix, SFML se usa desde el sistema. Puede utilizar el archivo de configuración para Linux incluido:
```bash
# Compilar usando el Makefile específico de Linux
make -f Makefile.linux

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
| `K` | Modo consulta KNN (click/arrastrar para punto, rueda para cambiar K) |
| `Right Click`| Insertar partícula nueva en la posición del mouse |
| `Rueda`| Cambiar radio de consulta circular o cantidad de vecinos (K) en KNN |
| `B` | Ejecutar benchmark completo (3 tamaños × 3 distribuciones) |
| `1` | Distribución uniforme |
| `2` | Distribución con clusters |
| `3` | Distribución con zona de alta densidad |
| `↑ / ↓` | Aumentar / disminuir número de partículas (±100) |
| `← / →` | Cambiar capacidad máxima del QuadTree |
| `R` | Reiniciar simulación |
| `ESC` | Salir |

---

## Métricas de la Visualización

El HUD muestra métricas en tiempo real sobre el estado y rendimiento de la simulación:

- **Partículas**: Cantidad total de partículas actualmente en simulación.
- **Colisiones**: Número total de choques detectados entre partículas durante el frame actual.
- **Frame QT**: Tiempo en milisegundos que toma calcular las colisiones utilizando el QuadTree.
- **Comp. QT**: Número de comparaciones realizadas por el QuadTree para detectar colisiones en el frame actual.
- **Comp. BF**: Número de comparaciones estimadas o reales que tomaría resolver las colisiones mediante Fuerza Bruta (O(N²)).
- **QT/BF ratio**: Relación de mejora de rendimiento; indica cuántas veces menos comparaciones requiere el QuadTree frente a Fuerza Bruta.

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

Proyecto final CS2023 – Algoritmos y Estructuras de Datos  
Departamento de Computer Science  
