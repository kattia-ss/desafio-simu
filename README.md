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



---

**¡Disfruta escapando del Templo al Fondo del Mar!**
