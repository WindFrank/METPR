import networkx as nx


def GED(elements1, elements2, t):
    G = nx.DiGraph()
    for e in elements1:
        if "NEXT" in e:
            nodes = e.split("NEXT")
            G.add_edge(nodes[0], nodes[1])
        else:
            G.add_node(e)
    G2 = nx.DiGraph()
    for e in elements2:
        if "NEXT" in e:
            nodes = e.split("NEXT")
            G2.add_edge(nodes[0], nodes[1])
        else:
            G2.add_node(e)
    print(elements1)
    print(elements2)
    return nx.graph_edit_distance(G, G2, timeout=t)


if __name__ == '__main__':
    print(GED(("1", "1NEXT2", "2", "2NEXT3", "3"), ("1", "1NEXT3", "2", "2NEXT3", "3")))