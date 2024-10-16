/*
    Block: block data structure in net.
*/
#pragma once

#include "ConnectNode.h"

#include <vector>
#include <iostream>
#include <map>

class BlockNode : public GNode{
private:
    //ID name
    string nodeName;
    //instance
    string instance;
    //instance index
    int insIndex;
    //Save the type of blocks out recent block
    string preType;
    //Save the mode of blocks out recent block
    string preMode;
    //Save the connect node 
    map<string, map<string, vector<string>>> ports;
    //Save the global pin used to connect
    map<string, string> realPorts;
    //Save the Block in it.
    vector<BlockNode*> subBlocks;
    //Save the node pointer connect to it
    //vector<ConnectNode*> connectNodes;
    vector<ConnectNode*> globalCNodes;
    //Save the inner connection
    vector<ConnectNode*> innerConnects;
    //Save the Delay info
    vector<GDelay> delays;
    //Save the node xml
    TiXmlElement* nodeXml;
    //If the inner conncections of subNodes have been set.
    bool isInnerCOutSet = false;
    bool isInnerCInSet = false;
    //Inner delay
    vector<GDelay> innerDelays;
    //If the BlockNode is iopad, it denotes the subblk of iopad in certain location.
    int subblk = -1;
    //If the BlockNode is iopad, it denotes the ptc of iopad in rr_graph.xml.
    int ptc = -1;
public:
    BlockNode();
    BlockNode(TiXmlElement* file);
    string getName();
    map<string, vector<string>> getTypePorts(string command);
    string getInstance();
    //Get the Full name of instance.
    string getFullInstance();
    void setPreType(string preType);
    void setPreMode(string preMode);
    void setIsConnect(string type);
    string getPreType();
    string getPreMode();
    //Get delay from one end to end path.
    double getDelay(string first, string endk);
    void addInOut(string command, string name, vector<string>* pins);
    void addSubBNode(BlockNode* subB);
    void addCNode(ConnectNode* cnode, string command);
    vector<ConnectNode*>* getCNode(string inout);
    //find a CNode set each of which is assigned start port
    vector<ConnectNode*> getCNode(GPort* startPort, string inout);
    //find a single CNode by certain sinkPort and the name of its node
    ConnectNode* getCNode(string sinkPort, string inout, string nextNodeName);
    void setSubblk(int subblk);
    int getSubblk();
    vector<BlockNode*> getSubBlocks();
    void loadXML(TiXmlElement* fileNode);
    TiXmlElement* getXMLNode();
    void loadPortfromTag(string command);
    string connectOutputPort(string outPort, GPort* outPortInfo, Arch* arch);
    void connectInputPort(Arch* arch);
    void setPtc(int ptc);
    int getPtc();
};