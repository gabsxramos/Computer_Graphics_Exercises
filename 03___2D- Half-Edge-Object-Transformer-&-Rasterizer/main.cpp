/* 
 * NOVAS FUNCIONALIDADES:
 *
 * 1. Algoritmos de Rasterização:
 * - O programa renderiza as arestas usando os algoritmos de Bresenham ou Xiaolin Wu.
 * - A escolha é feita por um parâmetro de inicialização na linha de comando.
 *
 * 2. Transformações Geométricas 2D:
 * - As teclas de consulta ('1'-'4') foram substituídas por teclas de transformação.
 * - As transformações (translação, escala, rotação, cisalhamento, reflexão)
 * são aplicadas diretamente aos vértices na estrutura de dados Half-Edge.
 *
 * 3. Salvar em .obj:
 * - O programa pode salvar o estado atual (transformado) do objeto em um novo
 * arquivo .obj.
 *
 * Compilação e Uso:
 * linux: g++ main.cpp -o visualizador -lglut -lGL -lGLU -lpthread
 * windows: g++ main.cpp -o visualizador -lfreeglut -lopengl32 -lglu32 -lpthread
 * 
 * Run: Selecione o algoritmo de rasterização e o arquivo de saída opcionalmente.
 * ./visualizador.exe objeto.obj <algoritmo> [arquivo_saida.obj]
 *
 * EXEMPLOS:
 * // Usar Bresenham
 * ./visualizador.exe objeto.obj bresenham output_b.obj
 *
 * // Usar Xiaolin Wu
 * ./visualizador.exe objeto.obj xiaolin_wu output_x.obj
 *
 *
 * CONTROLES (pressionados na janela gráfica):
 * - Teclas 'v', 'e', 'f': Ativam/desativam a visualização dos IDs.
 * - Tecla 'w': Salva o estado atual do objeto no arquivo de saída especificado.
 *
 * --- Teclas de Transformação (entrada de dados no console) ---
 * - 't': Translação (pede dx, dy)
 * - 's': Escala (pede sx, sy em torno do centro do objeto)
 * - 'r': Rotação (pede angulo em graus em torno do centro do objeto)
 * - 'h': Cisalhamento (Shear) (pede shx, shy)
 * - 'm': Reflexão (Mirror) (pede eixo 'x' ou 'y')
 *
 * - Tecla 'ESC': Encerra o programa.
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <utility>
#include <algorithm>
#include <cmath>
#include <GL/glut.h>
#include <set>
#include <iomanip> // Para std::setprecision ao salvar

// --- Estruturas de Dados (Half-Edge) ---
struct HalfEdge;
struct Vertex;
struct Face;

struct Vertex {
    float x, y; // Coordenadas no ESPAÇO DE MUNDO [-1, 1]
    HalfEdge* leaving = nullptr;
    int id;
};

struct Face {
    HalfEdge* edge = nullptr;
    int id;
};

struct HalfEdge {
    Vertex* origin = nullptr;
    HalfEdge* twin = nullptr;
    HalfEdge* next = nullptr;
    Face* face = nullptr;
    int id;
};

// --- Variáveis Globais ---
std::vector<Vertex*> g_vertices;
std::vector<Face*> g_faces;
std::vector<HalfEdge*> g_halfEdges;

bool showVertexIDs = true;
bool showEdgeIDs = true;
bool showFaceIDs = true;

enum DrawAlgorithm { BRESENHAM, XIAOLIN_WU };
DrawAlgorithm g_drawAlgorithm = BRESENHAM;
std::string g_outputFilename = "output.obj";
int g_windowWidth = 600;
int g_windowHeight = 600;
float g_worldMargin = 0.8f;


std::pair<int, int> worldToPixel(float x, float y) {
    float normX = (x * g_worldMargin + 1.0f) * 0.5f;
    float normY = (y * g_worldMargin + 1.0f) * 0.5f;
    int px = static_cast<int>(normX * g_windowWidth);
    int py = static_cast<int>(normY * g_windowHeight);
    return {px, py};
}

std::pair<float, float> pixelToWorld(float px, float py) {
    float normX = px / g_windowWidth;
    float normY = py / g_windowHeight;
    float x = ((normX * 2.0f) - 1.0f) / g_worldMargin;
    float y = ((normY * 2.0f) - 1.0f) / g_worldMargin;
    return {x, y};
}

void drawText(int x, int y, const std::string& text) {
    glRasterPos2i(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}


void plotPixel(int x, int y) {
    glVertex2i(x, y);
}

void drawBresenham(int x1, int y1, int x2, int y2) {
    int dx = std::abs(x2 - x1);
    int dy = -std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx + dy;  

    while (true) {
        plotPixel(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { 
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) { 
            err += dx;
            y1 += sy;
        }
    }
}

inline float fractionalPart(float x) { return x - std::floor(x); }
inline float oneMinusFractionalPart(float x) { return 1.0f - fractionalPart(x); }
void plotPixelWu(int x, int y, float brightness) {
    glColor4f(1.0f, 1.0f, 1.0f, brightness);
    plotPixel(x, y);
}

void drawXiaolinWu(float x1, float y1, float x2, float y2) {
    bool steep = std::abs(y2 - y1) > std::abs(x2 - x1);
    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    float dx = x2 - x1;
    float dy = y2 - y1;
    float gradient = (dx == 0) ? 1.0 : dy / dx;

    int xend = static_cast<int>(round(x1));
    float yend = y1 + gradient * (xend - x1);
    float xgap = oneMinusFractionalPart(x1 + 0.5f);
    int xpxl1 = xend;
    int ypxl1 = static_cast<int>(yend);
    if (steep) {
        plotPixelWu(ypxl1, xpxl1, oneMinusFractionalPart(yend) * xgap);
        plotPixelWu(ypxl1 + 1, xpxl1, fractionalPart(yend) * xgap);
    } else {
        plotPixelWu(xpxl1, ypxl1, oneMinusFractionalPart(yend) * xgap);
        plotPixelWu(xpxl1, ypxl1 + 1, fractionalPart(yend) * xgap);
    }
    float intery = yend + gradient;

    xend = static_cast<int>(round(x2));
    yend = y2 + gradient * (xend - x2);
    xgap = fractionalPart(x2 + 0.5f);
    int xpxl2 = xend;
    int ypxl2 = static_cast<int>(yend);
    if (steep) {
        plotPixelWu(ypxl2, xpxl2, oneMinusFractionalPart(yend) * xgap);
        plotPixelWu(ypxl2 + 1, xpxl2, fractionalPart(yend) * xgap);
    } else {
        plotPixelWu(xpxl2, ypxl2, oneMinusFractionalPart(yend) * xgap);
        plotPixelWu(xpxl2, ypxl2 + 1, fractionalPart(yend) * xgap);
    }
    if (steep) {
        for (int x = xpxl1 + 1; x < xpxl2; ++x) {
            plotPixelWu(static_cast<int>(intery), x, oneMinusFractionalPart(intery));
            plotPixelWu(static_cast<int>(intery) + 1, x, fractionalPart(intery));
            intery += gradient;
        }
    } else {
        for (int x = xpxl1 + 1; x < xpxl2; ++x) {
            plotPixelWu(x, static_cast<int>(intery), oneMinusFractionalPart(intery));
            plotPixelWu(x, static_cast<int>(intery) + 1, fractionalPart(intery));
            intery += gradient;
        }
    }
}

std::pair<float, float> getCentroid() {
    float cX = 0, cY = 0;
    if (g_vertices.empty()) return {0, 0};
    for (const auto& vertex : g_vertices) {
        cX += vertex->x;
        cY += vertex->y;
    }
    return {cX / g_vertices.size(), cY / g_vertices.size()};
}

void applyTranslation(float dx, float dy) {
    std::cout << "  > Aplicando Translação (" << dx << ", " << dy << ")" << std::endl;
    for (Vertex* v : g_vertices) {
        v->x += dx;
        v->y += dy;
    }
}

void applyScaling(float sx, float sy) {
    std::cout << "  > Aplicando Escala (" << sx << ", " << sy << ")" << std::endl;
    std::pair<float, float> center = getCentroid();
    for (Vertex* v : g_vertices) {
        v->x = center.first + (v->x - center.first) * sx;
        v->y = center.second + (v->y - center.second) * sy;
    }
}

void applyRotation(float angleDeg) {
    std::cout << "  > Aplicando Rotação (" << angleDeg << " graus)" << std::endl;
    float angleRad = angleDeg * 3.14159265f / 180.0f;
    float cosA = std::cos(angleRad);
    float sinA = std::sin(angleRad);
    std::pair<float, float> center = getCentroid();
    for (Vertex* v : g_vertices) {
        float tx = v->x - center.first;
        float ty = v->y - center.second;
        v->x = center.first + tx * cosA - ty * sinA;
        v->y = center.second + tx * sinA + ty * cosA;
    }
}

void applyShearing(float shx, float shy) {
    std::cout << "  > Aplicando Cisalhamento (" << shx << ", " << shy << ")" << std::endl;
    std::pair<float, float> center = getCentroid(); 
    for (Vertex* v : g_vertices) {
        float tx = v->x - center.first;
        float ty = v->y - center.second;
        v->x = center.first + tx + shx * ty;
        v->y = center.second + ty + shy * tx;
    }
}

void applyReflection(bool reflectX, bool reflectY) {
    std::cout << "  > Aplicando Reflexão (X=" << reflectX << ", Y=" << reflectY << ")" << std::endl;
    std::pair<float, float> center = getCentroid();
    for (Vertex* v : g_vertices) {
        if (reflectX) {
            v->x = center.first - (v->x - center.first);
        }
        if (reflectY) {
            v->y = center.second - (v->y - center.second);
        }
    }
}

void buildHalfEdgeStructure(const std::vector<std::vector<int>>& face_indices) {
    std::map<std::pair<int, int>, HalfEdge*> edgeMap;
    int edgeIdCounter = 0;
    for (size_t i = 0; i < face_indices.size(); ++i) {
        const auto& indices = face_indices[i];
        Face* f = new Face(); f->id = i; g_faces.push_back(f);
        std::vector<HalfEdge*> faceEdges;
        for (size_t j = 0; j < indices.size(); ++j) {
            int startIndex = indices[j]; int endIndex = indices[(j + 1) % indices.size()];
            HalfEdge* he = new HalfEdge(); he->id = edgeIdCounter++; he->origin = g_vertices[startIndex]; he->face = f;
            if (g_vertices[startIndex]->leaving == nullptr) { g_vertices[startIndex]->leaving = he; }
            faceEdges.push_back(he); g_halfEdges.push_back(he); edgeMap[{startIndex, endIndex}] = he;
        }
        f->edge = faceEdges[0];
        for (size_t j = 0; j < faceEdges.size(); ++j) { faceEdges[j]->next = faceEdges[(j + 1) % faceEdges.size()]; }
    }
    for (auto const& [key, val] : edgeMap) {
        int start = key.first; int end = key.second;
        if (edgeMap.count({end, start})) { val->twin = edgeMap[{end, start}]; }
    }
}

bool loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) { std::cerr << "Erro: Nao foi possivel abrir o arquivo " << path << std::endl; return false; }
    std::vector<std::vector<int>> face_indices;
    std::string line; int vertexIdCounter = 0;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') { line.pop_back(); }
        std::stringstream ss(line); std::string prefix; ss >> prefix;
        if (prefix == "v") {
            Vertex* v = new Vertex();
            ss >> v->x >> v->y; 
            v->id = vertexIdCounter++;
            g_vertices.push_back(v);
        } else if (prefix == "f") {
            std::vector<int> indices; int index;
            while (ss >> index) { indices.push_back(index - 1); }
            face_indices.push_back(indices);
        }
    }
    buildHalfEdgeStructure(face_indices);
    return true;
}

void saveOBJ(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Erro: Nao foi possivel criar o arquivo de saida " << filename << std::endl;
        return;
    }

    std::cout << " > Salvando estado atual em " << filename << "..." << std::endl;

    std::map<Vertex*, int> vertexIndexMap;
    outFile << std::fixed << std::setprecision(6);

    for (size_t i = 0; i < g_vertices.size(); ++i) {
        Vertex* v = g_vertices[i];
        //std::pair<float, float> worldCoords = pixelToWorld(v->x, v->y);
        //outFile << "v " << worldCoords.first << " " << worldCoords.second << " 0.0" << std::endl;
        outFile << "v " << v->x << " " << v->y << " 0.0" << std::endl;
        vertexIndexMap[v] = i + 1;
    }

    outFile << std::endl;

    for (const auto& face : g_faces) {
        outFile << "f";
        HalfEdge* startEdge = face->edge;
        HalfEdge* currentEdge = startEdge;
        do {
            outFile << " " << vertexIndexMap[currentEdge->origin];
            currentEdge = currentEdge->next;
        } while (currentEdge != startEdge);
        outFile << std::endl;
    }

    outFile.close();
    std::cout << " > Salvo com sucesso." << std::endl;
}


void init() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, g_windowWidth, 0, g_windowHeight);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor4f(0.8f, 0.8f, 1.0f, 1.0f);
    
    glBegin(GL_POINTS);
    for(const auto& edge : g_halfEdges) {
        if (edge->twin == nullptr || edge->id < edge->twin->id) {
            if (edge->origin && edge->next && edge->next->origin) {
                Vertex* v1 = edge->origin;
                Vertex* v2 = edge->next->origin;

                std::pair<int, int> p1 = worldToPixel(v1->x, v1->y);
                std::pair<int, int> p2 = worldToPixel(v2->x, v2->y);

                if (g_drawAlgorithm == BRESENHAM) {
                    drawBresenham(p1.first, p1.second, p2.first, p2.second);
                } else {
                    drawXiaolinWu(p1.first, p1.second, p2.first, p2.second);
                }
            }
        }
    }
    glEnd();


    if (showFaceIDs) {
        glColor3f(0.0f, 1.0f, 0.0f); 
        for (const auto& face : g_faces) {
            float cX = 0, cY = 0; 
            int numVertices = 0;
            HalfEdge* startEdge = face->edge;
            HalfEdge* currentEdge = startEdge;
            do {
                cX += currentEdge->origin->x;
                cY += currentEdge->origin->y;
                numVertices++;
                currentEdge = currentEdge->next;
            } while (currentEdge != startEdge);
            
            if (numVertices > 0) {
                std::pair<int, int> p = worldToPixel(cX / numVertices, cY / numVertices);
                drawText(p.first, p.second, "F" + std::to_string(face->id));
            }
        }
    }

    if (showEdgeIDs) {
        glColor3f(1.0f, 1.0f, 0.0f); 
        for (const auto& edge : g_halfEdges) {
            if (edge->twin == nullptr || edge->id < edge->twin->id) {
                if (edge->origin && edge->next && edge->next->origin) {
                    Vertex* v1 = edge->origin;
                    Vertex* v2 = edge->next->origin;
                    float mX = (v1->x + v2->x) / 2.0f;
                    float mY = (v1->y + v2->y) / 2.0f;
                    
                    std::pair<int, int> p = worldToPixel(mX, mY);
                    drawText(p.first, p.second, "E" + std::to_string(edge->id));
                }
            }
        }
    }
    
    if (showVertexIDs) {
        glColor3f(1.0f, 0.5f, 0.0f); 
        for (const auto& vertex : g_vertices) {
            std::pair<int, int> p = worldToPixel(vertex->x, vertex->y);
            drawText(p.first + 3, p.second + 3, "V" + std::to_string(vertex->id));
        }
    }

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    float dx, dy, sx, sy, angle, shx, shy;
    char axis;

    switch (key) {
        case 27: exit(0); break;

        // Teclas para Ligar/Desligar a exibição dos IDs
        case 'f': showFaceIDs = !showFaceIDs; break;
        case 'e': showEdgeIDs = !showEdgeIDs; break;
        case 'v': showVertexIDs = !showVertexIDs; break;

        // 'w' -> Write (Salvar)
        case 'w':
        case 'W':
            saveOBJ(g_outputFilename);
            break;

        // 't' -> Translação
        case 't':
        case 'T':
            std::cout << "\n[TRANSFORMAÇÃO] Digite dx dy (ex: 0.1 0.0): ";
            std::cin >> dx >> dy;
            applyTranslation(dx, dy);
            break;

        // 's' -> Escala
        case 's':
        case 'S':
            std::cout << "\n[TRANSFORMAÇÃO] Digite sx sy (ex: 1.1 1.1): ";
            std::cin >> sx >> sy;
            applyScaling(sx, sy);
            break;

        // 'r' -> Rotação
        case 'r':
        case 'R':
            std::cout << "\n[TRANSFORMAÇÃO] Digite o angulo (ex: 15.0): ";
            std::cin >> angle;
            applyRotation(angle);
            break;

        // 'h' -> Cisalhamento (Shear)
        case 'h':
        case 'H':
            std::cout << "\n[TRANSFORMAÇÃO] Digite shx shy (ex: 0.1 0.0): ";
            std::cin >> shx >> shy;
            applyShearing(shx, shy);
            break;
        
        // 'm' -> Reflexão (Mirror)
        case 'm':
        case 'M':
            std::cout << "\n[TRANSFORMAÇÃO] Digite o eixo (x ou y): ";
            std::cin >> axis;
            applyReflection( (axis == 'x' || axis == 'X'), (axis == 'y' || axis == 'Y') );
            break;
    } 
    glutPostRedisplay();
}


int main(int argc, char** argv) {
  if (argc < 3) { 
    std::cerr << "Uso: " << argv[0] << " <arquivo_entrada.obj> <bresenham|xiaolin_wu> [arquivo_saida.obj]" << std::endl; 
    return 1; 
  }
  
  if (!loadOBJ(argv[1])) { return 1; }

  std::string algorithmStr = argv[2];
  if (algorithmStr == "bresenham") {
      g_drawAlgorithm = BRESENHAM;
      std::cout << "Usando algoritmo de Bresenham." << std::endl;
  } else if (algorithmStr == "xiaolin_wu") {
      g_drawAlgorithm = XIAOLIN_WU;
      std::cout << "Usando algoritmo de Xiaolin Wu." << std::endl;
  } else {
      std::cerr << "Algoritmo '" << algorithmStr << "' nao reconhecido. Use 'bresenham' or 'xiaolin_wu'." << std::endl;
      return 1;
  }

  if (argc >= 4) {
      g_outputFilename = argv[3];
  }
  std::cout << "Arquivo de saida (ao pressionar 'w'): " << g_outputFilename << std::endl;

  
  std::cout << "Arquivo OBJ carregado e estrutura Half-Edge construida com sucesso." << std::endl;
  std::cout << "Total de Vertices: " << g_vertices.size() << std::endl;
  std::cout << "Total de Faces: " << g_faces.size() << std::endl;
  std::cout << "Total de Half-Edges: " << g_halfEdges.size() << std::endl;
  
  std::cout << "\n--- Janela Grafica Iniciada ---" << std::endl;
  std::cout << "\n--- Pressione as teclas na JANELA GRAFICA para interagir ---" << std::endl;
  std::cout << "--- Controles de Exibicao ---" << std::endl;
  std::cout << " [V] - Liga/Desliga IDs dos Vertices" << std::endl;
  std::cout << " [E] - Liga/Desliga IDs das Arestas" << std::endl;
  std::cout << " [F] - Liga/Desliga IDs das Faces" << std::endl;
  std::cout << "\n--- Teclas de Transformacao (entrada no terminal) ---" << std::endl;
  std::cout << " [T] - Translação (dx dy)" << std::endl;
  std::cout << " [S] - Escala (sx sy)" << std::endl;
  std::cout << " [R] - Rotação (angulo)" << std::endl;
  std::cout << " [H] - Cisalhamento (shx shy)" << std::endl;
  std::cout << " [M] - Reflexão (eixo 'x' ou 'y')" << std::endl;
  std::cout << "\n--- Outros Controles ---" << std::endl;
  std::cout << " [W] - Salvar estado atual para '" << g_outputFilename << "'" << std::endl;
  std::cout << " [ESC] - Fechar o programa." << std::endl;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // RGBA para blending
  glutInitWindowSize(g_windowWidth, g_windowHeight);
  glutCreateWindow("Visualizador Half-Edge (Atividade 4)");
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  init();
  glutPostRedisplay();
  glutMainLoop();

  return 0;
}