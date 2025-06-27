# Escape The Grid: Templo al Fondo del Mar 

## Descripción General del Juego

**Escape The Grid: Templo al Fondo del Mar** es un puzzle game basado en cuadrículas hexagonales donde el jugador debe escapar de un templo que se está inundando gradualmente. El agua sube nivel por nivel, bloqueando primero las zonas más bajas (las más oscuras) y progresivamente las más altas, creando una experiencia de juego tensa y estratégica.

### Características Principales:
- **Grilla Hexagonal**: El mapa utiliza celdas hexagonales en lugar de cuadrados tradicionales
- **Sistema de Inundación**: El agua sube gradualmente basándose en la altura de las celdas
- **Sistema de Puntuación**: Penalización por backtracking y bonificaciones por items
- **Sistema de Energía**: Permite romper muros cuando la barra está llena
- **Pathfinding A***: Algoritmo inteligente para encontrar rutas óptimas
- **Modo Automático**: El juego puede resolverse automáticamente mostrando la solución

### Mecánicas del Juego:
1. **Inundación por Altura**: Las celdas se inundan según su altura, empezando por las más bajas
2. **Sistema de Puntuación**: 
   - Puntuación inicial: 100 puntos
   - Penalización por backtracking: -5 puntos
   - Bonificación por items: +100 puntos
3. **Energía**: Se gana energía al explorar nuevas celdas y permite romper un muro

## Instrucciones de Compilación

### Requisitos Previos:
- **Compilador**: tbd
- **SFML**: Biblioteca gráfica SFML 
- **Fuente**: Archivo `ARCADECLASSIC.TTF` en carpeta `fonts/`

### Compilación:

#### Opción 1: Compilación Manual
```bash
# tbd
```

#### Opción 2: CMake 
```cmake
# tbd
```

## Cómo Jugar

### Objetivo:
Escapar del templo llegando a la celda meta (roja) antes de que el agua te alcance.

### Mecánicas:
1. **Movimiento**: Navega por el mapa hexagonal usando las teclas de movimiento
2. **Agua Ascendente**: El agua sube cada 4 segundos, inundando celdas por altura
3. **Energía**: Ganas energía explorando nuevas celdas (máximo 5)
4. **Romper Muros**: Con energía completa, puedes romper un muro
5. **Items**: Recoge items amarillos para ganar puntos extra
6. **Puntuación**: Evita el backtracking para mantener tu puntuación alta

### Estrategias:
- **Planifica tu ruta**: Usa la función A* (tecla F) para ver el camino óptimo
- **Gestiona la energía**: Úsala sabiamente para romper muros estratégicos
- **Evita el backtracking**: Cada celda revisitada reduce tu puntuación
- **Considera la altura**: Las celdas más altas tardan más en inundarse

## Controles

### Movimiento Hexagonal:
```
    W     E
     \ . /
A  ----*---- D                              * = Posición actual
     / . \
    Z     X
```

- **W**: Mover arriba-izquierda  
- **E**: Mover arriba-derecha  
- **A**: Mover izquierda  
- **D**: Mover derecha  
- **Z**: Mover abajo-izquierda  
- **X**: Mover abajo-derecha  

### Funciones Especiales:
- **F**: Mostrar/calcular ruta A* hacia la meta  
- **R + (tecla de movimiento hexagonal)**: Romper muro (requiere energía completa)  
- **M**: Activar modo automático (resuelve el puzzle automáticamente)  
- **ESC**: Salir del juego  

### Leyenda del Mapa:
- 🟢 **Verde**: Punto de inicio. Las celdas tienen un color dependiendo de su altura, entre más baja la altura de la celda, más oscura es la tonalidad del verde.  
- 🔴 **Rojo**: Meta/Objetivo  
- 🟡 **Amarillo**: Item coleccionable  
- ⬛ **Gris**: Muro/Pared  
- ⬜ **Blanco**: Celda libre  
- 🔵 **Azul**: Agua/Celda inundada  

### Interfaz de Usuario:
- **Barra Superior Izquierda**: Puntuación actual  
- **Barra de Energía**: Progreso hacia poder romper muro  
- **Información de Estado**: Posición, celdas visitadas, tiempo transcurrido  
- **Instrucciones**: Controles básicos en la parte inferior  

### Condiciones de Victoria/Derrota:
- **Victoria**: Llegar a la celda meta (roja)  
- **Derrota**: Ser alcanzado por el agua (celda se inunda)  

---

## Cómo Crear un Mapa Personalizado en JSON

Puedes crear tus propios mapas y jugarlos importando un archivo `.json` con la estructura correcta. Asegúrate de que esté ubicado en la misma carpeta que el ejecutable o donde el juego pueda accederlo.

### Estructura del Archivo JSON

```json
{
  "name": "Nombre del Mapa",
  "cols": 6,
  "rows": 5,
  "grid": [
    ["S", ".", ".", ".", ".", "G"],
    [".", "#", ".", "K", ".", "."],
    [".", ".", ".", ".", "#", "."],
    [".", ".", ".", ".", ".", "."],
    ["#", ".", ".", ".", ".", "."]
  ],
  "heightMap": [
    [0, 0, 1, 2, 2, 1],
    [1, 9, 1, 1, 1, 1],
    [2, 2, 2, 2, 9, 2],
    [3, 3, 3, 3, 3, 3],
    [4, 4, 4, 4, 4, 4]
  ]
}
```

### Campos Explicados

- `"name"`: Nombre que se mostrará en el juego.  
- `"cols"` y `"rows"`: Número de columnas y filas del mapa.  
- `"grid"`: Mapa base que define el tipo de cada celda:  
  - `"S"`: Inicio del jugador  
  - `"G"`: Meta  
  - `"."`: Celda libre  
  - `"#"`: Muro  
  - `"K"`: Item coleccionable  

- `"heightMap"`: Mapa de alturas (mismos índices que `grid`).  
  - Números bajos (0-2): Zonas bajas, se inundan primero  
  - Números altos (8-10): Zonas elevadas, se inundan al final  

### Reglas Importantes

- Debe haber **una sola celda de inicio** (`"S"`) y **una sola celda meta** (`"G"`).  
- `"grid"` y `"heightMap"` deben tener la misma forma (filas y columnas).  
- Usa solo números positivos en `heightMap`.  
- Puedes agregar varios items (`"K"`), pero al menos uno es recomendable.  

### Ejemplo de Mapa Más Grande

```json
{
  "name": "Ruinas Profundas",
  "cols": 8,
  "rows": 6,
  "grid": [
    ["S", ".", ".", ".", ".", ".", ".", "G"],
    [".", "#", ".", "K", ".", "#", ".", "."],
    [".", ".", ".", ".", ".", ".", "#", "."],
    ["#", ".", "#", ".", ".", ".", ".", "."],
    [".", ".", ".", "K", "#", ".", ".", "."],
    ["#", ".", ".", ".", ".", ".", ".", "."]
  ],
  "heightMap": [
    [0, 0, 1, 2, 2, 1, 1, 1],
    [1, 9, 2, 2, 2, 9, 2, 1],
    [2, 2, 3, 3, 4, 4, 9, 2],
    [3, 4, 9, 5, 5, 5, 5, 3],
    [4, 4, 4, 4, 9, 6, 6, 4],
    [5, 5, 5, 6, 6, 7, 7, 5]
  ]
}
```

Una vez creado el archivo `.json`, puedes cargarlo desde el juego al iniciarlo o especificarlo por línea de comandos si tu juego lo permite (ej. `./EscapeTheGrid mapa.json`).
