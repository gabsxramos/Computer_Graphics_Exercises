# 2D Half-Edge Topological Visualizer & Querier

This project is a C++ application designed to load, visualize, and perform complex topological queries on 2D graphic objects using the **Half-Edge** data structure. Built with OpenGL and GLUT, it focuses on demonstrating the relationships between vertices, edges, and faces in a polygonal mesh.

---

## 🚀 Key Features

### 1. Advanced Data Structure
* **Half-Edge Implementation:** Represents the object topology by mapping the connectivity between components. Each edge is split into two "half-edges" with opposite directions, allowing for efficient mesh traversal.
* **OBJ Parser:** Reads vertex coordinates and face definitions from standard Wavefront `.obj` files.

### 2. Custom Rendering Engine
* **Point-Based Line Rendering:** To satisfy specific geometric requirements, edges are rendered using the **line equation** and `GL_POINTS` rather than standard OpenGL line primitives.
* **ID Visualization:** Overlay display of unique IDs for **Vertices (V)**, **Edges (E)**, and **Faces (F)** directly on the graphical window.

### 3. Topological Query System
The program allows real-time interaction through the console to explore the object's mesh:
* **Face-to-Face:** List all faces adjacent to a specific face.
* **Edge-to-Face:** List faces sharing a specific edge.
* **Vertex-to-Face:** List all faces that share a specific vertex.
* **Vertex-to-Edge:** List all edges originating from a specific vertex.

---

## 🛠️ Installation & Compilation

### Prerequisites
* **Linux:** `freeglut3-dev`, `mesa-common-dev`
* **Windows:** `freeglut` (MinGW or MSYS2 recommended)

### Build Commands

**Linux:**
```bash
g++ main.cpp -o visualizer -lglut -lGL -lGLU -lpthread
```

**Windows (MinGW)**
```bash
g++ main.cpp -o visualizer -lfreeglut -lopengl32 -lglu32 -lpthread
```

---

## 💻 Usage

Run the program by passing the path to a .obj file as an argument:
```bash
./visualizer path/to/your_object.obj
```

---

## 🎮 Controls

The program uses a hybrid interaction system: the graphical window displays the mesh, while the terminal handles data input and results for topological queries.

### Display Toggles (Press in the Graphics Window)
* **`v`**: Toggle **Vertex IDs** (Displayed in Orange).
* **`e`**: Toggle **Edge IDs** (Displayed in Yellow).
* **`f`**: Toggle **Face IDs** (Displayed in Green).
* **`ESC`**: Close the application.

### Topological Queries (Terminal Interaction)
Pressing these keys in the graphics window will trigger a prompt in your terminal:
* **`1`**: **Adjacent Faces of a Face** — List all faces that share an edge with the selected Face ID.
* **`2`**: **Adjacent Faces of an Edge** — List the faces on both sides of the selected Edge ID.
* **`3`**: **Faces sharing a Vertex** — List all faces that meet at the selected Vertex ID.
* **`4`**: **Edges sharing a Vertex** — List all half-edges that have the selected Vertex ID as their origin.

---

## 📂 Data Structure Detail

This visualizer is built upon the **Half-Edge** data structure (also known as DCEL - Doubly Connected Edge List). This structure is ideal for manifold meshes as it explicitly defines the connectivity between vertices, edges, and faces.



### Core Components
The implementation uses a set of pointers to allow $O(1)$ or $O(n)$ navigation through the mesh:

* **Vertex**: Stores the $(x, y)$ coordinates and a pointer to one **leaving** half-edge. This is the starting point for finding all edges connected to a vertex.
* **Face**: Stores a pointer to a single **half-edge** that forms its boundary. By following the `next` pointers, you can traverse the entire perimeter of the face.
* **Half-Edge**: The fundamental building block. Each physical edge is represented by two directed half-edges:
    * `origin`: The vertex where the half-edge begins.
    * `twin`: The half-edge running in the opposite direction.
    * `next`: The next half-edge in the counter-clockwise sequence around the face.
    * `face`: The specific face that this half-edge bounds.



### Query Logic
To perform queries like "Faces sharing a vertex," the algorithm starts at the `vertex->leaving` pointer and rotates around the vertex by repeatedly jumping to `current_edge->twin->next`. This traversal demonstrates the power of the Half-Edge structure for spatial analysis without needing to search through the entire dataset.

---