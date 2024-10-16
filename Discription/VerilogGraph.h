/*
    VerilogGraph: The graph structur of a verilog
*/
#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

struct Node {
    string id;
    vector<string> neighbors;
    Node() {}
    Node(string _id) : id(_id) {}
};

class VerilogGraph {
public:
    map<string, Node> nodes;

    void addNode(string nodeId) {
        nodes.emplace(nodeId, Node(nodeId));
    }

    void addEdge(string fromNode, string toNode) {
        nodes[fromNode].neighbors.push_back(toNode);
    }
};