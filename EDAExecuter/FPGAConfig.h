/*
    FPGAConfig: Build FPGA data structure.
*/

#pragma once

#include "NetAnalyzer.h"
#include "PlaceAnalyzer.h"
#include "RouteAnalyzer.h"

class Channel{
public:
    int track;
    vector<string> routeIDlist;
};

class FPGAConfig{
private:
    Arch* arch;
    bool isLoad = false;
    string module;
    BlockNode* fpgaOutBlock;
    vector<vector<BlockNode*>> clbLayout;
    vector<vector<vector<BlockNode*>>> ioLayout;
    vector<vector<vector<Channel*>>> channelLayout;
    vector<RouteInfo*> vRoute;
    vector<int> rrNodeUsed;
    NetAnalyzer* na;
    PlaceAnalyzer* pa;
    RouteAnalyzer* ra;
    pair<int, int> size;
public:
    FPGAConfig();
    FPGAConfig(string module, string archFileName, string rr_graph_filename);
    void buildFPGA();
    bool loadModule(string module);
    BlockNode* loadNet(string filename);
    bool loadPlace(string filename);
    bool loadRoute(string filename);
    bool infoSave();
    bool channelAdd(pair<int, int> location, int track, string routeID);
    vector<vector<BlockNode*>> getClbLayout();
    vector<vector<vector<BlockNode*>>> getIoLayout();
    vector<vector<vector<Channel*>>> getChannelLayout();
    vector<RouteInfo*> getVRoute();
    BlockNode* getFpgaOutBlock();
    Arch* getArch();
    BlockNode* findTargetBlockNode(pair<int, int> loc, string ioName="");
    pair<int, int> getSize();
    vector<int> getRRNodeUsed();
};