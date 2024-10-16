/*
    TileEdge: save the info of edge in rr_graph.xml
*/

class TileEdge{
public:
    int sourceNode;
    int sinkNode;
    int switchID;

    TileEdge(int sourceNode, int sinkNode, int switchID) :
        sourceNode(sourceNode), sinkNode(sinkNode), switchID(switchID){}
};