/*
    VerilogNode: The node data structure used in action leading and variety evaluation.
*/
#pragma once

#include <string>
#include <vector>

using namespace std;

class VerilogNode{
public:
    string nodeName;
    vector<string> dependency;
    vector<string> inputsDepen;
    bool isInAlways = false;

    VerilogNode(string nodeName) : nodeName(nodeName){}

    VerilogNode(string nodeName, vector<string> dependency, vector<string> inputsDepen) :
    nodeName(nodeName),
    dependency(dependency),
    inputsDepen(inputsDepen)
    {}

    VerilogNode(string nodeName, vector<string> dependency, vector<string> inputsDepen, bool isInAlways) :
    nodeName(nodeName),
    isInAlways(isInAlways),
    dependency(dependency),
    inputsDepen(inputsDepen)
    {}

};