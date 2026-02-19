from typing import List, Tuple, Dict, Any

# Define um tamanho para a janela de visualização
SIZE = 600

# --------------------------
# Estruturas de Dados
# --------------------------

# Type alias para Coordenadas (x, y)
Point = Tuple[int, int]
# Type alias para Cor (RGB)
Color = Tuple[float, float, float]


class IntersectionPoint:
    """
    Representa um ponto de interseção ou um vértice do polígono,
    junto com metadados relevantes para o algoritmo.
    """

    def __init__(self, p: Point, point_flag: int = 0, index0: int = -1, index1: int = -1, in_flag: bool = False,
                 dis: int = 0):
        # Coordenada do ponto (x, y)
        self.p: Point = p
        # 0: Vértice do Polígono; 1: Ponto de Interseção
        self.point_flag: int = point_flag
        # Índice do segmento do polígono 1 (padrão) onde está a interseção
        self.index0: int = index0
        # Índice do segmento do polígono 2 (recorte) onde está a interseção
        self.index1: int = index1
        # Flag de "Dentro" (True se a próxima aresta for 'dentro')
        self.in_flag: bool = in_flag
        # Distância ao ponto inicial do segmento para ordenação
        self.dis: int = dis

    def __repr__(self) -> str:
        return f"IP(p={self.p}, flag={self.point_flag}, in={self.in_flag})"

    def __eq__(self, other: Any) -> bool:
        if isinstance(other, IntersectionPoint):
            return self.p == other.p
        elif isinstance(other, tuple) and len(other) == 2:
            return self.p == other
        return False

    def __hash__(self) -> int:
        return hash(self.p)


class Polygon:
    """
    Representa um polígono como uma lista de pontos.
    No Python, a lógica de desenho GL é substituída por uma função
    que pode ser usada com Matplotlib ou outra biblioteca gráfica.
    """

    def __init__(self, pts: List[Point] = None):
        self.pts: List[Point] = pts if pts is not None else []

    def draw_pg_line(self, ax, color: Color):
        """
        Desenha o polígono na tela (usando Matplotlib 'ax').
        Substitui a lógica de desenho GL_LINE_LOOP do C++.
        """
        if not self.pts:
            return

        # Cria uma lista de coordenadas X e Y, fechando o loop
        x_coords = [p[0] for p in self.pts]
        y_coords = [p[1] for p in self.pts]

        # Fecha o loop adicionando o primeiro ponto novamente
        x_coords.append(self.pts[0][0])
        y_coords.append(self.pts[0][1])

        ax.plot(x_coords, y_coords, color=color, linewidth=2.0)
        ax.scatter(x_coords[:-1], y_coords[:-1], color=color, s=20)  # Desenha os vértices


# --------------------------
# Funções Geométricas
# --------------------------

def is_point_inside_pg(p: Point, py: Polygon) -> bool:
    """
    Testa se um ponto está dentro do polígono usando o algoritmo
    Ray Casting (lançamento de raio).
    """
    x, y = p
    size = len(py.pts)
    cnt = 0

    for i in range(size):
        p1 = py.pts[i]
        p2 = py.pts[(i + 1) % size]

        x1, y1 = p1
        x2, y2 = p2

        # Segmento horizontal, ignora
        if y1 == y2:
            continue

        # Ponto abaixo do segmento, ignora
        if y < min(y1, y2):
            continue

        # Ponto acima ou na linha superior do segmento, ignora (exclui limite superior)
        # O limite superior max(y1, y2) deve ser incluído, exceto se for um vértice
        if y >= max(y1, y2):
            continue

        # Calcula a interseção do raio horizontal (y constante) com a aresta
        # x = (y - y1) * (x2 - x1) / (y2 - y1) + x1
        x_intersect = (y - y1) * (x2 - x1) / (y2 - y1) + x1

        # Verifica se a interseção está à direita do ponto
        if x_intersect > x:
            cnt += 1

    # O ponto está dentro se o número de interseções for ímpar
    return (cnt % 2 == 1)


def cross_product(p0: Point, p1: Point, p2: Point) -> int:
    """
    Calcula o produto vetorial 2D (P1-P0) x (P2-P0).
    Retorna um valor que indica a orientação:
    > 0: P2 à esquerda de P0->P1
    < 0: P2 à direita de P0->P1
    = 0: P2 é colinear
    """
    x0, y0 = p0
    x1, y1 = p1
    x2, y2 = p2
    return (x2 - x0) * (y1 - y0) - (x1 - x0) * (y2 - y0)


def on_segment(p0: Point, p1: Point, p2: Point) -> bool:
    """
    Verifica se o ponto p2 está no segmento de linha P0P1.
    (Assume colinearidade já verificada ou implícita).
    """
    minx = min(p0[0], p1[0])
    maxx = max(p0[0], p1[0])
    miny = min(p0[1], p1[1])
    maxy = max(p0[1], p1[1])

    return (minx <= p2[0] <= maxx and
            miny <= p2[1] <= maxy)


def segments_intersect(p1: Point, p2: Point, p3: Point, p4: Point) -> bool:
    """
    Verifica se os segmentos P1P2 e P3P4 se intersectam.
    Usa o teste de orientação (produto vetorial).
    """
    d1 = cross_product(p3, p4, p1)
    d2 = cross_product(p3, p4, p2)
    d3 = cross_product(p1, p2, p3)
    d4 = cross_product(p1, p2, p4)

    # Interseção propriamente dita
    if (((d1 > 0 and d2 < 0) or (d1 < 0 and d2 > 0)) and
            ((d3 > 0 and d4 < 0) or (d3 < 0 and d4 > 0))):
        return True

    # Casos de colinearidade e extremidades (ponto no segmento)
    if d1 == 0 and on_segment(p3, p4, p1): return True
    if d2 == 0 and on_segment(p3, p4, p2): return True
    if d3 == 0 and on_segment(p1, p2, p3): return True
    if d4 == 0 and on_segment(p1, p2, p4): return True

    return False


def get_intersect_point(p1: Point, p2: Point, p3: Point, p4: Point) -> Point:
    """
    Calcula as coordenadas exatas do ponto de interseção
    entre as linhas definidas por P1P2 e P3P4.
    """
    x1, y1 = p1
    x2, y2 = p2
    x3, y3 = p3
    x4, y4 = p4

    # Linha 1: A1*x + B1*y = C1  (onde A1 = y2-y1, B1 = x1-x2, C1 = A1*x1 + B1*y1)
    A1 = y2 - y1
    B1 = x1 - x2
    C1 = A1 * x1 + B1 * y1

    # Linha 2: A2*x + B2*y = C2
    A2 = y4 - y3
    B2 = x3 - x4
    C2 = A2 * x3 + B2 * y3

    # Determinante principal
    D = A1 * B2 - A2 * B1

    # Se D for 0, as linhas são paralelas ou colineares.
    # Assumimos que a intersecção foi verificada por segments_intersect.
    # Para lidar com a precisão de ponto flutuante em Python:
    if D == 0:
        # Pega o ponto médio se forem colineares e sobrepostos
        # (simplificação, a lógica original pode não estar totalmente correta para colinearidade)
        return int((x1 + x3) / 2), int((y1 + y3) / 2)

    # Coordenadas de intersecção
    x = (B2 * C1 - B1 * C2) / D
    y = (A1 * C2 - A2 * C1) / D

    # O código C++ usa aritmética de inteiros (D1/D, D2/D).
    # Usamos round() para retornar inteiros como na versão C++.
    return round(x), round(y)


def get_distance(p1: Point, p2: Point) -> int:
    """Calcula a distância euclidiana ao quadrado (para evitar sqrt)"""
    return (p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2


# --------------------------
# Lógica do Weiler-Atherton
# --------------------------

def generate_intersect_points(py_clip: Polygon, py: Polygon) -> List[IntersectionPoint]:
    """
    Encontra todos os pontos de interseção entre as arestas
    do polígono (py) e do polígono de recorte (py_clip).
    """
    iplist: List[IntersectionPoint] = []
    clip_size = len(py_clip.pts)
    py_size = len(py.pts)

    for i in range(clip_size):
        p1 = py_clip.pts[i]
        p2 = py_clip.pts[(i + 1) % clip_size]

        for j in range(py_size):
            p3 = py.pts[j]
            p4 = py.pts[(j + 1) % py_size]

            if segments_intersect(p1, p2, p3, p4):
                ip_coord = get_intersect_point(p1, p2, p3, p4)

                # Adiciona o ponto de interseção à lista
                ip = IntersectionPoint(
                    p=ip_coord,
                    point_flag=1,
                    index0=j,  # Índice do segmento do polígono principal (py)
                    index1=i  # Índice do segmento do polígono de recorte (py_clip)
                )
                iplist.append(ip)

    return iplist


def distance_comparator(ip1: IntersectionPoint, ip2: IntersectionPoint) -> bool:
    """Compara IntersectionPoints pela distância (para ordenação)"""
    return ip1.dis < ip2.dis


def generate_list(py: Polygon, iplist: List[IntersectionPoint], index_selector: int) -> List[IntersectionPoint]:
    """
    Cria uma lista combinada de vértices e interseções ordenadas por segmento
    para um polígono (py ou py_clip).
    - index_selector=0: Polígono principal (usa index0 para filtrar)
    - index_selector=1: Polígono de recorte (usa index1 para filtrar)
    """
    comlist: List[IntersectionPoint] = []
    size = len(py.pts)

    for i in range(size):
        p1 = py.pts[i]

        # Adiciona o vértice do polígono (point_flag=0)
        comlist.append(IntersectionPoint(p=p1, point_flag=0))

        one_seg: List[IntersectionPoint] = []

        # Coleta interseções para o segmento i (p1 -> p_next)
        for ip in iplist:
            is_on_segment = False
            if index_selector == 0 and i == ip.index0:
                is_on_segment = True
            elif index_selector == 1 and i == ip.index1:
                is_on_segment = True

            if is_on_segment:
                # Calcula a distância de p1 ao ponto de interseção
                ip.dis = get_distance(ip.p, p1)
                ip.point_flag = 1  # Garante que está marcado como interseção
                one_seg.append(ip)

        # Ordena as interseções pela distância a p1
        one_seg.sort(key=lambda ip: ip.dis)

        # Adiciona as interseções ordenadas entre os vértices
        comlist.extend(one_seg)

    return comlist


def get_pg_point_in_out(pg_list: List[IntersectionPoint], py_clip: Polygon):
    """
    Marca os pontos de interseção no polígono principal (Pglist)
    com a flag 'in_flag' (dentro/fora) e propaga o estado 'dentro'
    para os vértices.
    """
    in_flag = False

    for ip in pg_list:
        if ip.point_flag == 0:  # Vértice
            # Determina o estado inicial do vértice
            in_flag = is_point_inside_pg(ip.p, py_clip)
            ip.in_flag = in_flag  # Marca o vértice (não estritamente necessário para o loop principal)
        else:  # Interseção
            # Inverte o estado ao passar por uma interseção
            in_flag = not in_flag
            ip.in_flag = in_flag


def get_clip_point_in_out(clip_list: List[IntersectionPoint], pg_list: List[IntersectionPoint]):
    """
    Transfere o estado 'in_flag' dos pontos de interseção
    de Pglist para Cliplist (eles devem ser os mesmos pontos).
    """
    # Cria um mapa de coordenadas de interseção (coordenada -> in_flag)
    intersections: Dict[Point, bool] = {}
    for ip in pg_list:
        if ip.point_flag == 1:
            intersections[ip.p] = ip.in_flag

    # Atualiza a clip_list
    for ip in clip_list:
        if ip.point_flag == 1:
            if ip.p in intersections:
                ip.in_flag = intersections[ip.p]


def find_first_entry_point(pg_list: List[IntersectionPoint]) -> int:
    """Encontra o índice do primeiro ponto de interseção onde in_flag é True (Entrada)."""
    for i, ip in enumerate(pg_list):
        if ip.point_flag == 1 and ip.in_flag:
            return i
    return -1  # Não encontrou ponto de entrada


def generate_clip_area(pg_list: List[IntersectionPoint], clip_list: List[IntersectionPoint], py_clip: Polygon,py: Polygon) -> List[Polygon]:
    clipped_polygons: List[Polygon] = []

    # Mapeia Interseção -> Índice na clip_list (para troca)
    clip_map: Dict[Point, int] = {ip.p: i for i, ip in enumerate(clip_list) if ip.point_flag == 1}
    # Mapeia Interseção -> Índice na pg_list (para troca)
    pg_map: Dict[Point, int] = {ip.p: i for i, ip in enumerate(pg_list) if ip.point_flag == 1}

    # Rastreia interseções visitadas para garantir que cada polígono é traçado uma única vez
    visited_intersections = {ip.p: False for ip in pg_list if ip.point_flag == 1 and ip.in_flag}

    current_index = 0

    while True:
        # Encontra o próximo ponto de partida (Entrada não visitado)
        start_index = -1
        # Busca em pg_list pelo próximo ponto de Entrada (in_flag=True) não visitado
        for i in range(len(pg_list)):
            ip = pg_list[i]
            if ip.point_flag == 1 and ip.in_flag and not visited_intersections[ip.p]:
                start_index = i
                break

        if start_index == -1:
            break  # Não há mais pontos de entrada não visitados. Fim do algoritmo de rastreamento.

        # --- Inicia um novo polígono ---
        current_polygon = Polygon()

        current_list = pg_list
        current_index = start_index
        is_on_pg_list = True

        start_point_coord = current_list[current_index].p  # Coordenada do ponto de partida

        # O ponto de partida é sempre o primeiro ponto adicionado
        current_polygon.pts.append(start_point_coord)
        visited_intersections[start_point_coord] = True

        while True:
            # Move para o próximo ponto na lista atual
            current_index = (current_index + 1) % len(current_list)
            current_ip = current_list[current_index]

            # 1. Verifica se retornamos ao ponto inicial
            if current_ip.p == start_point_coord:
                # Polígono fechado!
                clipped_polygons.append(current_polygon)
                break

                # 2. Verifica se é um ponto de troca (SAÍDA)
            if current_ip.point_flag == 1:
                # Condição de SAÍDA:
                # - Se estiver no Pglist (is_on_pg_list=True) e for um ponto de SAÍDA (!ip.in_flag)
                # - OU
                # - Se estiver no Cliplist (is_on_pg_list=False) e for um ponto de SAÍDA (ip.in_flag)
                is_turn_point = (is_on_pg_list and not current_ip.in_flag) or \
                                (not is_on_pg_list and current_ip.in_flag)

                if is_turn_point:
                    # Este é o ponto de SAÍDA (último ponto antes da troca)
                    current_polygon.pts.append(current_ip.p)
                    visited_intersections[current_ip.p] = True

                    # Troca de lista:
                    is_on_pg_list = not is_on_pg_list
                    current_list = clip_list if not is_on_pg_list else pg_list

                    # Atualiza o índice para a posição do ponto de troca na *nova* lista
                    map_to_use = clip_map if not is_on_pg_list else pg_map
                    current_index = map_to_use.get(current_ip.p, -1)

                    if current_index == -1:
                        # Falha de navegação (não deveria acontecer se os mapas estiverem corretos)
                        print(f"Erro de navegação: Ponto {current_ip.p} não encontrado na lista de destino.")
                        return clipped_polygons

                        # O loop continua a partir do ponto atual (que será o ponto de entrada da nova lista)
                    continue

                    # 3. Adiciona o ponto (vértice ou interseção de Entrada) e continua na mesma lista
            current_polygon.pts.append(current_ip.p)

        # Se quebrou o loop interno, o polígono foi salvo, e o loop externo
        # irá buscar o próximo ponto de partida não visitado.

    # 4. Caso de polígono totalmente DENTRO do recorte (sem interseções)
    # Verifica o primeiro vértice do polígono principal (py)
    if pg_list and is_point_inside_pg(pg_list[0].p, py_clip):
        # Se o polígono principal estiver totalmente dentro do polígono de recorte,
        # ele é o resultado.
        if not clipped_polygons:  # Verifica se nenhum polígono já foi gerado
            # O resultado deve ser o polígono principal
            return [Polygon(pts=py.pts)]

    return clipped_polygons


# OBS: Você precisa garantir que a definição da classe Polygon e das funções
# is_point_inside_pg, clip_map, pg_map, find_first_entry_point, e os outros
# elementos auxiliares estão disponíveis e corretos no seu script.

# OBS: A implementação de 'generateClipArea' é complexa de portar
# devido à navegação por listas duplamente ligadas e iteração no C++.
# A versão acima é uma tentativa de replicação da lógica de 'rastreamento'
# com listas Python. O código C++ usa `list` (que é lista duplamente ligada) e iteradores.
# Uma implementação mais robusta em Python usaria uma estrutura de nó/lista ligada.

def weiler_atherton(py_clip: Polygon, py: Polygon) -> List[Polygon]:
    """
    Função principal do Algoritmo de Weiler-Atherton.
    """
    # 1. Gera pontos de interseção
    iplist = generate_intersect_points(py_clip, py)

    # 2. Gera listas combinadas (vértices + interseções ordenadas)
    pg_list = generate_list(py, iplist, 0)
    clip_list = generate_list(py_clip, iplist, 1)

    # 3. Marca os estados 'Dentro/Fora' para Pglist
    get_pg_point_in_out(pg_list, py_clip)

    # 4. Transfere os estados 'Dentro/Fora' para Cliplist
    get_clip_point_in_out(clip_list, pg_list)

    # 5. Gera as áreas recortadas rastreando as listas
    clipped_polygons = generate_clip_area(pg_list, clip_list, py_clip, py)

    return clipped_polygons


# --------------------------
# Simulação (Substitui GLUT)
# --------------------------

# Para a simulação, precisamos da biblioteca matplotlib
try:
    import matplotlib.pyplot as plt
    import numpy as np
except ImportError:
    print("Por favor, instale 'matplotlib' para executar a visualização:")
    print("pip install matplotlib")
    plt = None
    np = None


def display_simulation():
    """
    Substitui a função display() do C++ e a chamada glutMainLoop().
    """
    if plt is None:
        return

    # 1. Definição dos Polígonos (dados do código C++)
    py_clip = Polygon()
    py_clip.pts = [(553, 495), (351, 175), (486, 71), (61, 86)]

    py = Polygon()
    py.pts = [(390, 424), (579, 585), (257, 50), (68, 245)]

    # Imprime os pontos (como no C++)
    print("Polígono de Recorte (py_clip):")
    for p in py_clip.pts:
        print(f"{p[0]} {p[1]}")
    print("\nPolígono Principal (py):")
    for p in py.pts:
        print(f"{p[0]} {p[1]}")
    print("---")

    # 2. Inicialização da Figura Matplotlib
    fig, ax = plt.subplots(figsize=(6, 6))
    ax.set_xlim(0, SIZE - 1)
    ax.set_ylim(0, SIZE - 1)
    ax.set_aspect('equal', adjustable='box')
    ax.set_title("Algoritmo de Recorte de Weiler-Atherton")
    ax.grid(True, linestyle='--', alpha=0.5)

    # 3. Desenho dos Polígonos Originais
    color_py = (1.0, 0.0, 0.0)  # Vermelho
    color_clip = (0.0, 1.0, 0.0)  # Verde

    py.draw_pg_line(ax, color_py)
    py_clip.draw_pg_line(ax, color_clip)

    # 4. Aplica o Recorte
    print("Iniciando Weiler-Atherton...")
    clipped_polygons = weiler_atherton(py_clip, py)
    print(f"Recorte concluído. {len(clipped_polygons)} polígono(s) resultante(s).")

    # 5. Desenho dos Polígonos Recortados
    color_result = (0.0, 0.0, 1.0)  # Azul
    for i, clipped_pg in enumerate(clipped_polygons):
        clipped_pg.draw_pg_line(ax, color_result)

        # Preenche a área recortada (opcional)
        x_coords = [p[0] for p in clipped_pg.pts]
        y_coords = [p[1] for p in clipped_pg.pts]
        ax.fill(x_coords, y_coords, color=color_result, alpha=0.3)
        print(f"Polígono Recortado {i + 1}: {clipped_pg.pts}")

    plt.show()


# --------------------------
# Ponto de Entrada Principal
# --------------------------

def main():
    """
    Substitui a função main() do C++.
    """
    # Não precisamos de srand(time(NULL)) se não usarmos GenerateRandomSimplePg
    # Se fosse necessário, seria: random.seed(time.time())

    print("Iniciando Simulação do Algoritmo de Weiler-Atherton...")
    display_simulation()


if __name__ == "__main__":
    main()