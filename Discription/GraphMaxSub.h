// Data structure used to max sub graph search

#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

using namespace std;

class GraphMaxSub {
public:
    unordered_map<string, vector<string>> adj;
    unordered_map<string, string> type;        

    void addNode(const string& nodeName, const string& nodeType) {
        if(type.find(nodeName) == type.end()){
            adj[nodeName];
            type[nodeName] = nodeType; 
        }
        else if(type[nodeName] == "")
            type[nodeName] = nodeType;
    }


    void addEdge(const string& from, const string& to) {
        adj[from].push_back(to);
    }


    bool canMatch(const string& node1, const string& node2, const GraphMaxSub& other) const {
        return type.at(node1) == other.type.at(node2);
    }
};

