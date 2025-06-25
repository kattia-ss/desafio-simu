import json
import random

ROWS, COLS = 100, 100
grid = [["#" for _ in range(COLS)] for _ in range(ROWS)]
heightMap = [[random.randint(10, 20) for _ in range(COLS)] for _ in range(ROWS)]  # muros relleno

# Crear camino S → G con escalones
row, col = 0, 0
grid[row][col] = "S"
heightMap[row][col] = 0

camino = [(row, col)]
altura = 0

while row < ROWS - 1:
    # Derecha
    while col < COLS - 2:
        col += 1
        altura += 1
        grid[row][col] = "."
        heightMap[row][col] = altura
        camino.append((row, col))
    row += 1
    if row >= ROWS: break
    altura += 1
    grid[row][col] = "."
    heightMap[row][col] = altura
    camino.append((row, col))

    # Izquierda
    while col > 0:
        col -= 1
        altura += 1
        grid[row][col] = "."
        heightMap[row][col] = altura
        camino.append((row, col))
    row += 1
    if row >= ROWS: break
    altura += 1
    grid[row][col] = "."
    heightMap[row][col] = altura
    camino.append((row, col))

# Meta
grid[row - 1][col] = "G"
heightMap[row - 1][col] = altura + 1

# Agregar ítems sobre el camino
num_items = 20
espaciado = len(camino) // num_items
for i in range(espaciado, len(camino), espaciado):
    r, c = camino[i]
    if grid[r][c] == ".":
        grid[r][c] = "*"

# Guardar JSON
data = {
    "name": "LaberintoConAlturaLentaYItems",
    "cols": COLS,
    "rows": ROWS,
    "grid": grid,
    "heightMap": heightMap
}

with open("mapa_items_alturas_lentas.json", "w") as f:
    json.dump(data, f, indent=2)

print("✅ Mapa generado: mapa_items_alturas_lentas.json")
