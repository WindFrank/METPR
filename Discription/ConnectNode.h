/*
    ConnectNode: Denote the global connection and its delay and more info.
    It is completely build in RouteAnalyzer
*/

#include "GNode.h"
#include "GPort.h"

class ConnectNode : public GNode{
private:
    string id;
    string nextInstance;
    GPort* startPort;
    GNode* nextNode;
    GPort* nextPort;
    double delay;
public:
    ConnectNode();
    ConnectNode(GPort* startPort, GPort* nextPort, GNode* nextNode) :
        startPort(startPort),
        nextPort(nextPort),
        nextNode(nextNode),
        nextInstance(nextNode->getInstance()){};
    
    string getIndexPin();
    //The firstEnd and secondEnd should be "" here.
    double getDelay(string firstEnd, string secondEnd);
    void setDelay(double delay);
    void setId(string s);
    string getInstance();
    GNode* getNextBlockNode();
    string getStartPort();
    string getID();
    string getNextPort();
    string getStartType();
    string getNextType();
};