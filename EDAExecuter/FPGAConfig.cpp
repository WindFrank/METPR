#include "FPGAConfig.h"
#include "../Tool.h"

/*
    FPGAConfig(string): directly configure net on FPGA. Save the basic info of FPGA.
    module: the name of module. You need to place the .net.post_routing, .place, .route into the same content.
    arch: the target architecture.
*/
FPGAConfig:: FPGAConfig(){}

FPGAConfig::FPGAConfig(string module, string archFileName, string rr_graph_filename){
    this->module = module;
    this->arch = new Arch(archFileName.substr(0, archFileName.find(".")), archFileName, rr_graph_filename);
    FPGAConfig();
}

void FPGAConfig::buildFPGA(){
    if(module == "" || !arch->isSet())
        Tool::error("Error: FPGAConfig unset.");
    fpgaOutBlock = loadNet(module + ".net.post_routing");
    loadPlace(module + ".place");
    loadRoute(module + ".route");
    infoSave();
}

bool FPGAConfig::loadModule(string module){
    this->module = module;
    return true;
}

BlockNode* FPGAConfig::loadNet(string filename)
{
    na = new NetAnalyzer(filename, arch);
    return na->netConfig(filename, arch);
}

bool FPGAConfig::loadPlace(string filename){
    pa = new PlaceAnalyzer(filename);
    pa->getSize();
    pa->setPlaceInfo();
    return true;
}

bool FPGAConfig::loadRoute(string filename)
{
    ra = new RouteAnalyzer();
    // ra->getSwitchInfo(arch);
    ra->loadRoute(filename);
    return true;
}

bool FPGAConfig::infoSave()
{
    //PlaceInfo Inject
    size = pa->getSize();    // clb and channel layout initial
    for(int i = 0; i < size.first; i++){
        vector<BlockNode*> vb;
        vector<vector<Channel*>> cb;
        vector<vector<BlockNode*>> vvb;
        for(int j = 0; j < size.second; j++){
            BlockNode* emptyBlock = new BlockNode();
            vb.push_back(emptyBlock);
            vector<Channel*> channels;
            cb.push_back(channels);
            vector<BlockNode*> ioblocks;
            vvb.push_back(ioblocks);
        }
        clbLayout.push_back(vb);
        channelLayout.push_back(cb);
        ioLayout.push_back(vvb);
    }
    vector<BlockNode*> globalBlocks = fpgaOutBlock->getSubBlocks(); // layout block location load
    for(auto it = globalBlocks.begin(); it < globalBlocks.end(); it++){
        BlockNode* block = *it;
        string blockName = block->getName();
        PlaceInfo pi= pa->getPlaceInfo(blockName);
        int x = pi.x;
        int y = pi.y;
        if(x == 0 || y == 0 || x == size.first - 1 || y == size.second - 1){
            block->setSubblk(pi.subblk);
            ioLayout[x][y].push_back(block);
        }
        else
            clbLayout[x][y] = block;
    }

    //RouteInfo Inject
    rrNodeUsed = ra->getRoutesRRUsed();
    vRoute = ra->getRouteList();
    for(auto it = vRoute.begin(); it < vRoute.end(); it++){
        RouteInfo* ri = *it;
        pair<int, int> pathStart = ri->sourceLoc;
        string sinkPort = ri->sinkPort;
        pair<int, int> pathEnd = ri->sinkLoc;

        //Get end BlockName
        string endIOName = ri->netName;
        BlockNode* endBlock = findTargetBlockNode(pathEnd, endIOName);
        endBlock->setPtc(ri->sinkPtcPad);
        string endBlockName = endBlock->getName();

        //Search corresponding cnode
        string targetIOName = ri->netName;
        BlockNode* targetBlock = findTargetBlockNode(pathStart, targetIOName);
        targetBlock->setPtc(ri->sourcePtcPad);
        ConnectNode* cnode = targetBlock->getCNode(sinkPort, "global", endBlockName);
        double cnodeDelay = ra->getRouteDelay(ri, arch);
        cnode->setDelay(cnodeDelay);
        //Channel source inject
        for(auto item = ri->segments.begin(); item < ri->segments.end(); item++){
            Segment* segment = *item;
            if(segment->end.first == 0 && segment->end.second == 0){
                pair<int, int> targetLoc = segment->start;
                channelAdd(targetLoc, segment->track, ri->id);
            }
            else{
                pair<int, int> start = segment->start;
                pair<int, int> end = segment->end;
                if(segment->segmentType == "CHANX"){
                    for(int i = start.first; i <= end.first; i++){
                        pair<int, int> location(i, start.second);
                        channelAdd(location, segment->track, ri->id);
                    }
                }
                else{   //CHANY
                    for(int i = start.second; i <= end.second; i++){
                        pair<int, int> location(start.first, i);
                        channelAdd(location, segment->track, ri->id);
                    }
                }
            }
        }
    }
    return true;
}

bool FPGAConfig::channelAdd(pair<int, int> location, int track, string routeID)
{
    vector<Channel*>* recentPassBlock = &channelLayout[location.first][location.second];
    for(auto it = recentPassBlock->begin(); it < recentPassBlock->end(); it++){
        Channel* recentC = *it;
        if(recentC->track == track){
            recentC->routeIDlist.push_back(routeID);
        }
        if(it + 1 == recentPassBlock->end()){
            Channel newChannel;
            newChannel.track = track;
            newChannel.routeIDlist.push_back(routeID);
            recentPassBlock->push_back(&newChannel);
        }
    }
    return true;
}

vector<vector<BlockNode *>> FPGAConfig::getClbLayout()
{
    return clbLayout;
}

vector<vector<vector<BlockNode *>>> FPGAConfig::getIoLayout()
{
    return ioLayout;
}

vector<vector<vector<Channel *>>> FPGAConfig::getChannelLayout()
{
    return channelLayout;
}

vector<RouteInfo*> FPGAConfig::getVRoute()
{
    return vRoute;
}

BlockNode* FPGAConfig::getFpgaOutBlock(){
    return fpgaOutBlock;
}

Arch *FPGAConfig::getArch()
{
    return arch;
}

BlockNode *FPGAConfig::findTargetBlockNode(pair<int, int> loc, string ioName)
{
    BlockNode* targetBlock;
    if(loc.first != 0 && loc.second != 0 && loc.first != size.second - 1 && loc.second != size.first - 1)
        targetBlock = clbLayout[loc.first][loc.second];
    else{   //iopad
        vector<BlockNode*> vb = ioLayout[loc.first][loc.second];
        for(int i = 0; i < vb.size(); i++)
            if(vb[i]->getName() == ioName || vb[i]->getName() == ("out:" + ioName))
                targetBlock = vb[i];
    }
    return targetBlock;
}

pair<int, int> FPGAConfig::getSize()
{
    return size;
}

vector<int> FPGAConfig::getRRNodeUsed()
{
    return rrNodeUsed;
}
