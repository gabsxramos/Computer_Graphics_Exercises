# Multi-Object Half-Edge Visualizer: Cohen-Sutherland & Rasterization

This project is an advanced C++ 2D graphical visualizer that implements the **Half-Edge** data structure to manage multiple geometric objects simultaneously. It features active **Cohen-Sutherland Line Clipping**, custom rasterization algorithms, and real-time geometric transformations.

---

## 🚀 Key Features

### 1. Line Clipping (Cohen-Sutherland)
The program actively clips lines in the **World Coordinate Space** ([-1, 1]) before they reach the rasterization stage. 
* Uses **Outcodes** (Inside, Left, Right, Bottom, Top) to identify line segments.
* Performs trivial acceptance, trivial rejection, and iterative intersection calculation.


### 2. Multi-Object Management
* Load multiple `.obj` files in a single execution.
* Toggle between objects using a dedicated "Active Object" system.
* Independent transformations: Apply movements only to the currently selected object.

### 3. Advanced Rasterization
Choose between two rendering techniques via command-line parameters:
* **Bresenham:** High-performance, integer-based line drawing.
* **Xiaolin Wu:** High-quality anti-aliasing for smooth, aesthetic edges.

### 4. Refined Viewport Transformation
Converts coordinates from the **World Space** ($-1.0$ to $1.0$) to the **Pixel Space** ($0$ to Window Width/Height), ensuring the clipping window matches the visible screen area perfectly.

---

## 🛠️ Compilation

### Linux
```bash
g++ main.cpp -o visualizer -lglut -lGL -lGLU -lpthread
```
### Windows (MinGW/MSYS2)
```bash
g++ main.cpp -o visualizer -lfreeglut -lopengl32 -lglu32 -lpthread
```
---

## 🚀 Usage

Run the program by specifying the algorithm, the input OBJ files, and an optional output file:

```bash
./visualizer <bresenham|xiaolin_wu> <object1.obj> [object2.obj ...] [output.obj]
```
### Example
```bash
./visualizer xiaolin_wu house.obj tree.obj result.obj
```
---

## 🎮 Controls (Graphics Window)

The application uses a hybrid interaction system: the graphical window handles visualization and object selection, while the terminal is used for entering precise transformation parameters.

### Object Management
* **`n`**: **Next Object** — Cycles through all loaded `.obj` files. The current "Active Object" is highlighted and will receive all transformations.
* **`v`**: Toggle **Vertex IDs** (Orange labels).
* **`e`**: Toggle **Edge IDs** (Yellow labels).
* **`f`**: Toggle **Face IDs** (Green labels).

### Geometric Transformations (Terminal Input)
When these keys are pressed in the window, the focus shifts to the terminal to input values:
* **`t`**: **Translation** — Prompts for `dx` and `dy`.
* **`s`**: **Scaling** — Prompts for `sx` and `sy` (relative to the object's centroid).
* **`r`**: **Rotation** — Prompts for the `angle` in degrees.
* **`h`**: **Shearing** — Prompts for `shx` and `shy`.
* **`m`**: **Reflection** — Prompts for the axis (`x` or `y`).

### System Commands
* **`w`**: **Save** — Exports the current state of the **active object** to the output file.
* **`ESC`**: **Exit** — Closes the program.

---

## 📂 Data Structure: Half-Edge & GraphicalObject

The project is structured to handle multiple distinct models within the same workspace by encapsulating the Half-Edge logic inside a higher-level object container.

### 1. The GraphicalObject Container
To manage multiple `.obj` files simultaneously, the `GraphicalObject` structure acts as a workspace for each model. 
* **Model Isolation**: Each object maintains its own unique lists of vertices, faces, and half-edges.
* **Centroid Calculation**: It includes a specialized method to calculate the object's center of mass (centroid), ensuring that transformations like **Rotation** and **Scaling** occur around the object's local origin rather than the world origin $(0,0)$.

### 2. The Half-Edge Logic
The core topology is managed using the **Half-Edge** (or DCEL) data structure. This is essential for maintaining mesh consistency after geometric transformations.



* **Vertex**: Stores the 2D world coordinates $(x, y)$ and a pointer to a "leaving" half-edge.
* **Face**: Stores a pointer to one of the half-edges that define its boundary.
* **Half-Edge**: The fundamental link that connects the mesh components:
    * `origin`: The vertex where the half-edge starts.
    * `twin`: The corresponding half-edge in the opposite direction.
    * `next`: The next half-edge in the face loop (counter-clockwise).
    * `face`: The face associated with this specific half-edge.



### 3. Transformation Persistence
Since the transformations are applied directly to the `Vertex` structures within the `GraphicalObject`, the topological relationships (the pointers between half-edges) remain intact. This allows the user to save the modified mesh to a new `.obj` file while preserving the manifold properties of the original geometry.

---

## 📐 Geometric Logic: World to Screen

The rendering pipeline follows a strict mathematical sequence to ensure that the 2D objects are correctly processed, clipped, and mapped before they appear as pixels on your monitor.

### 1. Line Clipping (Cohen-Sutherland)
Before any drawing occurs, the edges of the object are tested against the **World Space** boundaries ($X: [-1, 1], Y: [-1, 1]$).
* **Outcoding**: Each vertex is assigned a 4-bit code based on its position relative to the viewing window.
* **Intersection**: If a line is partially outside, the algorithm calculates the exact intersection point with the boundary and "clips" the line, preventing the rasterizer from processing coordinates outside the visible area.



### 2. Viewport Transformation
Once clipped, the coordinates are still in **World Units** (floating point). They must be converted into **Screen Units** (integers/pixels) using the following mapping:

$$px = \frac{x_{world} - X_{min}}{X_{max} - X_{min}} \cdot \text{WindowWidth}$$
$$py = \frac{y_{world} - Y_{min}}{Y_{max} - Y_{min}} \cdot \text{WindowHeight}$$

This transformation ensures that the coordinate $(-1, -1)$ maps to the bottom-left pixel $(0, 0)$ and $(1, 1)$ maps to the top-right pixel $(600, 600)$.

### 3. Rasterization (Bresenham vs. Xiaolin Wu)
The final step is converting the mathematical line into a set of discrete pixels:
* **Bresenham**: Uses integer arithmetic to decide which pixel is closest to the actual line path. It is fast and produces "jagged" (aliased) lines.
* **Xiaolin Wu**: Calculates the distance from the line to the two closest pixels and assigns a transparency (Alpha) value to each. This produces a smooth, **anti-aliased** look by simulating sub-pixel accuracy.

---