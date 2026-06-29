# Reporte Técnico – Opción A: QuadTree  
## Simulador 2D de Partículas  
### CS2023 – Algoritmos y Estructuras de Datos

---

## 1. Introducción

El objetivo de este proyecto es implementar un **QuadTree** desde cero y demostrar que permite resolver el problema de detección de colisiones y consultas espaciales de manera más eficiente que una solución ingenua de fuerza bruta.

Un QuadTree es una estructura de datos en árbol en la que cada nodo interno tiene exactamente cuatro hijos, particionando el espacio 2D en cuadrantes de manera recursiva. Fue introducida por Finkel y Bentley en 1974 y es ampliamente utilizada en motores de videojuegos, sistemas de información geográfica (GIS), renderización y simulaciones físicas.


---

## 2. Descripción de la estructura de datos

### 2.1 Definición

Un **QuadTree de región (PR-QuadTree)** divide el espacio 2D recursivamente en cuatro cuadrantes (NW, NE, SW, SE) cuando la cantidad de objetos en una región supera la capacidad máxima del nodo. Cada nodo almacena:

```
Node {
    AABB    boundary      // región que representa este nodo
    int     capacity      // máximo de objetos antes de subdividir
    int     depth         // profundidad en el árbol
    bool    divided       // si ya fue subdividido
    list    particles     // objetos en este nodo (si es hoja o desbordamiento)
    Node*   children[4]  // hijos SW, SE, NW, NE
}
```

### 2.2 Invariantes

1. Cada nodo representa un rectángulo axialmente alineado (AABB).
2. Un nodo subdivide cuando `|particles| >= capacity` y `depth < MAX_DEPTH`.
3. Un objeto se inserta en el primer hijo cuya región lo contenga.
4. Las regiones de los hijos cubren exactamente la región del padre sin solapamiento.
5. La profundidad máxima garantiza terminación (máx. 8 niveles en esta implementación).

### 2.3 Operaciones y complejidad

| Operación | Descripción | Complejidad |
|-----------|-------------|-------------|
| `insert(p)` | Insertar objeto en el árbol | O(log n) promedio |
| `query(AABB)` | Buscar objetos en región rectangular | O(log n + k) |
| `queryCircle(c, r)` | Buscar objetos en radio r | O(log n + k) |
| `queryKNN(p, k)` | Buscar los k vecinos más cercanos a un punto | O(log n + k) |
| `clear()` | Vaciar el árbol | O(n) |
| `rebuild(particles)` | Reconstruir el árbol completo | O(n log n) |

Donde `n` es el número de objetos y `k` el número de resultados encontrados.

### 2.4 Inserción

```
insert(particle p):
  si boundary no contiene p → return false
  si |particles| < capacity o depth >= MAX_DEPTH:
      particles.add(p)
      return true
  si no subdividido:
      subdivide()
  para cada hijo:
      si hijo.insert(p) → return true
  particles.add(p)   // fallback: insertar aquí
  return true
```

### 2.5 Subdivisión

Al subdividir, el nodo crea 4 hijos con la mitad del tamaño en cada dimensión y redistribuye las partículas actuales entre ellos:

```
subdivide():
  hw = boundary.w / 2
  hh = boundary.h / 2
  children[SW] = Node(x-hw, y-hh, hw, hh)
  children[SE] = Node(x+hw, y-hh, hw, hh)
  children[NW] = Node(x-hw, y+hh, hw, hh)
  children[NE] = Node(x+hw, y+hh, hw, hh)
  redistribuir partículas actuales a hijos
```

### 2.6 Consulta rectangular

```
query(range, found, nodesVisited):
  nodesVisited++
  si boundary no intersecta range → return
  para cada partícula p en particles:
      si range contiene p → found.add(p)
  si divided:
      para cada hijo:
          hijo.query(range, found, nodesVisited)
```

La clave es la **poda espacial**: si el bounding box de un nodo no intersecta la región buscada, se descarta todo ese subárbol sin revisarlo.

### 2.7 Consulta circular

Similar a la rectangular pero usando el radio como bounding box para el filtro inicial:

```
queryCircle(center, radius, found, nodesVisited):
  circleBB = AABB(center.x, center.y, radius, radius)
  si boundary no intersecta circleBB → return
  para cada p en particles:
      si dist(p, center) <= radius → found.add(p)
  si divided → propagar a hijos
```

### 2.8 Consulta K-Nearest Neighbors (KNN)

Permite buscar los $k$ elementos más cercanos a un punto objetivo:

```
queryKNN(target, k, best, nodesVisited):
  si |best| == k y dist(target, boundary) > peor_dist(best):
      return // poda
  insertar ordenadamente en best cada p en particles (manteniendo máx k elementos)
  si divided:
      ordenar hijos por distancia a target (más cercano primero)
      para cada hijo:
          hijo.queryKNN(target, k, best, nodesVisited)
```

---

## 3. Aplicación: Simulador de Partículas 2D

### 3.1 Descripción

La aplicación simula `n` partículas circulares moviéndose en un espacio 2D acotado. En cada frame:

1. Se actualiza la posición de cada partícula (`x += vx * dt`, `y += vy * dt`).
2. Las partículas que salen del espacio se "envuelven" al lado opuesto (toroidal).
3. Se reconstruye el QuadTree con las nuevas posiciones.
4. Se detectan colisiones entre partículas usando el QuadTree.

### 3.2 Modelo de partícula

```cpp
struct Particle {
    int    id;
    double x, y;      // posición
    double vx, vy;    // velocidad
    double radius;    // radio
    bool   colliding; // flag para visualización
};
```

### 3.3 Detección de colisiones con QuadTree

Para cada partícula `p`, en lugar de comparar con todas las demás (O(n²)), se consulta el QuadTree en la región `[p.x ± 2r, p.y ± 2r]`. Solo se comparan las partículas candidatas devueltas por la consulta:

```
para cada partícula p:
    area = AABB(p.x, p.y, p.radius*2 + margen, p.radius*2 + margen)
    candidatos = quadTree.query(area)
    para cada q en candidatos (q.id > p.id):
        dist = dist(p, q)
        si dist < p.radius + q.radius:
            // 1. Separar para evitar solapamiento
            // 2. Modificar velocidades (choque elástico)
            resolverColisionFisica(p, q, dist)
```

Esto reduce el número de comparaciones de `O(n²)` a `O(n log n)` en media, y la física de choques elásticos previene que las partículas se superpongan continuamente, permitiendo conteos de colisiones realistas.

### 3.4 Distribuciones de partículas

Se implementaron tres distribuciones configurables:

| Distribución | Descripción |
|-------------|-------------|
| **Uniforme** | Posiciones aleatorias con distribución uniforme en todo el espacio |
| **Clusters** | 5 centros aleatorios; partículas distribuidas con normal σ=6% del ancho |
| **Zona densa** | 70% en región central (σ=8%), 30% uniforme en todo el espacio |

*Nota: La aleatoriedad (posiciones, velocidades y tamaños) utiliza un motor `std::mt19937` con una semilla determinista configurable (`seed`), garantizando que la simulación sea reproducible en cada reinicio.*

---

## 4. Comparación experimental

### 4.1 Metodología

Se comparó el QuadTree contra fuerza bruta midiendo:
- Tiempo promedio por frame (ms)
- Número de comparaciones realizadas
- Nodos visitados por el QuadTree

Cada experimento corre 60 frames a `dt = 1/60 s`.

### 4.2 Tamaños de entrada

| Tamaño | n (partículas) | Justificación |
|--------|---------------|---------------|
| Pequeño | 1,000 | Escenario de baja densidad, diferencias iniciales visibles |
| Mediano | 5,000 | Escenario típico de juego 2D, diferencia notable |
| Grande | 10,000 | Límite práctico; evidencia diferencia asintótica clara |

### 4.3 Resultados experimentales reales

Benchmark ejecutado con capacidad=4, profundidad máxima=8, 60 frames por experimento:

| n | Dist. | QT frame (ms) | BF frame (ms) | Comp. QT | Comp. BF | Ratio |
|---|-------|--------------|--------------|---------|---------|-------|
| 1,000 | Uniforme | 0.87 | 0.95 | 20,998 | 499,500 | **23.8x** |
| 1,000 | Clusters | 0.78 | 0.99 | 27,447 | 499,500 | 18.2x |
| 1,000 | Zona densa | 0.86 | 0.99 | 31,120 | 499,500 | 16.1x |
| 5,000 | Uniforme | 5.12 | 23.65 | 158,884 | 12,497,500 | **78.7x** |
| 5,000 | Clusters | 7.96 | 23.84 | 233,061 | 12,497,500 | 53.6x |
| 5,000 | Zona densa | 9.29 | 24.03 | 275,875 | 12,497,500 | 45.3x |
| 10,000 | Uniforme | 14.70 | 95.11 | 402,660 | 49,995,000 | **124.2x** |
| 10,000 | Clusters | 23.43 | 95.88 | 608,953 | 49,995,000 | 82.1x |
| 10,000 | Zona densa | 28.48 | 96.74 | 773,289 | 49,995,000 | 64.7x |

### 4.4 Efecto de la distribución espacial

- **Uniforme**: QuadTree bien equilibrado, mejor caso típico.
- **Clusters**: Alta densidad local puede profundizar el árbol; más comparaciones por cluster.
- **Zona densa**: Peor caso relativo para el QuadTree en la zona central (mayor profundidad).

### 4.5 Complejidad teórica

| Estructura | Detección de colisiones | Consulta rect/círculo |
|-----------|------------------------|----------------------|
| Fuerza bruta | O(n²) | O(n) |
| QuadTree | O(n log n) promedio | O(log n + k) |

---

## 5. Visualización

La aplicación muestra en tiempo real:

| Elemento | Descripción |
|----------|-------------|
| Partículas | Círculos coloreados por velocidad; rojo si hay colisión |
| Subdivisiones QT | Cuadrículas azules que muestran la partición actual |
| Región consultada | Rectángulo/círculo amarillo al realizar consultas, o punto con radio en KNN |
| Resultados | Partículas encontradas destacadas con borde amarillo |
| Panel lateral | Stats en tiempo real: comparaciones QT vs BF, colisiones, frame time |
| Insertar Partícula| Añadir un objeto nuevo en cualquier posición de la simulación mediante *Right Click* |
| Gráfico temporal | Histórico de frame time QT vs estimado BF |
| Resultados benchmark | Tabla comparativa completa al presionar `B` |

---

## 6. Conclusiones

1. El QuadTree reduce el número de comparaciones de O(n²) a O(n log n) mediante **poda espacial**: regiones que no intersectan la consulta se descartan sin revisar.
2. La ventaja aumenta con n: para n=10,000, el QuadTree realiza órdenes de magnitud menos comparaciones que fuerza bruta.
3. La distribución espacial afecta el rendimiento: distribuciones uniformes favorecen un árbol equilibrado; distribuciones con zonas densas pueden degradar localmente la eficiencia.
4. La reconstrucción del árbol por frame tiene costo O(n log n), lo que es aceptable frente al O(n²) de la detección bruta.

---

## 7. Referencias

- Finkel, R. A., & Bentley, J. L. (1974). *Quad trees: A data structure for retrieval on composite keys*. Acta informatica, 4(1), 1–9.
- de Berg, M., Cheong, O., van Kreveld, M., & Overmars, M. (2008). *Computational Geometry: Algorithms and Applications* (3rd ed.). Springer.
- Hunter, J. (2018). *Game Programming Patterns*. Cap. Spatial Partition.
- SFML Documentation. https://www.sfml-dev.org/documentation/

