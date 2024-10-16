/*
Arch: Read and save information from xml file
*/
#pragma once

#include "Discription/GDelay.h"
#include "Discription/SwitchInfo.h"
#include "Discription/TileNode.h"
#include "Discription/TileEdge.h"
// #include "lib/rapidXML/rapidxml_ns.hpp"
// #include "lib/rapidXML/rapidxml_ns_print.hpp"
// #include "lib/rapidXML/rapidxml_ns_utils.hpp"
#include "lib/tinyXML/tinyxml.h"

#include <iostream>
#include <string>
#include <vector>
#include <climits>
#include <map>

using namespace std;
// using namespace rapidxml_ns;

class Arch{
private:
    string archName = "";
    string archPath = "";
    TiXmlElement* root;
    vector<GDelay> delayV;
    string allElements[16] = {"architecture", "complexblocklist", "delay_constant",
                            "delay_matrix", "complete", "input", "output"
                            , "clock", "T_setup", "T_clock_to_Q", "pb_type",
                            "interconnect", "mode", "direct", "pack_pattern", "mux"};
    TiXmlElement* rr_graph_root;
    map<int, SwitchInfo> finalSwitches;
    map<int, TileNode> tileNodes;
    map<int, vector<TileEdge>> tileEdges;
    int ioNum = -1;
public:
    Arch();
    Arch(string archName, string filename);
    Arch(string archName, string filename, string rr_graph_filename);
    bool archSet(string archName, string filename);
    bool rrGraphSet(string rr_graph_filename);
    bool isSet();
    double getDelay(string preType, string preMode, string input, string output);
    bool isPortMatch(string fpgaPin, string connPin);
    static void isolateNameandIndex(string nameandPin, string* name, int* minIndex, int* maxIndex);
    double getNodeDelay(TiXmlElement* xmlNode);
    string getName();
    bool checkType(string s);
    map<int, SwitchInfo> getSwitchInfo();
    TileNode getTileNode(int id);
    vector<TileEdge> getTileEdges(int sourceID);
    vector<int> getRRUsed();
    string getArchPath();
    int getIONum();
};