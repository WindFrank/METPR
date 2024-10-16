/*
NetAnalyzer: Analyze and get the datastructure of net.xml
*/

#pragma once

#include "../Discription/BlockNode.h"

#include <iostream>
#include <queue>
#include <string>

using namespace std;

class NetAnalyzer{
private:
    bool isLoad = false;
    BlockNode* outBlock;
    Arch* arch;
    TiXmlElement* file;
    
public:
    NetAnalyzer();
    NetAnalyzer(string module, Arch* arch);
    BlockNode* netConfig();
    BlockNode* netConfig(string filename, Arch* arch);
    bool constructNode(BlockNode* root, string instance, string mode);
};