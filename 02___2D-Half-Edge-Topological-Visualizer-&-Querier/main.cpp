
/*
 * =========================================================================================
 * DESCRIÇÃO DO PROGRAMA
 * =========================================================================================
 *
 * Este programa utiliza OpenGL e a estrutura de dados Half-Edge (Aresta-Metade) para
 * carregar, visualizar e consultar a topologia de um objeto gráfico 2D a partir de um
 * arquivo no formato .obj.
 *
 * PRINCIPAIS FUNCIONALIDADES:
 *
 * 1. Carregamento e Estrutura de Dados:
 * - Lê as coordenadas de vértices e a definição de faces de um arquivo .obj.
 * - Constrói uma representação interna completa do objeto usando a estrutura Half-Edge,
 * que mapeia as relações entre vértices, arestas (half-edges) e faces.
 *
 * 2. Renderização Gráfica:
 * - Utiliza a biblioteca GLUT para criar uma janela e gerenciar eventos.
 * - As arestas do objeto são desenhadas usando apenas a primitiva de ponto (GL_POINTS)
 * e a equação da reta, conforme o requisito original.
 *
 * 3. Visualização Interativa de Dados:
 * - Permite exibir/ocultar os IDs dos vértices, arestas e faces diretamente sobre o objeto,
 * facilitando a identificação de cada componente.
 * - Oferece um modo de visualização que demonstra a direção de cada half-edge com setas
 * coloridas, facilitando o entendimento da estrutura cíclica das faces.
 *
 * 4. Sistema de Consultas Topológicas:
 * - O usuário pode realizar consultas complexas na estrutura de dados em tempo real.
 * - As consultas são ativadas por teclas na janela gráfica, com entrada de dados e
 * resultados exibidos no terminal (console).
 * - Consultas implementadas:
 * - Dada uma face, listar suas faces adjacentes.
 * - Dada uma aresta, listar suas faces adjacentes.
 * - Dado um vértice, listar as faces que o compartilham.
 * - Dado um vértice, listar as arestas (half-edges) que o compartilham como origem.
 *
 * CONTROLES (pressionados na janela gráfica):
 * - Teclas 'v', 'e', 'f': Ativam/desativam a visualização dos IDs de Vértices, Arestas e Faces.
 * - Tecla 'h': Ativa/desativa o modo de visualização de setas Half-Edge.
 * - Teclas '1'-'4': Iniciam os respectivos prompts de consulta no terminal.
 * - Tecla 'ESC': Encerra o programa.
 *
 * 
 * COMANDOS PARA RODAR O PROGRAMA:
 * g++ main.cpp -o visualizador -lfreeglut -lopengl32 -lglu32 -lpthread
 * ./visualizador.exe objeto.obj
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

// --- Estruturas de Dados (Half-Edge) ---
struct HalfEdge;
struct Vertex;
struct Face;

struct Vertex {
    float x, y;
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

// Flags para controlar a exibição dos IDs
bool showVertexIDs = true;
bool showEdgeIDs = true;
bool showFaceIDs = true;

// Função para desenhar texto no OpenGL
void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

// --- Função de Renderização ---
void drawSegmentByLineEquation(float x1, float y1, float x2, float y2) {
    float dx = (x2 - x1);
    float dy = (y2 - y1);
    if (std::abs(dx) < 1e-6) {
        float startY = std::min(y1, y2);
        float endY = std::max(y1, y2);
        glBegin(GL_POINTS);
        for (float y = startY; y <= endY; y += 0.001f) { glVertex2f(x1, y); }
        glEnd();
        return;
    }
    float m = dy / dx;
    float b = y1 - m * x1;
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    if (std::abs(dx) > std::abs(dy)) {
        float startX = std::min(x1, x2);
        float endX = std::max(x1, x2);
        for (float x = startX; x <= endX; x += 0.001f) { float y = m * x + b; glVertex2f(x, y); }
    } else {
        float startY = std::min(y1, y2);
        float endY = std::max(y1, y2);
        for (float y = startY; y <= endY; y += 0.001f) { float x = (y - b) / m; glVertex2f(x, y); }
    }
    glEnd();
}

// --- Funções de Consulta (sem alterações) ---
void queryAdjacentFaces_Face(int faceId) {
    if (faceId < 0 || faceId >= g_faces.size()) { std::cout << "  > Face com ID " << faceId << " invalida." << std::endl; return; }
    std::cout << "  > Faces adjacentes a Face " << faceId << ":" << std::endl;
    Face* face = g_faces[faceId];
    HalfEdge* startEdge = face->edge;
    HalfEdge* currentEdge = startEdge;
    do {
        if (currentEdge->twin && currentEdge->twin->face) { std::cout << "    - Face " << currentEdge->twin->face->id << std::endl; }
        currentEdge = currentEdge->next;
    } while (currentEdge != startEdge);
}
void queryAdjacentFaces_Edge(int edgeId) {
    if (edgeId < 0 || edgeId >= g_halfEdges.size()) { std::cout << "  > Aresta com ID " << edgeId << " invalida." << std::endl; return; }
    std::cout << "  > Faces adjacentes a Aresta " << edgeId << " (e sua gemea):" << std::endl;
    HalfEdge* he = g_halfEdges[edgeId];
    if (he->face) { std::cout << "    - Face " << he->face->id << std::endl; }
    if (he->twin && he->twin->face) { std::cout << "    - Face " << he->twin->face->id << std::endl; }
}

// Função auxiliar para encontrar a aresta anterior em um ciclo de face

// Função auxiliar para encontrar a aresta anterior em um ciclo de face
HalfEdge* findPreviousEdge(HalfEdge* targetEdge) {
    if (!targetEdge || !targetEdge->face) return nullptr;
    HalfEdge* current = targetEdge;
    // Prevenção de loop infinito em caso de malha malformada.
    for (int i = 0; i < g_vertices.size() + 1 && current->next != targetEdge; ++i) {
        current = current->next;
    }
    return current;
}

void queryFacesSharingVertex(int vertexId) {
    if (vertexId < 0 || vertexId >= g_vertices.size()) {
        std::cout << "  > Vertice com ID " << vertexId << " invalido." << std::endl;
        return;
    }
    std::cout << "  > Faces que compartilham o Vertice " << vertexId << ":" << std::endl;

    Vertex* vertex = g_vertices[vertexId];
    HalfEdge* startEdge = vertex->leaving;
    if (!startEdge) {
        std::cout << "    (Vertice isolado, sem faces)" << std::endl;
        return;
    }

    std::set<int> foundFaces;

    // Travessia 1: Sentido anti-horário (CCW)
    HalfEdge* current = startEdge;
    while (true) {
        if (current->face) {
            foundFaces.insert(current->face->id);
        }
        if (!current->twin) break; // Atingiu uma borda
        current = current->twin->next;
        if (current == startEdge) break; // Completou o ciclo
    }

    // Travessia 2: Sentido horário (CW) a partir da aresta inicial
    current = startEdge;
    while (true) {
        HalfEdge* prev = findPreviousEdge(current);
        if (!prev || !prev->twin) break; // Atingiu uma borda
        current = prev->twin;
        if (current->face) {
            foundFaces.insert(current->face->id);
        }
        if (current == startEdge) break; // Evita re-percorrer
    }

    if (foundFaces.empty()) {
        std::cout << "    (Nenhuma face encontrada para este vertice)" << std::endl;
    } else {
        for (int faceId : foundFaces) {
            std::cout << "    - Face " << faceId << std::endl;
        }
    }
}

void queryEdgesSharingVertex(int vertexId) {
    if (vertexId < 0 || vertexId >= g_vertices.size()) {
        std::cout << "  > Vertice com ID " << vertexId << " invalido." << std::endl;
        return;
    }
    std::cout << "  > Arestas que compartilham o Vertice " << vertexId << ":" << std::endl;

    Vertex* targetVertex = g_vertices[vertexId];
    if (!targetVertex) return;

    // Itera por TODAS as half-edges e verifica a origem de cada uma.
    
    std::vector<HalfEdge*> foundEdges;
    for (HalfEdge* edge : g_halfEdges) {
        if (edge->origin == targetVertex) {
            foundEdges.push_back(edge);
        }
    }

    if (foundEdges.empty()) {
        std::cout << "    (Nenhuma aresta encontrada para este vertice)" << std::endl;
    } else {
        for (HalfEdge* edge : foundEdges) {
            if (edge->twin && edge->twin->origin) {
                std::cout << "    - Aresta " << edge->id << " (origem: " << edge->origin->id << ", destino: " << edge->twin->origin->id << ")" << std::endl;
            } else {
                std::cout << "    - Aresta " << edge->id << " (aresta de borda, sem gemea)" << std::endl;
            }
        }
    }
}

// --- Lógica de Construção e Parser (sem alterações) ---
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
            Vertex* v = new Vertex(); ss >> v->x >> v->y; v->id = vertexIdCounter++; g_vertices.push_back(v);
        } else if (prefix == "f") {
            std::vector<int> indices; int index;
            while (ss >> index) { indices.push_back(index - 1); }
            face_indices.push_back(indices);
        }
    }
    buildHalfEdgeStructure(face_indices);
    return true;
}

// --- Funções do OpenGL ---
void init() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. Desenha o objeto
    glColor3f(0.8f, 0.8f, 1.0f); // Cor do objeto
    for(const auto& edge : g_halfEdges) {
        if (edge->twin == nullptr || edge->id < edge->twin->id) {
            if (edge->origin && edge->next && edge->next->origin) {
                Vertex* v1 = edge->origin;
                Vertex* v2 = edge->next->origin;
                drawSegmentByLineEquation(v1->x, v1->y, v2->x, v2->y);
            }
        }
    }

    //  Desenha os IDs sobre o objeto
    // 2. Desenha os IDs das Faces (se habilitado)
    if (showFaceIDs) {
        glColor3f(0.0f, 1.0f, 0.0f); // Verde para Faces
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
                drawText(cX / numVertices, cY / numVertices, "F" + std::to_string(face->id));
            }
        }
    }

    // 3. Desenha os IDs das Arestas (se habilitado)
    if (showEdgeIDs) {
        glColor3f(1.0f, 1.0f, 0.0f); // Amarelo para Arestas
        for (const auto& edge : g_halfEdges) {
            if (edge->twin == nullptr || edge->id < edge->twin->id) {
                if (edge->origin && edge->next && edge->next->origin) {
                    Vertex* v1 = edge->origin;
                    Vertex* v2 = edge->next->origin;
                    float mX = (v1->x + v2->x) / 2.0f;
                    float mY = (v1->y + v2->y) / 2.0f;
                    drawText(mX, mY, "E" + std::to_string(edge->id));
                }
            }
        }
    }
    
    // 4. Desenha os IDs dos Vértices (se habilitado)
    if (showVertexIDs) {
        glColor3f(1.0f, 0.5f, 0.0f); // Laranja para Vértices
        for (const auto& vertex : g_vertices) {
            drawText(vertex->x + 0.02f, vertex->y, "V" + std::to_string(vertex->id));
        }
    }

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    int id;
    switch (key) {
        case 27: exit(0); break;

        // Teclas para Ligar/Desligar a exibição dos IDs
        case 'f':
        case 'F':
            showFaceIDs = !showFaceIDs;
            std::cout << " > Exibicao de IDs de Face: " << (showFaceIDs ? "ON" : "OFF") << std::endl;
            break;
        case 'e':
        case 'E':
            showEdgeIDs = !showEdgeIDs;
            std::cout << " > Exibicao de IDs de Aresta: " << (showEdgeIDs ? "ON" : "OFF") << std::endl;
            break;
        case 'v':
        case 'V':
            showVertexIDs = !showVertexIDs;
            std::cout << " > Exibicao de IDs de Vertice: " << (showVertexIDs ? "ON" : "OFF") << std::endl;
            break;

        // Seção de consultas (mantida como antes)
        case '1':
            std::cout << "\n[CONSULTA] Digite o ID da FACE para ver adjacentes: ";
            std::cin >> id; queryAdjacentFaces_Face(id); break;
        case '2':
            std::cout << "\n[CONSULTA] Digite o ID da ARESTA para ver faces adjacentes: ";
            std::cin >> id; queryAdjacentFaces_Edge(id); break;
        case '3':
            std::cout << "\n[CONSULTA] Digite o ID do VÉRTICE para ver faces que o compartilham: ";
            std::cin >> id; queryFacesSharingVertex(id); break;
        case '4':
            std::cout << "\n[CONSULTA] Digite o ID do VÉRTICE para ver arestas que o compartilham: ";
            std::cin >> id; queryEdgesSharingVertex(id); break;
    }
    // Força o redesenho da tela após qualquer tecla ser pressionada
    glutPostRedisplay();
}


int main(int argc, char** argv) {
  if (argc < 2) { std::cerr << "Uso: " << argv[0] << " <caminho_para_o_arquivo.obj>" << std::endl; return 1; }
  if (!loadOBJ(argv[1])) { return 1; }
  
  std::cout << "Arquivo OBJ carregado e estrutura Half-Edge construida com sucesso." << std::endl;
  std::cout << "Total de Vertices: " << g_vertices.size() << std::endl;
  std::cout << "Total de Faces: " << g_faces.size() << std::endl;
  std::cout << "Total de Half-Edges: " << g_halfEdges.size() << std::endl;
  
  // Menu
  std::cout << "\n--- Janela Grafica Iniciada ---" << std::endl;
  std::cout << "\n--- Pressione as teclas na JANELA GRAFICA para interagir ---" << std::endl;
  std::cout << "--- Controles de Exibicao ---" << std::endl;
  std::cout << " [V] - Liga/Desliga IDs dos Vertices" << std::endl;
  std::cout << " [E] - Liga/Desliga IDs das Arestas" << std::endl;
  std::cout << " [F] - Liga/Desliga IDs das Faces" << std::endl;
  std::cout << "\n--- Teclas de Consulta (resultados aparecem no terminal) ---" << std::endl;
  std::cout << " [1] - Dado uma face, listar as faces adjacentes." << std::endl;
  std::cout << " [2] - Dada uma aresta, listar as faces adjacentes." << std::endl;
  std::cout << " [3] - Dado um vertice, listar quais faces compartilham o vertice." << std::endl;
  std::cout << " [4] - Dado um vertice, listar quais arestas compartilham o vertice." << std::endl;
  std::cout << " [ESC] - Fechar o programa." << std::endl;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(600, 600);
  glutCreateWindow("Visualizador Half-Edge (com IDs)");
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  init();
  glutPostRedisplay();
  glutMainLoop();

  return 0;
}