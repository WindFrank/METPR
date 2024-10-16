#include "CriticalPath.h"
#include "algorithm"
#include <set>

CriticalPath::CriticalPath(){}

/*
    findInPath(sinkBlock, sinkPort, currentPath): find all paths from top a clb block to bottom blocks
    sinkBlock: next blockNode
    sinkPort: sinkPort
    currentPath: save path info
*/
void CriticalPath::findInPath(BlockNode* sinkBlock, string sinkPort, Path currentPath){
    vector<ConnectNode*> innerCNode = *sinkBlock->getCNode("inner");
    if(innerCNode.empty()){
        inPaths.push_back(currentPath);
        return;
    }
    else{
        string initialSinkPort = sinkPort;
        for(int i = 0; i < innerCNode.size(); i++){
            if(innerCNode[i]->getStartPort().compare(sinkPort) == 0){
                sinkBlock = static_cast<BlockNode*>(innerCNode[i]->getNextBlockNode());
                currentPath.delay += innerCNode[i]->getDelay("", "");
                currentPath.bNode.push_back(sinkBlock);
                currentPath.cNode.push_back(innerCNode[i]);
                sinkPort = innerCNode[i]->getIndexPin();
                findInPath(sinkBlock, sinkPort, currentPath);
                currentPath.bNode.pop_back();
                currentPath.cNode.pop_back();
                currentPath.delay -= innerCNode[i]->getDelay("", "");
                sinkPort = initialSinkPort;
            }
        }
    }
}

/*
    findOutPath(sourceBlock, clbBlock, sinkPort, currentPath): find all paths from a bottom block to the clb block
    sourceBlock: the bottom block to find paths
    clbBlock: which the bottom block belongs to
    sinkPort: sinkPort
    currentPath: save path info
*/
void CriticalPath::findOutPath(BlockNode* sourceBlock, BlockNode* clbBlock, string sinkPort, Path currentPath){
    currentPath.bNode.push_back(sourceBlock);
    vector<ConnectNode*> globalCNode = *sourceBlock->getCNode("global");
    if(sourceBlock == clbBlock){
        outPaths.push_back(currentPath);
        return;
    }
    else{
        string initialSinkPort = sinkPort;
        for(int i = 0; i < globalCNode.size(); i++){
            if(sinkPort.compare("none") == 0 || globalCNode[i]->getStartPort().compare(sinkPort) == 0){
                //same level
                if(globalCNode[i]->getStartType().compare("outputs") == 0 && globalCNode[i]->getNextType().compare("inputs") == 0){
                    currentPath.cNode.push_back(globalCNode[i]);
                    currentPath.delay += globalCNode[i]->getDelay("", "");
                    BlockNode* sinkBlock = static_cast<BlockNode*>(globalCNode[i]->getNextBlockNode());
                    //bottom
                    if(sinkBlock->getCNode("inner")->empty()){
                        if(visitedEndNode.count(sinkBlock) > 0){
                            currentPath.bNode.push_back(sinkBlock);
                            graph.second.push_back(currentPath);
                        }
                        else{
                            visitedEndNode.insert(sinkBlock);
                            currentPath.bNode.push_back(sinkBlock);
                            graph.first.insert(sinkBlock);
                            graph.second.push_back(currentPath);
                            Path p;
                            findOutPath(sinkBlock, clbBlock, "none", p);
                        }
                        currentPath.cNode.pop_back();
                        currentPath.delay -= globalCNode[i]->getDelay("", "");
                        sinkPort = initialSinkPort;
                    }
                    else{
                        //not bottom, change direction
                        Path initialPath = currentPath;
                        sinkPort = globalCNode[i]->getIndexPin();
                        inPaths.clear();
                        currentPath.bNode.push_back(sinkBlock);
                        findInPath(sinkBlock, sinkPort, currentPath);
                        vector<Path> initInPaths = inPaths;
                        for(int j = 0; j < inPaths.size(); j++){
                            sinkBlock = inPaths[j].bNode.back();
                            if(visitedEndNode.count(sinkBlock) > 0){
                                graph.second.push_back(inPaths[j]);
                            }
                            else{
                                graph.first.insert(sinkBlock);
                                visitedEndNode.insert(sinkBlock);
                                graph.second.push_back(inPaths[j]);
                                Path p;
                                findOutPath(sinkBlock, clbBlock, "none", p);
                            }
                            inPaths = initInPaths;
                            currentPath = initialPath;
                            currentPath.cNode.pop_back();
                            currentPath.delay -= globalCNode[i]->getDelay("", "");
                            sinkPort = initialSinkPort;
                        }
                    }
                }
                //common out
                else{
                    currentPath.cNode.push_back(globalCNode[i]);
                    currentPath.delay += globalCNode[i]->getDelay("", "");
                    BlockNode* sinkBlock = static_cast<BlockNode*>(globalCNode[i]->getNextBlockNode());
                    sinkPort = globalCNode[i]->getIndexPin();
                    findOutPath(sinkBlock, clbBlock, sinkPort, currentPath);
                    currentPath.cNode.pop_back();
                    currentPath.delay -= globalCNode[i]->getDelay("", "");
                    sinkPort = initialSinkPort;
                }
            }
        }
    }
}

/*
    getThroughCLB: load and save all the paths during the process of getting in and getting out a clb block
    fconfig: basic info of fpga
    ri: routeInfo between clb blocks
    p: save path info
*/
void CriticalPath::getThroughCLB(FPGAConfig* fconfig, RouteInfo* ri, Path p){
    vector<vector<BlockNode*>> clbLayout = fconfig->getClbLayout();
    vector<vector<vector<BlockNode*>>> ioLayout = fconfig->getIoLayout();
    vector<RouteInfo*> vRoute = fconfig->getVRoute();
    if(ri->sinkPort.substr(0, 4) == "io.o"){
        BlockNode* sinkBlock;
        vector<BlockNode*> sbLoc = ioLayout[ri->sinkLoc.first][ri->sinkLoc.second];
        for(int i = 0; i < sbLoc.size(); i++) {
            if(sbLoc[i]->getName().substr(4).compare(ri->netName) == 0) {
                sinkBlock = sbLoc[i];
                p.bNode.push_back(sinkBlock);
                graph.first.insert(sbLoc[i]);
                break;
            }
        }
        string sinkPort = ri->sinkPort.substr(3);
        vector<ConnectNode*> innerCNode = *sinkBlock->getCNode("inner");
        while(!innerCNode.empty()){
            for(int i = 0; i < innerCNode.size(); i++){
                if(innerCNode[i]->getStartPort().compare(sinkPort) == 0){
                    sinkBlock = static_cast<BlockNode*>(innerCNode[i]->getNextBlockNode());
                    p.bNode.push_back(sinkBlock);
                    p.cNode.push_back(innerCNode[i]);
                    p.delay += innerCNode[i]->getDelay("", "");
                    sinkPort = innerCNode[i]->getIndexPin();
                    break;
                }
            }
            innerCNode = *sinkBlock->getCNode("inner");
        }
        graph.first.insert(sinkBlock);
        graph.second.push_back(p);
        return;
    }
    else{
        BlockNode* clbBlock = clbLayout[ri->sinkLoc.first][ri->sinkLoc.second];
        BlockNode* sinkBlock = clbLayout[ri->sinkLoc.first][ri->sinkLoc.second];
        p.bNode.push_back(sinkBlock);
        string sinkPort = ri->sinkPort.substr(4);
        inPaths.clear();
        findInPath(sinkBlock, sinkPort, p);
        vector<Path> initInPaths = inPaths;
        for(int m = 0; m < inPaths.size(); m++){
            sinkBlock = inPaths[m].bNode.back();
            graph.first.insert(sinkBlock);
            graph.second.push_back(inPaths[m]);
            Path path;
            outPaths.clear();
            findOutPath(sinkBlock, clbBlock, "none", path);
            for(int i = 0; i < outPaths.size(); i++){
                string instance;
                string startPort;
                sinkPort = outPaths[i].cNode.back()->getNextPort();
                vector<ConnectNode*> globalCNode = *outPaths[i].bNode.back()->getCNode("global");
                if(globalCNode.empty()) continue;
                ConnectNode* cn;
                for(int j = 0; j < globalCNode.size(); j++){
                    if(globalCNode[j]->getStartPort().compare(sinkPort) == 0){
                        cn = globalCNode[j];
                        instance = cn->getInstance();
                        sinkPort = cn->getNextPort();
                        startPort = cn->getStartPort();
                        break;
                    }
                }
                bool connected = false;
                for(int j = 0; j < vRoute.size(); j++){
                    if(instance.compare("clb") == 0){
                        if(vRoute[j]->sourcePort.substr(4).compare(startPort) == 0 && vRoute[j]->sinkPort.substr(4).compare(sinkPort) == 0){
                            ri = vRoute[j];
                            if(clbLayout[ri->sourceLoc.first][ri->sourceLoc.second] == clbBlock && clbLayout[ri->sinkLoc.first][ri->sinkLoc.second] == cn->getNextBlockNode()){
                                connected = true;
                                break;
                            }
                        }
                    }
                    else{
                        if(vRoute[j]->sourcePort.substr(4).compare(startPort) == 0 && vRoute[j]->sinkPort.substr(3).compare(sinkPort) == 0){
                            ri = vRoute[j];
                            vector<BlockNode*> outBlocks = ioLayout[ri->sinkLoc.first][ri->sinkLoc.second];
                            auto it = find(outBlocks.begin(), outBlocks.end(), cn->getNextBlockNode());
                            if(clbLayout[ri->sourceLoc.first][ri->sourceLoc.second] == clbBlock && it != outBlocks.end()){
                                connected = true;
                                break;
                            }
                        }
                    }
                }
                if(!connected) continue;
                outPaths[i].cNode.push_back(cn);
                outPaths[i].routeInfo = ri;
                outPaths[i].delay += ri->delay;
                vector<Path> initialOutPaths = outPaths;
                getThroughCLB(fconfig, ri, outPaths[i]);
                outPaths = initialOutPaths;
            }
            inPaths = initInPaths;
        }
    }
}
/*
    findCompletePath(matrix, start): find all paths from the adjancy matrix
    matrix: the adjancy matrix
    start: from which block to start
*/
void CriticalPath::findCompletePath(vector<vector<vector<Path>>> matrix, int start){
    bool end = true;
    for(int i = 0; i < matrix[start].size(); i++){
        if(!matrix[start][i].empty()){
            end = false;
            stack.push_back(i);
            findCompletePath(matrix, i);
            stack.pop_back();
        }
    }
    if(end){
        completePath.push_back(stack);
        return;
    }
}

/*
    CriticalPath(FPGAConfig): find the critical path in the net on FPGA
    FPGAConfig: basic info of FPGA
*/
CriticalPath::CriticalPath(FPGAConfig* fconfig){
    vector<vector<vector<BlockNode*>>> ioLayout = fconfig->getIoLayout();
    vector<vector<BlockNode*>> clbLayout = fconfig->getClbLayout();
    vector<RouteInfo*> vRoute = fconfig->getVRoute();
    CompletePath criticalPath;
    double criticalDelay = 0;
    for(int it = 0; it < vRoute.size(); it++) {
        RouteInfo* ri = vRoute[it];
        // in
        if(ri->sourcePort.substr(0, 4) == "io.i") {
            graph.first.clear();
            graph.second.clear();
            visitedEndNode.clear();
            vector<BlockNode*> sbLoc = ioLayout[ri->sourceLoc.first][ri->sourceLoc.second];
            BlockNode* sourceBlock;
            for(int i = 0; i < sbLoc.size(); i++) {
                if(sbLoc[i]->getName().compare(ri->netName) == 0) {
                    sourceBlock = sbLoc[i]->getSubBlocks()[0];
                    graph.first.insert(sourceBlock);
                    break;
                }
            }
            BlockNode* clbBlock = clbLayout[ri->sinkLoc.first][ri->sinkLoc.second];
            BlockNode* sinkBlock = clbLayout[ri->sinkLoc.first][ri->sinkLoc.second];
            Path p;
            p.bNode.push_back(sourceBlock);
            vector<ConnectNode*> tmp = *sourceBlock->getCNode("global");
            p.cNode.push_back(tmp[0]);
            p.delay = tmp[0]->getDelay("", "");
            sourceBlock = static_cast<BlockNode*>(tmp[0]->getNextBlockNode());
            p.bNode.push_back(sourceBlock);
            vector<ConnectNode*> startCNodes = *sourceBlock->getCNode("global");
            ConnectNode* startCNode;
            for(int i = 0; i < startCNodes.size(); i++){
                if(ri->sinkPort.substr(4) == startCNodes[i]->getNextPort()){
                    startCNode = startCNodes[i];
                    break;
                }
            }
            p.cNode.push_back(startCNode);
            p.bNode.push_back(clbBlock);
            p.routeInfo = ri;
            p.delay += ri->delay;
            string sinkPort = ri->sinkPort.substr(4);
            //tofix:multiple in(multiple CNode)
            inPaths.clear();
            findInPath(sinkBlock, sinkPort, p);
            vector<Path> initInPaths = inPaths;
            for(int m = 0; m < inPaths.size(); m++){
                sinkBlock = inPaths[m].bNode.back();
                graph.first.insert(sinkBlock);
                graph.second.push_back(inPaths[m]);
                //find all the way out
                Path path;
                outPaths.clear();
                findOutPath(sinkBlock, clbBlock, "none", path);
                //todo: use routeInfo to complete paths
                for(int i = 0; i < outPaths.size(); i++){
                    string instance;
                    string startPort;
                    sinkPort = outPaths[i].cNode.back()->getNextPort();
                    vector<ConnectNode*> globalCNode = *outPaths[i].bNode.back()->getCNode("global");
                    if(globalCNode.empty()) continue;
                    ConnectNode* cn;
                    for(int j = 0; j < globalCNode.size(); j++){
                        if(globalCNode[j]->getStartPort().compare(sinkPort) == 0){
                            cn = globalCNode[j];
                            instance = cn->getInstance();
                            sinkPort = cn->getNextPort();
                            startPort = cn->getStartPort();
                            break;
                        }
                    }
                    //check routeInfo
                    RouteInfo* ri;
                    bool connected = false;
                    for(int j = 0; j < vRoute.size(); j++){
                        if(instance.compare("clb") == 0){
                            if(vRoute[j]->sourcePort.substr(4).compare(startPort) == 0 && vRoute[j]->sinkPort.substr(4).compare(sinkPort) == 0){
                                ri = vRoute[j];
                                if(clbLayout[ri->sourceLoc.first][ri->sourceLoc.second] == clbBlock && clbLayout[ri->sinkLoc.first][ri->sinkLoc.second] == cn->getNextBlockNode()){
                                    connected = true;
                                    break;
                                }
                            }
                        }
                        else{
                            if(vRoute[j]->sourcePort.substr(4).compare(startPort) == 0 && vRoute[j]->sinkPort.substr(3).compare(sinkPort) == 0){
                                ri = vRoute[j];
                                vector<BlockNode*> outBlocks = ioLayout[ri->sinkLoc.first][ri->sinkLoc.second];
                                auto it = find(outBlocks.begin(), outBlocks.end(), cn->getNextBlockNode());
                                if(clbLayout[ri->sourceLoc.first][ri->sourceLoc.second] == clbBlock && it != outBlocks.end()){
                                    connected = true;
                                    break;
                                }
                            }
                        }
                    }
                    
                    if(!connected) continue;
                    outPaths[i].cNode.push_back(cn);
                    outPaths[i].routeInfo = ri;
                    outPaths[i].delay += ri->delay;
                    vector<Path> initialOutPaths = outPaths;
                    getThroughCLB(fconfig, ri, outPaths[i]);
                    outPaths = initialOutPaths;
                }
                inPaths = initInPaths;
            }

            vector<BlockNode*> vertex;
            vertex.assign(graph.first.begin(), graph.first.end());
            vector<Path> edge = graph.second;
            int start;
            int vNum = vertex.size();
            vector<vector<vector<Path>>> matrix(vNum, vector<vector<Path>>(vNum));
            for(int i = 0; i < edge.size(); i++){
                int begin;
                int end;
                for(int j = 0; j < vNum; j++){
                    if(vertex[j] == edge[i].bNode.front()) begin = j;
                    if(vertex[j] == edge[i].bNode.back()) end = j;
                }
                if(edge[i].bNode.front()->getInstance().compare("inpad") == 0) start = begin;
                matrix[begin][end].push_back(edge[i]);
            }
            stack.clear();
            stack.push_back(start);
            completePath.clear();
            findCompletePath(matrix, start);
            CompletePath longestPath;
            double totalDelay = 0;
            double maxDelay = 0;
            for(int i = 0; i < completePath.size(); i++){
                vector<Path> completePathSegments;
                for(int j = 0; j < completePath[i].size()-1; j++){
                    if(j != completePath[i].size() - 1) completePathSegments.push_back(matrix[completePath[i][j]][completePath[i][j+1]][0]);
                    totalDelay += matrix[completePath[i][j]][completePath[i][j+1]][0].delay;
                    if(j != completePath[i].size()-2){
                        BlockNode* b = vertex[completePath[i][j+1]];      
                        string firstPin = matrix[completePath[i][j]][completePath[i][j+1]][0].cNode.back()->getNextPort();
                        string endPin = matrix[completePath[i][j+1]][completePath[i][j+2]][0].cNode.front()->getStartPort();
                        totalDelay += b->getDelay(firstPin, endPin);
                    }
                }
                CompletePath cp;
                for(int j = 0; j < completePathSegments.size(); j++){
                    for(int k = 0; k < completePathSegments[j].bNode.size() - 1; k++){
                        cp.bNode.push_back(completePathSegments[j].bNode[k]);
                    }
                    for(int k = 0; k < completePathSegments[j].cNode.size(); k++){
                        cp.cNode.push_back(completePathSegments[j].cNode[k]);
                    }
                    if(completePathSegments[j].routeInfo != nullptr){
                        if(completePathSegments[j].routeInfo->delay >= 1e-11 && completePathSegments[j].routeInfo->delay < 1e-8) cp.cRoute.push_back(completePathSegments[j].routeInfo);
                    }
                    cp.delay = totalDelay;
                }
                cp.bNode.push_back(completePathSegments.back().bNode.back());
                allPaths.push_back(cp);
                if(totalDelay > maxDelay){
                    maxDelay = totalDelay;
                    longestPath = cp;
                }
                totalDelay = 0;
            }
            if(maxDelay > criticalDelay){
                criticalDelay = maxDelay;
                criticalPath = longestPath;
            }
        }
    }

    this->bNode = criticalPath.bNode;
    this->cNode = criticalPath.cNode;
    this->cRoute = criticalPath.cRoute;
    this->delay = criticalPath.delay;

    CriticalPath();
}

vector<BlockNode*> CriticalPath::getBlockNode(){
    return bNode;
}

vector<ConnectNode*> CriticalPath::getConnectNode(){
    return cNode;
}

vector<RouteInfo*> CriticalPath::getRouteInfo(){
    return cRoute;
}

vector<CompletePath> CriticalPath::getAllPaths(){
    return allPaths;
}

double CriticalPath::getDelay(){
    return delay;
}