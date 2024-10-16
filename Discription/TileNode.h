/*
    TileNode: Save the info of the tile node in the rr_graph.xml
*/

#include <string>

using namespace std;

class TileNode{
public:
    int capacity = -1;
    int nodeID = -1;
    string type;
    int pin;
    pair<int, int> lowPoint;
    pair<int, int> highPoint;
    double C = -0.1;
    double R = -1;

    TileNode(){};
    TileNode(int capacity, int nodeID, string type, int pin, int xhigh, int xlow, int yhigh, int ylow, double C, double R)
      : capacity(capacity),
        nodeID(nodeID),
        type(type),
        pin(pin),
        lowPoint(pair<int, int>(xlow, ylow)),
        highPoint(pair<int, int>(xhigh, yhigh)),
        C(C),
        R(R){};
};