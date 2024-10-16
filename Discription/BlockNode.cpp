#include "BlockNode.h"
#include "../Tool.h"

BlockNode::BlockNode(){}

BlockNode::BlockNode(TiXmlElement* file){
    loadXML(file);
}

string BlockNode::getName(){
    return nodeName;
}

string BlockNode::getInstance(){
    return instance;
}

string BlockNode::getFullInstance(){
    if(insIndex == -1)
        return instance;
    else
        return instance + "[" + to_string(insIndex) + "]";
}

void BlockNode::setPreType(string preType)
{
    this->preType = preType;
}

void BlockNode::setPreMode(string preMode)
{
    this->preMode = preMode;
}

void BlockNode::setIsConnect(string type)
{
    if(type == "input")
        isInnerCInSet = true;
    else if(type == "output")
        isInnerCOutSet = true;
}

string BlockNode::getPreType()
{
    return preType;
}

string BlockNode::getPreMode()
{
    return preMode;
}

map<string, vector<string>> BlockNode::getTypePorts(string command){
    if(ports.count(command) != 0)
        return ports[command];
    else{
        return map<string, vector<string>>();
    }
}

/*
    getDelay: find and get delay from the end to end path. If no match ports, return 0.0
    first: the pin into the block.
    end: the pin out the block.
*/
double BlockNode::getDelay(string first, string end)
{
    for(auto it = delays.begin(); it < delays.end(); it++){
        GDelay delay = *it;
        string firstPin = delay.getFirstPin();
        string endPin = delay.getEndPin();
        if(firstPin.find("-") != string::npos)
            firstPin = firstPin.substr(0, firstPin.find("-"));
        if(endPin.find("-") != string::npos)
            endPin = endPin.substr(0, endPin.find("-"));
        if(firstPin.find(".") != string::npos)
            firstPin = firstPin.substr(firstPin.find(".") + 1, firstPin.size() - firstPin.find(".") - 1);
        if(endPin.find(".") != string::npos)
            endPin = endPin.substr(endPin.find(".") + 1, endPin.size() - endPin.find(".") - 1);
        if(first == firstPin && end == endPin)
            return delay.getValue();
    }
    return 0.0;
}

void BlockNode::addInOut(string command, string name, vector<string>* pins)
{
    if(command == "inputs" || command == "outputs" || command == "clocks"){
        vector<string> newPins = *pins;
        vector<string> elem;
        for(auto it = newPins.begin(); it < newPins.end(); it++)
            elem.push_back(*it);
        ports[command][name] = elem;
    }
    else{
        Tool::error("Error: BlockNode command invalid.");
    }
}

void BlockNode::addSubBNode(BlockNode* subB){
    subBlocks.push_back(subB);
}

void BlockNode::addCNode(ConnectNode* cnode, string command){
    if(command == "inner")
        innerConnects.push_back(cnode);
    else if(command == "outer")
        globalCNodes.push_back(cnode);
    else
        Tool::error("Error: addCNode cannot found command" + command);
}

/*
    Get target cnodes by type.
    inout: "global" get from global ConnectNode; "inner" get from inner
*/
vector<ConnectNode*>* BlockNode::getCNode(string inout){
    vector<ConnectNode*>* cv;
    if(inout == "global")
        cv = &globalCNodes;
    else
        cv = &innerConnects;
    return cv;
}

/*
    find a CNode set each of which is assigned start port
    startPort: the assigned start port of target cnode
    inout: "global" get from global ConnectNode; "inner" get from inner
*/
vector<ConnectNode *> BlockNode::getCNode(GPort *startPort, string inout)
{
    vector<ConnectNode*>* cv = getCNode(inout);
    vector<ConnectNode *> result;
    for(auto it = cv->begin(); it < cv->end(); it++){
        ConnectNode* rcnode = *it;
        string recentStartPin =  rcnode->getStartPort();
        if(startPort->getPortExp() == recentStartPin)
            result.push_back(rcnode);
    }
    return result;
}

/*
    Get target single CNode by sinkPort and nextNode's Name
    inout: "global" get from global ConnectNode; "inner" get from inner
    nextNodeName: the name of nextNode
*/
ConnectNode *BlockNode::getCNode(string sinkPort, string inout, string nextNodeName)
{
    vector<ConnectNode*>* cv = getCNode(inout);
    for(auto it = cv->begin(); it < cv->end(); it++){
        ConnectNode* rcnode = *it;
        string recentPin =  rcnode->getInstance() + "." + rcnode->getIndexPin();
        string nextnodeName = ((BlockNode*)(rcnode->getNextBlockNode()))->getName();
        if(recentPin == sinkPort && nextNodeName == nextnodeName)
            return (*it);
    }
    return NULL;
}

void BlockNode::setSubblk(int subblk)
{
    this->subblk = subblk;
}

int BlockNode::getSubblk()
{
    return subblk;
}

vector<BlockNode*> BlockNode::getSubBlocks(){
    return subBlocks;
}

void BlockNode::loadXML(TiXmlElement* fileNode){
    nodeXml = fileNode;
    nodeName = nodeXml->FirstAttribute()->Value();
    string instance_index = nodeXml->FirstAttribute()->Next()->Value();
    int leftIndex = instance_index.find("[");
    if(leftIndex != string::npos){
        instance = instance_index.substr(0, leftIndex);
        insIndex = Tool::getNumFromClose(instance_index);
    }
    else{
        instance = instance_index;
        insIndex = -1;
    }
}

TiXmlElement* BlockNode::getXMLNode(){
    return nodeXml;
}


void BlockNode::loadPortfromTag(string command){
    if(command == "all"){
        loadPortfromTag("inputs");
        loadPortfromTag("outputs");
        loadPortfromTag("clocks");
        return;
    }
    if(command != "inputs" && command != "outputs" && command != "clocks"){
        Tool::error("Error: BlockNode loadPortfromTag command invalid.");
        return;
    }
    if(nodeXml == NULL){
        Tool::error("Error: BlockNode nodeXml unload.");
        return;
    }
    if(instance.find("FPGA_packed_netlist") != string::npos || nodeXml->FirstChildElement(command.c_str()) == NULL){   //No port, directly load pins or empty node
        for(TiXmlElement* pNode = nodeXml->FirstChildElement();
            pNode != NULL;
            pNode = pNode->NextSiblingElement()){
            string pContent = "";
            if(command != pNode->Value())
                continue;
            if(pNode->GetText() != NULL){
                pContent = pNode->GetText();
                vector<string>* pins = new vector<string>();
                vector<string> temp = Tool::split(pContent, ' ');
                for(int i = 0; i < temp.size(); i++){
                    string item = temp[i];
                    pins->push_back(item);
                }
                addInOut(command, command, pins);
            }
            else{
                addInOut(command, command, new vector<string>());
            }
        }
    }
    else{
        for(TiXmlElement* pNode = nodeXml->FirstChildElement(command.c_str())->FirstChildElement();
            pNode != NULL;
            pNode = pNode->NextSiblingElement()){
            string tagName = pNode->Value();
            if(tagName != "port")
                continue;
            string pName = pNode->FirstAttribute()->Value();
            string pContent = pNode->GetText();
            vector<string>* pins = new vector<string>();
            vector<string> temp = Tool::split(pContent, ' ');
            for(int i = 0; i < temp.size(); i++){
                string item = temp[i];
                pins->push_back(item);
            }
            addInOut(command, pName, pins);
        }
    }
    
}

string BlockNode::connectOutputPort(string outPort, GPort* outPortInfo, Arch* arch)
{
    string realPort = "";
    string fatherFullIns = getFullInstance();
    string pureOutport = "";
    string fullOutport = "";
    string nodeInstance = "";

    string targetNodeName = "";
    string targetPortName = "";
    int targetPinIndex = 0;



    if(outPort.find("out:") != string::npos)        //out:vector^out: skip
        return "";
    else if(outPort == "open")                      //open: skip
        return "";
    else if(outPort.find("->") != string::npos){ //inner connect
        fullOutport = outPort.substr(0, outPort.find('-'));
        nodeInstance = outPort.substr(0, outPort.find('.'));
        pureOutport = fullOutport.substr(fullOutport.find('.') + 1, fullOutport.size() - fullOutport.find(".") - 1);
        Arch::isolateNameandIndex(pureOutport, &targetPortName, &targetPinIndex, &targetPinIndex);
        string endPin = getFullInstance() + "." + outPortInfo->getPortExp();
        if(nodeInstance.find("[") != string::npos)
            targetNodeName = nodeInstance.substr(0, nodeInstance.find("["));
        else
            targetNodeName = nodeInstance;
        if(targetNodeName == instance){              //connect to itself: directly set the delay to the node
            if(!isInnerCOutSet){
                string fatherPreType = preType.substr(0, preType.rfind("."));
                string fatherPreMode = preMode.substr(0, preMode.rfind("."));
                double delay = arch->getDelay(fatherPreType, fatherPreMode, outPort, endPin);
                GDelay gDelay(fatherPreType, fatherPreMode, outPort, outPortInfo->getPortExp(), delay);
                delays.push_back(gDelay);   //The output's source is input, no realPort need to be returned.
            }
        }
        //find corresponding subNode port meanwhile configure them
        string thisFullInstance = getFullInstance();
        for(auto it = subBlocks.begin(); it < subBlocks.end(); it++){
            BlockNode* recentBlock = (*it);
            //check if the node is target node
            bool isTargetNode = nodeInstance == recentBlock->getFullInstance();
            //Configure subNodes
            map<string, vector<string>> outPortList = recentBlock->getTypePorts("outputs");
            for(auto item = outPortList.begin(); item != outPortList.end(); item++){
                string portName = item->first;
                vector<string> pins = item->second;
                bool isTargetPort = portName == targetPortName;
                for(int i = 0; i < pins.size(); i++){
                    string thisPin = pins[i];
                    GPort* port = new GPort("outputs", portName, i);
                    string thisRealPin = recentBlock->connectOutputPort(thisPin, port, arch);
                    if(isTargetNode && isTargetPort && i == targetPinIndex){
                        realPort = thisRealPin;
                        if(!isInnerCOutSet){
                            ConnectNode* cnode = new ConnectNode(port, outPortInfo, this);
                            double delay = arch->getDelay(preType, preMode, outPort, endPin);
                            cnode->setDelay(delay);
                            recentBlock->addCNode(cnode, "outer");
                        } 
                    }
                }
            }
            recentBlock->setIsConnect("output");
        }
        return realPort;
    }
    else{   //global port load, node's delay set of this out
        if(!isInnerCOutSet){
            string fatherPreType = preType.substr(0, preType.rfind("."));
            string fatherPreMode = preMode.substr(0, preMode.rfind("."));
            map<string, vector<string>> inputs = getTypePorts("inputs");
            string thisOutputPin = getFullInstance() + "." + outPortInfo->getPortExp();
            for(auto it = inputs.begin(); it != inputs.end(); it++){
                string portName = it->first;
                vector<string> pins = it->second;
                for(int i = 0; i < pins.size(); i++){
                    string thisInputPin = getFullInstance() + "." + portName + "[" + to_string(i) + "]";
                    double delay = arch->getDelay(fatherPreType, fatherPreMode, thisInputPin, thisOutputPin);
                    GDelay gDelay(fatherPreType, fatherPreMode, thisInputPin, thisOutputPin, delay);
                    delays.push_back(gDelay);
                }
            }
        }
        return outPort;
    }
}

void BlockNode::connectInputPort(Arch* arch)
{
    //SubNode InputPort Connect
    for(auto it = subBlocks.begin();
        it < subBlocks.end();
        it++){
        BlockNode* recentBlock = (*it);
        if(isInnerCInSet) //If it has cnode, it must have been connected before, break
            break;
        map<string, vector<string>> portList = recentBlock->getTypePorts("inputs");
        for(auto item = portList.begin();
            item != portList.end();
            item++){
            vector<string> pins = item->second;
            for(int i = 0; i < pins.size(); i++){
                string pinName = pins[i];
                if(pinName == "open")
                    continue;
                else if(pinName.find("->") == string::npos){ // global port, skip
                    continue;
                }
                else{
                    //Connect input to its father or siblings
                    string involveIns = pinName.substr(0, pinName.find('.'));
                    ConnectNode* cnode;
                    GPort* port;
                    GPort* startPort;
                    //Get start port
                    int startPoint = pinName.find('.');
                    string backPartString = pinName.substr(startPoint + 1, pinName.size() - startPoint - 1);
                    int startPinIndex = Tool::getNumFromClose(backPartString);
                    int leftCloseIndex = backPartString.find('[');
                    string startPortName = backPartString.substr(0, leftCloseIndex);
                
                    string fatherFullIns = instance;
                    if(fatherFullIns == involveIns){//Father check
                        startPort = new GPort("inputs", startPortName, startPinIndex);
                        port = new GPort("inputs", item->first, i);
                        cnode = new ConnectNode(startPort, port, recentBlock);
                        string firstPin = pinName;
                        string endPin = recentBlock->getFullInstance() + "." + port->getPortExp();
                        double delay = arch->getDelay(preType, preMode, firstPin, endPin);
                        cnode->setDelay(delay);
                        addCNode(cnode, "inner");
                    }
                    else{   //Subnode Check
                        for(auto subIt = subBlocks.begin(); subIt < subBlocks.end(); subIt++){
                            BlockNode* subBlock = *subIt;
                            string subBlockIns = subBlock->getFullInstance();
                            if(subBlockIns == involveIns){
                                startPort = new GPort("outputs", startPortName, startPinIndex);
                                port = new GPort("inputs", item->first, i);
                                cnode = new ConnectNode(startPort, port, recentBlock);
                                string endPin = recentBlock->getFullInstance() + "." + port->getPortExp();
                                double delay = arch->getDelay(preType, preMode, pinName, endPin);
                                cnode->setDelay(delay);
                                subBlock->addCNode(cnode, "outer");
                                break;
                            }
                            if(subIt + 1 == subBlocks.end()){
                                Tool::error("Error: BlockNode connectInputPort cannot find nodeType: " + involveIns);
                            }
                        }
                    }
                }
            }
        }
        recentBlock->connectInputPort(arch);
    }
    isInnerCInSet = true;
}

void BlockNode::setPtc(int ptc)
{
    this->ptc = ptc;
}

int BlockNode::getPtc()
{
    return ptc;
}
