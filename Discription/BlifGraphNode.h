/*
Used to descripe blif graph node
*/

#pragma once

#include <string>
#include <vector>

using namespace std;

class BlifGraphNode{
public:
    string type;
    string name;
    vector<string> inputs;
    vector<string> outputs;
    vector<string> nextNodes;
    bool isPortFake = false;

    string toString(){
        string result = name + " " + type;
        for(string nextNode : nextNodes)
            result += " " + nextNode;
        return result;
    }
};