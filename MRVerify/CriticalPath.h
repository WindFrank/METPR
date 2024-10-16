#include "../EDAExecuter/FPGAConfig.h"
#include <set>

struct Path{
    vector<BlockNode*> bNode;
    vector<ConnectNode*> cNode;
    double delay = 0;
    RouteInfo* routeInfo;
};

struct CompletePath{
    vector<BlockNode*> bNode;
    vector<ConnectNode*> cNode;
    vector<RouteInfo*> cRoute;
    double delay;
};

class CriticalPath{
private:
    vector<BlockNode*> bNode;
    vector<ConnectNode*> cNode;
    vector<RouteInfo*> cRoute;
    double delay;
    vector<CompletePath> allPaths;
    pair<set<BlockNode*>, vector<Path>> graph;
    vector<Path> inPaths;
    vector<Path> outPaths;
    set<BlockNode*> visitedEndNode;
    vector<int> stack;
    vector<vector<int>> completePath;
    void findInPath(BlockNode* sinkBlock, string sinkPort, Path curentPath);
    void findOutPath(BlockNode* sourceBlock, BlockNode* clbBlock, string sinkPort, Path currentPath);
    void getThroughCLB(FPGAConfig* fconfig, RouteInfo* ri, Path p);
    void findCompletePath(vector<vector<vector<Path>>> matrix, int start);
public:
    CriticalPath();
    CriticalPath(FPGAConfig* fconfig);
    vector<BlockNode*> getBlockNode();
    vector<ConnectNode*> getConnectNode(); 
    vector<RouteInfo*> getRouteInfo();
    vector<CompletePath> getAllPaths();
    double getDelay();
};