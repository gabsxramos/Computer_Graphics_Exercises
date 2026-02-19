/* 
 * =========================================================================================
 * DESCRIÇÃO DO PROGRAMA ( Versão Aprimorada com Clipping Cohen-Sutherland)
 * =========================================================================================
 *
 * NOVAS FUNCIONALIDADES/Aprimoramentos:
 *
 * 1. Suporte a Múltiplos Objetos: (Mantido)
 * 2. Transformação de Viewport Refinada: (Mantido)
 * 3. Transformações Geométricas 2D: (Mantido)
 * 4. Algoritmos de Rasterização: (Mantido)
 * 5. Clipping de Linha: Implementado ativamente via Cohen-Sutherland nas coordenadas de mundo.
 *
 * Compilação e Uso:
 * linux: g++ main.cpp -o visualizador -lglut -lGL -lGLU -lpthread
 * windows: g++ main.cpp -o visualizador -lfreeglut -lopengl32 -lglu32 -lpthread
 * * Run: Selecione o algoritmo e os arquivos OBJ. O primeiro OBJ será o objeto ativo.
 * ./visualizador.exe <algoritmo> <objeto1.obj> [objeto2.obj ...] [arquivo_saida.obj]
 *
 * CONTROLES (pressionados na janela gráfica):
 * - Teclas 'v', 'e', 'f': Ativam/desativam a visualização dos IDs no objeto ativo.
 * - Tecla 'n': Alterna para o Próximo Objeto (torna-o o objeto ativo).
 * - Tecla 'w': Salva o estado atual do objeto ATIVO no arquivo de saída.
 * - 't', 's', 'r', 'h', 'm': Aplicam a transformação ao objeto ATIVO (entrada de dados no console).
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
#include <GL/freeglut.h>
#include <set>
#include <iomanip>

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

// --- Estrutura para Múltiplos Objetos ---
struct GraphicalObject {
    std::string name;
    std::vector<Vertex*> vertices;
    std::vector<Face*> faces;
    std::vector<HalfEdge*> halfEdges;
    std::pair<float, float> getCentroid() const {
        float cX = 0, cY = 0;
        if (vertices.empty()) return {0, 0};
        for (const auto& vertex : vertices) {
            cX += vertex->x;
            cY += vertex->y;
        }
        return {cX / vertices.size(), cY / vertices.size()};
    }
};


// --- Variáveis Globais ---
std::vector<GraphicalObject*> g_objects;
GraphicalObject* g_activeObject = nullptr;

bool showVertexIDs = true;
bool showEdgeIDs = true;
bool showFaceIDs = true;

enum DrawAlgorithm { BRESENHAM, XIAOLIN_WU };
DrawAlgorithm g_drawAlgorithm = BRESENHAM;
std::string g_outputFilename = "output.obj";
int g_windowWidth = 600;
int g_windowHeight = 600;


// --- CONSTANTES DE CLIPPING (Coordenadas de Mundo) ---
const float X_MIN = -1.0f;
const float X_MAX = 1.0f;
const float Y_MIN = -1.0f;
const float Y_MAX = 1.0f;


// --- TRANSFORMACAO DE VIEWPORT (Mundo [-1, 1] para Pixel [0, W]x[0, H]) ---
std::pair<int, int> worldToPixel(float x, float y) {
    // Mapeia x de [-1, 1] para [0, 1] (Normalização)
    float normX = (x - X_MIN) / (X_MAX - X_MIN); 
    // Mapeia y de [-1, 1] para [0, 1] (Normalização)
    float normY = (y - Y_MIN) / (Y_MAX - Y_MIN);
    
    // Mapeia [0, 1] para [0, W]x[0, H] (Viewport)
    int px = static_cast<int>(normX * g_windowWidth);
    int py = static_cast<int>(normY * g_windowHeight);
    return {px, py};
}


// --- ALGORITMOS DE RASTERIZAÇÃO (Mantidos) ---
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
    // A cor de base é definida no display
    glColor4f(1.0f, 1.0f, 1.0f, brightness); 
    plotPixel(x, y);
}

void drawXiaolinWu(float x1, float y1, float x2, float y2) {
    // ... (Implementação de Xiaolin Wu, mantida a original) ...
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


// --- ALGORITMO DE CLIPPING DE LINHA (Cohen-Sutherland) ---

// Códigos de Região (Outcodes)
enum { INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8 };

int computeCode(float x, float y) {
    int code = INSIDE;
    if (x < X_MIN)        code |= LEFT;
    else if (x > X_MAX)   code |= RIGHT;
    if (y < Y_MIN)        code |= BOTTOM;
    else if (y > Y_MAX)   code |= TOP;
    return code;
}

// Algoritmo Cohen-Sutherland para clipping de linha
void cohenSutherlandClip(float& x1, float& y1, float& x2, float& y2) {
    int code1 = computeCode(x1, y1);
    int code2 = computeCode(x2, y2);
    bool done = false;

    while (!done) {
        if ((code1 | code2) == INSIDE) { // Trivial accept
            done = true;
        } else if (code1 & code2) { // Trivial reject
            return; // Linha completamente fora
        } else {
            // Caso de intersecção, precisa clipar
            int code_out = code1 ? code1 : code2;
            float x = 0, y = 0;

            if (code_out & TOP) { // Interseção com limite superior
                x = x1 + (x2 - x1) * (Y_MAX - y1) / (y2 - y1);
                y = Y_MAX;
            } else if (code_out & BOTTOM) { // Interseção com limite inferior
                x = x1 + (x2 - x1) * (Y_MIN - y1) / (y2 - y1);
                y = Y_MIN;
            } else if (code_out & RIGHT) { // Interseção com limite direito
                y = y1 + (y2 - y1) * (X_MAX - x1) / (x2 - x1);
                x = X_MAX;
            } else if (code_out & LEFT) { // Interseção com limite esquerdo
                y = y1 + (y2 - y1) * (X_MIN - x1) / (x2 - x1);
                x = X_MIN;
            }
            
            // Atualiza o ponto e o código de região
            if (code_out == code1) {
                x1 = x;
                y1 = y;
                code1 = computeCode(x1, y1);
            } else {
                x2 = x;
                y2 = y;
                code2 = computeCode(x2, y2);
            }
        }
    }
    
    // Se a linha foi clipada e aceita, ela será desenhada no final da função chamadora.
}


// --- Funções de Transformação (Mantidas) ---
void applyTranslation(GraphicalObject* obj, float dx, float dy) {
    // ... (Implementação de Translação mantida) ...
    if (!obj) return;
    std::cout << "  > Aplicando Translação (" << dx << ", " << dy << ") ao objeto: " << obj->name << std::endl;
    for (Vertex* v : obj->vertices) {
        v->x += dx;
        v->y += dy;
    }
}

void applyScaling(GraphicalObject* obj, float sx, float sy) {
    // ... (Implementação de Escala mantida) ...
    if (!obj) return;
    std::cout << "  > Aplicando Escala (" << sx << ", " << sy << ") ao objeto: " << obj->name << std::endl;
    std::pair<float, float> center = obj->getCentroid();
    for (Vertex* v : obj->vertices) {
        v->x = center.first + (v->x - center.first) * sx;
        v->y = center.second + (v->y - center.second) * sy;
    }
}

void applyRotation(GraphicalObject* obj, float angleDeg) {
    // ... (Implementação de Rotação mantida) ...
    if (!obj) return;
    std::cout << "  > Aplicando Rotação (" << angleDeg << " graus) ao objeto: " << obj->name << std::endl;
    float angleRad = angleDeg * 3.14159265f / 180.0f;
    float cosA = std::cos(angleRad);
    float sinA = std::sin(angleRad);
    std::pair<float, float> center = obj->getCentroid();
    for (Vertex* v : obj->vertices) {
        float tx = v->x - center.first;
        float ty = v->y - center.second;
        v->x = center.first + tx * cosA - ty * sinA;
        v->y = center.second + tx * sinA + ty * cosA;
    }
}

void applyShearing(GraphicalObject* obj, float shx, float shy) {
    // ... (Implementação de Cisalhamento mantida) ...
    if (!obj) return;
    std::cout << "  > Aplicando Cisalhamento (" << shx << ", " << shy << ") ao objeto: " << obj->name << std::endl;
    std::pair<float, float> center = obj->getCentroid(); 
    for (Vertex* v : obj->vertices) {
        float tx = v->x - center.first;
        float ty = v->y - center.second;
        v->x = center.first + tx + shx * ty;
        v->y = center.second + ty + shy * tx;
    }
}

void applyReflection(GraphicalObject* obj, bool reflectX, bool reflectY) {
    // ... (Implementação de Reflexão mantida) ...
    if (!obj) return;
    std::cout << "  > Aplicando Reflexão (X=" << reflectX << ", Y=" << reflectY << ") ao objeto: " << obj->name << std::endl;
    std::pair<float, float> center = obj->getCentroid();
    for (Vertex* v : obj->vertices) {
        if (reflectX) {
            v->x = center.first - (v->x - center.first);
        }
        if (reflectY) {
            v->y = center.second - (v->y - center.second);
        }
    }
}


// --- Funções de Construção/Leitura/Escrita (Mantidas) ---
void buildHalfEdgeStructure(GraphicalObject* obj, const std::vector<std::vector<int>>& face_indices) {
    // ... (Implementação de Half-Edge mantida) ...
    std::map<std::pair<int, int>, HalfEdge*> edgeMap;
    int edgeIdCounter = 0;
    for (size_t i = 0; i < face_indices.size(); ++i) {
        const auto& indices = face_indices[i];
        Face* f = new Face(); f->id = i; obj->faces.push_back(f);
        std::vector<HalfEdge*> faceEdges;
        for (size_t j = 0; j < indices.size(); ++j) {
            int startIndex = indices[j]; int endIndex = indices[(j + 1) % indices.size()];
            HalfEdge* he = new HalfEdge(); he->id = edgeIdCounter++; he->origin = obj->vertices[startIndex]; he->face = f;
            if (obj->vertices[startIndex]->leaving == nullptr) { obj->vertices[startIndex]->leaving = he; }
            faceEdges.push_back(he); obj->halfEdges.push_back(he); edgeMap[{startIndex, endIndex}] = he;
        }
        f->edge = faceEdges[0];
        for (size_t j = 0; j < faceEdges.size(); ++j) { faceEdges[j]->next = faceEdges[(j + 1) % faceEdges.size()]; }
    }
    for (auto const& [key, val] : edgeMap) {
        int start = key.first; int end = key.second;
        if (edgeMap.count({end, start})) { val->twin = edgeMap[{end, start}]; }
    }
}

GraphicalObject* loadOBJ(const std::string& path) {
    // ... (Implementação de loadOBJ mantida) ...
    std::ifstream file(path);
    if (!file.is_open()) { std::cerr << "Erro: Nao foi possivel abrir o arquivo " << path << std::endl; return nullptr; }
    
    GraphicalObject* newObj = new GraphicalObject();
    newObj->name = path;
    
    std::vector<std::vector<int>> face_indices;
    std::string line; int vertexIdCounter = 0;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') { line.pop_back(); }
        std::stringstream ss(line); std::string prefix; ss >> prefix;
        if (prefix == "v") {
            Vertex* v = new Vertex();
            ss >> v->x >> v->y; 
            v->id = vertexIdCounter++;
            newObj->vertices.push_back(v);
        } else if (prefix == "f") {
            std::vector<int> indices; int index;
            while (ss >> index) { indices.push_back(index - 1); }
            face_indices.push_back(indices);
        }
    }
    buildHalfEdgeStructure(newObj, face_indices);
    return newObj;
}

void saveOBJ(GraphicalObject* obj, const std::string& filename) {
    // ... (Implementação de saveOBJ mantida) ...
    if (!obj) { std::cerr << "Erro: Nenhum objeto ativo para salvar." << std::endl; return; }
    std::ofstream outFile(filename);
    if (!outFile.is_open()) { std::cerr << "Erro: Nao foi possivel criar o arquivo de saida " << filename << std::endl; return; }

    std::cout << " > Salvando estado atual do objeto '" << obj->name << "' em " << filename << "..." << std::endl;

    std::map<Vertex*, int> vertexIndexMap;
    outFile << "# OBJ exportado pelo visualizador Half-Edge (Objeto: " << obj->name << ")" << std::endl;
    outFile << std::fixed << std::setprecision(6);

    for (size_t i = 0; i < obj->vertices.size(); ++i) {
        Vertex* v = obj->vertices[i];
        outFile << "v " << v->x << " " << v->y << " 0.0" << std::endl;
        vertexIndexMap[v] = i + 1;
    }

    outFile << std::endl;
    outFile << "g " << obj->name << std::endl;

    for (const auto& face : obj->faces) {
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

// --- Funções OpenGL (Modificada a rotina de desenho) ---

void drawText(int x, int y, const std::string& text) {
    glRasterPos2i(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}


void init() {
    // ... (Implementação de init mantida) ...
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    
    // Projeção: Mapeia [0, W]x[0, H] da janela para as coordenadas de desenho
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, g_windowWidth, 0, g_windowHeight);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(2.0f); // Para melhor visualização dos pixels
}

void drawGraphicalObject(GraphicalObject* obj, bool isActive) {
    if (!obj) return;
    
    // 1. Desenhar Arestas (usando clipping e algoritmo selecionado)
    if (isActive) {
        glColor4f(0.8f, 0.8f, 1.0f, 1.0f);
    } else {
        glColor4f(0.5f, 0.5f, 0.6f, 1.0f);
    }
    
    glBegin(GL_POINTS);
    for(const auto& edge : obj->halfEdges) {
        // Desenha apenas uma Half-Edge por aresta geométrica
        if (edge->twin == nullptr || edge->id < edge->twin->id) {
            if (edge->origin && edge->next && edge->next->origin) {
                float x1_w = edge->origin->x;
                float y1_w = edge->origin->y;
                float x2_w = edge->next->origin->x;
                float y2_w = edge->next->origin->y;
                
                // --- CLIPPING ATIVO ---
                cohenSutherlandClip(x1_w, y1_w, x2_w, y2_w);
                
                // Se o clipping não rejeitou a linha:
                if (computeCode(x1_w, y1_w) == INSIDE && computeCode(x2_w, y2_w) == INSIDE) {
                    // Aplica a Transformação de Viewport (Mundo -> Pixel) antes da rasterização
                    std::pair<int, int> p1 = worldToPixel(x1_w, y1_w);
                    std::pair<int, int> p2 = worldToPixel(x2_w, y2_w);

                    if (g_drawAlgorithm == BRESENHAM) {
                        drawBresenham(p1.first, p1.second, p2.first, p2.second);
                    } else {
                        drawXiaolinWu((float)p1.first, (float)p1.second, (float)p2.first, (float)p2.second);
                    }
                }
            }
        }
    }
    glEnd();
    
    // 2. Desenhar IDs (Mantido, mas só dentro da janela de clipping)
    if (isActive) {
        // ... (Desenho de IDs de Face, Aresta e Vértice - Omitido por brevidade, mas mantido o original) ...
        if (showFaceIDs) {
            glColor3f(0.0f, 1.0f, 0.0f); 
            for (const auto& face : obj->faces) {
                float cX = 0, cY = 0; int numVertices = 0;
                HalfEdge* startEdge = face->edge; HalfEdge* currentEdge = startEdge;
                do {
                    cX += currentEdge->origin->x; cY += currentEdge->origin->y;
                    numVertices++; currentEdge = currentEdge->next;
                } while (currentEdge != startEdge);
                
                if (numVertices > 0) {
                    float midX = cX / numVertices;
                    float midY = cY / numVertices;
                    if (computeCode(midX, midY) == INSIDE) { // Desenha ID só se o centroide estiver visível
                        std::pair<int, int> p = worldToPixel(midX, midY);
                        drawText(p.first, p.second, "F" + std::to_string(face->id));
                    }
                }
            }
        }

        if (showEdgeIDs) {
            glColor3f(1.0f, 1.0f, 0.0f); 
            for (const auto& edge : obj->halfEdges) {
                if (edge->twin == nullptr || edge->id < edge->twin->id) {
                    if (edge->origin && edge->next && edge->next->origin) {
                        float v1x = edge->origin->x; float v1y = edge->origin->y;
                        float v2x = edge->next->origin->x; float v2y = edge->next->origin->y;
                        float mX = (v1x + v2x) / 2.0f;
                        float mY = (v1y + v2y) / 2.0f;
                        
                        if (computeCode(mX, mY) == INSIDE) { // Desenha ID só se o ponto médio estiver visível
                            std::pair<int, int> p = worldToPixel(mX, mY);
                            drawText(p.first, p.second, "E" + std::to_string(edge->id));
                        }
                    }
                }
            }
        }
        
        if (showVertexIDs) {
            glColor3f(1.0f, 0.5f, 0.0f); 
            for (const auto& vertex : obj->vertices) {
                if (computeCode(vertex->x, vertex->y) == INSIDE) { // Desenha ID só se o vértice estiver visível
                    std::pair<int, int> p = worldToPixel(vertex->x, vertex->y);
                    drawText(p.first + 3, p.second + 3, "V" + std::to_string(vertex->id));
                }
            }
        }
    }
}

void display() {
    // ... (Implementação de display mantida) ...
    glClear(GL_COLOR_BUFFER_BIT);

    // Desenha todos os objetos
    for (GraphicalObject* obj : g_objects) {
        drawGraphicalObject(obj, obj == g_activeObject);
    }
    
    // Texto de Objeto Ativo
    if (g_activeObject) {
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(10, g_windowHeight - 20, "OBJETO ATIVO: " + g_activeObject->name);
    }

    glutSwapBuffers();
}

void switchActiveObject() {
    // ... (Implementação de switchActiveObject mantida) ...
    if (g_objects.empty()) return;
    if (g_objects.size() == 1) return;

    auto it = std::find(g_objects.begin(), g_objects.end(), g_activeObject);
    
    if (it == g_objects.end() || (it + 1) == g_objects.end()) {
        g_activeObject = g_objects.front();
    } else {
        g_activeObject = *(it + 1);
    }
    std::cout << "\n>>> Objeto ATIVO alterado para: " << g_activeObject->name << std::endl;
}

void keyboard(unsigned char key, int x, int y) {
    // ... (Implementação de keyboard mantida) ...
    float dx, dy, sx, sy, angle, shx, shy;
    char axis;

    switch (key) {
        case 27: exit(0); break;

        // Teclas para Ligar/Desligar a exibição dos IDs (no objeto ativo)
        case 'f': showFaceIDs = !showFaceIDs; break;
        case 'e': showEdgeIDs = !showEdgeIDs; break;
        case 'v': showVertexIDs = !showVertexIDs; break;
        
        // 'n' -> Next Object (Alternar objeto ativo)
        case 'n':
        case 'N':
            switchActiveObject();
            break;

        // 'w' -> Write (Salvar o objeto ativo)
        case 'w':
        case 'W':
            saveOBJ(g_activeObject, g_outputFilename);
            break;

        // --- Transformações (Aplicadas ao objeto ATIVO) ---
        case 't': case 'T':
            std::cout << "\n[TRANSFORMAÇÃO] Digite dx dy (ex: 0.1 0.0): ";
            std::cin >> dx >> dy;
            applyTranslation(g_activeObject, dx, dy);
            break;

        case 's': case 'S':
            std::cout << "\n[TRANSFORMAÇÃO] Digite sx sy (ex: 1.1 1.1): ";
            std::cin >> sx >> sy;
            applyScaling(g_activeObject, sx, sy);
            break;

        case 'r': case 'R':
            std::cout << "\n[TRANSFORMAÇÃO] Digite o angulo (ex: 15.0): ";
            std::cin >> angle;
            applyRotation(g_activeObject, angle);
            break;

        case 'h': case 'H':
            std::cout << "\n[TRANSFORMAÇÃO] Digite shx shy (ex: 0.1 0.0): ";
            std::cin >> shx >> shy;
            applyShearing(g_activeObject, shx, shy);
            break;
        
        case 'm': case 'M':
            std::cout << "\n[TRANSFORMAÇÃO] Digite o eixo (x ou y): ";
            std::cin >> axis;
            applyReflection(g_activeObject, (axis == 'x' || axis == 'X'), (axis == 'y' || axis == 'Y'));
            break;
    } 
    glutPostRedisplay();
}


int main(int argc, char** argv) {
    // ... (Implementação de main mantida) ...
    if (argc < 3) { 
        std::cerr << "Uso: " << argv[0] << " <bresenham|xiaolin_wu> <objeto1.obj> [objeto2.obj ...] [arquivo_saida.obj]" << std::endl; 
        return 1; 
    }
 
    // 1. Processar Algoritmo
    std::string algorithmStr = argv[1];
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
    
    // 2. Processar Arquivos OBJ (Permitindo múltiplos)
    int fileCount = 0;
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.find(".obj") != std::string::npos) {
            GraphicalObject* obj = loadOBJ(arg);
            if (obj) {
                g_objects.push_back(obj);
                fileCount++;
                if (!g_activeObject) {
                    g_activeObject = obj; // O primeiro objeto é o objeto ativo
                }
            }
        }
    }
    
    if (fileCount == 0) {
        std::cerr << "Erro: Nenhum arquivo OBJ de entrada valido fornecido." << std::endl;
        return 1;
    }

    // 3. Processar Arquivo de Saída (o último argumento que é OBJ)
    if (argc > 3 && std::string(argv[argc - 1]).find(".obj") != std::string::npos && std::string(argv[argc - 1]) != g_objects.back()->name) {
        g_outputFilename = argv[argc - 1];
    }
    
    std::cout << "\nTotal de Objetos Carregados: " << g_objects.size() << std::endl;
    std::cout << "Objeto ATIVO: " << g_activeObject->name << std::endl;
    std::cout << "Arquivo de saida (ao pressionar 'w'): " << g_outputFilename << std::endl;

 
    std::cout << "\n--- Janela Grafica Iniciada ---" << std::endl;
    std::cout << "\n--- Pressione as teclas na JANELA GRAFICA para interagir ---" << std::endl;
    std::cout << "--- Controles de Objeto ---" << std::endl;
    std::cout << " [N] - Alternar para o Proximo Objeto Ativo" << std::endl;
    std::cout << " [V] [E] [F] - Ligar/Desliga IDs (Objeto Ativo)" << std::endl;
    std::cout << "\n--- Teclas de Transformacao (entrada no terminal - Objeto Ativo) ---" << std::endl;
    std::cout << " [T] - Translação (dx dy)" << std::endl;
    std::cout << " [S] - Escala (sx sy)" << std::endl;
    std::cout << " [R] - Rotação (angulo)" << std::endl;
    std::cout << " [H] - Cisalhamento (shx shy)" << std::endl;
    std::cout << " [M] - Reflexão (eixo 'x' ou 'y')" << std::endl;
    std::cout << "\n--- Outros Controles ---" << std::endl;
    std::cout << " [W] - Salvar estado do Objeto Ativo para '" << g_outputFilename << "'" << std::endl;
    std::cout << " [ESC] - Fechar o programa." << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // RGBA para blending (Xiaolin Wu)
    glutInitWindowSize(g_windowWidth, g_windowHeight);
    glutCreateWindow("Visualizador Half-Edge (Multi-Objeto com Clipping)");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    init();
    glutPostRedisplay();
    glutMainLoop();

    return 0;
}