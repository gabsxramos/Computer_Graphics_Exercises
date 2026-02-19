# Polygon Clipping Algorithm: Weiler-Atherton

This project is a Python implementation of the **Weiler-Atherton** algorithm, used in Computer Graphics for polygon clipping. Unlike simpler algorithms (such as Sutherland-Hodgman), Weiler-Atherton can handle concave polygons and can generate multiple separate polygons as the result of a single clipping operation.

---

## 🚀 Features

* **Intersection Detection:** Identifies points where the main polygon edges cross the clipping polygon.
* **Point Classification:** Determines if intersection points are "Entering" or "Exiting" the clipping area.
* **List Traversing:** Navigates between the vertex lists of both the subject and clipping polygons to construct new geometric shapes.
* **Graphical Visualization:** Uses `Matplotlib` to display the original subject, the clipping area, and the final filled result.

---

## 🛠️ Technologies Used

* **Python 3.x**
* **Matplotlib:** For rendering and polygon visualization.
* **Static Typing (`typing`):** For better code clarity and maintainability.

---

## 📖 How the Algorithm Works

The process is divided into five main stages:

1.  **Intersection Generation:** Finds all points where the subject polygon ($py$) intersects the clipping polygon ($py\_clip$).
2.  **Combined List Creation:** Inserts the intersection points between the original vertices of both polygons, maintaining geometric order.
3.  **In/Out Marking:** Defines whether each intersection represents an entry into the clipping polygon or an exit from it (using *Ray Casting* logic).

4.  **Traversing:**
    * Starts at an "Entry" point.
    * Follows the subject polygon list until an "Exit" point is reached.
    * Jumps to the clipping polygon list and follows it until the original "Entry" point is found.
5.  **Polygon Closing:** Repeats the traversing process until all entry intersections have been visited.

---

## 🔧 Execution

### Prerequisites
Make sure you have `matplotlib` and `numpy` installed:
```bash
pip install matplotlib 
```

## Running the script
Simply run the main Python file:
```bash
python weiler_atherton.py
```

---

## 📊 Example Output

Upon running the code, a graphical window will open displaying the clipping result.

* **Red Line:** The original subject polygon.
* **Green Line:** The clipping polygon (the "window").
* **Blue Shaded Area:** The final resulting polygon(s).

# Console Output:
The terminal will provide the specific coordinates of the intersection points and the final vertices of the clipped polygon(s):
```bash
Clipped Polygon 1: [(x1, y1), (x2, y2), ...]
Clipping completed. 1 polygon(s) resulting.
```
---

## 📂 Class Structure

The code is organized into clear data structures to manage the complexity of the list-jumping logic:

* **`IntersectionPoint`**: The core data unit. It stores the $(x, y)$ coordinates, a `point_flag` (identifying if it's a vertex or an intersection), and the `in_flag` (identifying entry or exit).
* **`Polygon`**: A class that handles a collection of points and includes the `draw_pg_line` method to interface with **Matplotlib**.
* **Geometric Functions**: Includes `is_point_inside_pg` (Ray Casting), `cross_product`, and `segments_intersect` for spatial calculations.
* **`weiler_atherton()`**: The main orchestrator function that calls the generation, marking, and traversing logic.

---