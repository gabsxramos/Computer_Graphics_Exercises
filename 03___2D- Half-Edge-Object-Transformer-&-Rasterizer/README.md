# 2D Half-Edge Object Transformer & Rasterizer

This project is an advanced 2D graphic tool built in C++ that utilizes the **Half-Edge** data structure to manage, transform, and render Wavefront (.obj) objects. It supports multiple rasterization algorithms and dynamic geometric transformations.

---

## 🚀 Features

### 1. Rasterization Algorithms
The program allows you to choose how edges are drawn at startup:
* **Bresenham's Algorithm:** A fast, integer-based line drawing algorithm.
* **Xiaolin Wu's Algorithm:** Provides high-quality anti-aliased lines for a smoother visual appearance.



### 2. Geometric Transformations
Transformations are applied directly to the vertices within the Half-Edge structure. The visualizer supports:
* **Translation:** Move the object along X and Y axes.
* **Scaling:** Resize the object relative to its centroid.
* **Rotation:** Rotate the object around its center.
* **Shearing:** Apply slant effects on the X or Y axes.
* **Reflection (Mirroring):** Flip the object over a specific axis.

### 3. Half-Edge Persistence
* **Load OBJ:** Reads 2D vertices and faces from Wavefront files.
* **Save OBJ:** Exports the current (transformed) state of the object back into a `.obj` file, ensuring the geometric data is preserved.

---

## 🛠️ Compilation

### Linux
Requires OpenGL and GLUT development libraries.
```bash
g++ main.cpp -o visualizer -lglut -lGL -lGLU -lpthread
```

---

## 💻 Usage

Run the executable by passing the input file, the chosen algorithm, and an optional output file path:

```bash
./visualizer <input.obj> <algorithm> [output.obj]
```

# Examples:

```bash
# Using Bresenham
./visualizer house.obj bresenham result.obj

# Using Xiaolin Wu (Anti-aliasing)
./visualizer house.obj xiaolin_wu result.obj
```

---

## 🎮 Controls

The application uses a hybrid interaction model. Visualization toggles are handled directly in the graphics window, while transformation parameters are entered via the terminal.

### Visualization Toggles (Graphics Window)
* **`v`**: Toggle Vertex IDs visibility.
* **`e`**: Toggle Edge IDs visibility.
* **`f`**: Toggle Face IDs visibility.

### Geometric Transformations (Console Input)
Pressing these keys in the window will pause to ask for parameters in your terminal:
* **`t`**: **Translation** — Prompts for `dx` and `dy`.
* **`s`**: **Scaling** — Prompts for `sx` and `sy` (relative to the object's centroid).
* **`r`**: **Rotation** — Prompts for an `angle` in degrees.
* **`h`**: **Shearing** — Prompts for `shx` and `shy`.
* **`m`**: **Reflection** — Prompts for the axis (`x` or `y`).

### System Commands
* **`w`**: **Save** — Writes the current transformed state to the output `.obj` file.
* **`ESC`**: **Exit** — Closes the application.

---

## 📂 Data Structure Detail

The project implements the **Half-Edge** (or Doubly Connected Edge List) data structure. This is crucial for maintaining topological consistency; when a vertex is transformed, all connected edges and faces remain logically linked.

### Core Components:
1.  **Vertex**: Stores the 2D coordinates ($x, y$) and a pointer to one "leaving" half-edge.
2.  **Face**: Stores a pointer to one of the half-edges that bound it.
3.  **Half-Edge**: The primary structure containing:
    * **Origin**: Pointer to the vertex it starts from.
    * **Twin**: Pointer to the opposite half-edge.
    * **Next**: Pointer to the next half-edge in the counter-clockwise loop of the face.
    * **Face**: Pointer to the face it belongs to.

After any geometric transformation, the `x` and `y` values in the **Vertex** structures are updated, and the edges are immediately re-rendered using the selected rasterization algorithm.

---