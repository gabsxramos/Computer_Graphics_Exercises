# Computer Graphics Exercises 2025

Welcome! This repository is a collection of computational geometry and computer graphics projects developed during my **Computer Engineering** studies. It covers essential topics such as polygon clipping, mesh data structures (Half-Edge), topological queries, and real-time rasterization.

---

## 📂 Project Portfolio

### Summarizing the information about folders 01-04:

COMPUTER_GRAPHICS_EXERCISES_2025/
├── README.md                                         # General explanation of the repository
├── .gitignore                                        # Files to be ignored by Git
│
├── 01__Weiler-Atherton/                              # Concave polygon clipping algorithm
│   ├── WeilerAtherton.py                             # Python source code
│   └── README.md                                     # Documentation for this specific project
│
├── 02__2D-Half-Edge-Topological-Visualizer-&-Querier/# Mesh topology and queries
│   ├── main.cpp                                      # C++/OpenGL source code
│   ├── objeto.obj                                    # Sample 2D object file
│   └── README.md                                     # Documentation for this specific project
│
├── 03__2D-Half-Edge-Object-Transformer-&-Rasterizer/ # Transformations and custom rasterization
│   ├── main.cpp                                      # C++/OpenGL source code
│   ├── objeto.obj                                    # Sample 2D object file
│   └── README.md                                     # Documentation for this specific project
│
└── 04__Multi-Object-Half-Edge-Visualizer/            # Multi-object support and CS clipping
    ├── main.cpp                                      # C++/OpenGL source code
    └── README.md                                     # Documentation for this specific project


Each folder below represents a specific challenge in the field of graphics and geometry.

### 01. Weiler-Atherton Polygon Clipping
* **Language:** Python
* **Description:** Implementation of the Weiler-Atherton algorithm to clip concave polygons. Features a graphical visualization using Matplotlib.
* **Key Topics:** Intersection detection, In/Out classification, and list traversing.
* [**View Project Details**](./01__Weiler-Atherton/README.md)

### 02. 2D Half-Edge Topological Visualizer & Querier
* **Language:** C++ / OpenGL
* **Description:** A tool to explore the topology of `.obj` files. It builds a Half-Edge structure and allows the user to query adjacency between vertices, edges, and faces in real-time via terminal.
* **Key Topics:** Mesh navigation, Topological queries, and Half-Edge persistence.
* [**View Project Details**](./02__2D-Half-Edge-Topological-Visualizer-&-Querier/README.md)

### 03. 2D Half-Edge Object Transformer & Rasterizer
* **Language:** C++ / OpenGL
* **Description:** Focuses on geometric transformations (Translation, Rotation, Scaling, Shearing, Reflection) applied directly to a Half-Edge mesh, with custom rasterization.
* **Key Topics:** Bresenham and Xiaolin Wu algorithms, Centroid-based transformations.
* [**View Project Details**](./03__2D-Half-Edge-Object-Transformer-&-Rasterizer/README.md)

### 04. Multi-Object Half-Edge Visualizer
* **Language:** C++ / OpenGL
* **Description:** An advanced visualizer that supports multiple objects simultaneously and implements the **Cohen-Sutherland** line clipping algorithm in world coordinates.
* **Key Topics:** Multi-object active focus, Cohen-Sutherland clipping, Viewport transformation.
* [**View Project Details**](./04__Multi-Object-Half-Edge-Visualizer/README.md)

---

## 🛠️ Main Technologies & Algorithms

| Category | Skills / Algorithms |
| :--- | :--- |
| **Languages** | C++, Python |
| **Graphics Libs** | OpenGL (GLUT/FreeGLUT), Matplotlib |
| **Data Structures** | Half-Edge (DCEL), Linked Lists |
| **Clipping** | Weiler-Atherton, Cohen-Sutherland |
| **Rasterization** | Bresenham, Xiaolin Wu (Anti-aliasing) |

---

## ⚙️ How to Run
Most C++ projects require an OpenGL environment. 
1. **Clone the repository:**
   ```bash
   git clone [https://github.com/gabsxramos/COMPUTER_GRAPHICS_EXERCISES_2025.git](https://github.com/gabsxramos/COMPUTER_GRAPHICS_EXERCISES_2025.git)