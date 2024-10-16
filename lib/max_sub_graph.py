import networkx as nx
from networkx.algorithms import isomorphism


def graphPreProcess(graph):
    lines = graph.split("\n")
    G = nx.Graph()
    for line in lines:
        items = line.split(" ")
        G.add_node(items[0], type=items[1])
    
    for line in lines:
        for i in range(2, len(items)):
            G.add_edge(items[0], items[i])
    
    return G


def node_match(n1, n2):
    return n1['type'] == n2['type']


def max_sub_graph(graph1, graph2):
    G1 = graphPreProcess(graph1)
    G2 = graphPreProcess(graph2)

    GM = isomorphism.GraphMatcher(G1, G2, node_match=node_match)

    subgraphs = []
    count = 0
    for subgraph in GM.subgraph_isomorphisms_iter():
        subgraphs.append(subgraph)
        count += 1
        if(count >= 2):
            break

    print("Maximum common subgraphs:")
    for sg in subgraphs:
        print(sg)

    if subgraphs:
        max_size = max(len(sg) for sg in subgraphs)
        max_subgraphs = [sg for sg in subgraphs if len(sg) == max_size]
        for sg in max_subgraphs:
            return '\n'.join(f"{k}:{v}" for k, v in sg.items())

    

if __name__ == '__main__':
    G1 = nx.Graph()
    G2 = nx.Graph()

    G1.add_node(1, type='A')
    G1.add_node(2, type='B')
    G1.add_node(3, type='A')
    G1.add_node(4, type='B')
    G1.add_edges_from([(1, 2), (2, 3), (3, 4), (4, 1), (1, 3)])

    G2.add_node(5, type='A', pre_nodes=[1, 2, 3])
    G2.add_node(6, type='B')
    G2.add_node(7, type='A')
    G2.add_node(8, type='B')
    G2.add_edges_from([(5, 6), (6, 7), (7, 8), (8, 5), (5, 7)])

    print(G2.nodes[5]['pre_nodes'])

    def node_match(n1, n2):
        return n1['type'] == n2['type']

    GM = isomorphism.GraphMatcher(G1, G2, node_match=node_match)

    subgraphs = []
    for subgraph in GM.subgraph_isomorphisms_iter():
        subgraphs.append(subgraph)

    print("Maximum common subgraphs:")
    for sg in subgraphs:
        print(sg)

    if subgraphs:
        max_size = max(len(sg) for sg in subgraphs)
        max_subgraphs = [sg for sg in subgraphs if len(sg) == max_size]
        print("Largest common subgraph(s):")
        for sg in max_subgraphs:
            print(sg)
