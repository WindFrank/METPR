#include "NetAnalyzer.h"
#include "../Tool.h"

NetAnalyzer::NetAnalyzer(){}

/*
    NetAnalyzer(string): directly configure net info.
    module: the name of module. You need to place the .net.post_routing, .place, .route into the same content.
    arch: the target architecture.
*/
NetAnalyzer::NetAnalyzer(string filename, Arch* arch){
    this->arch = arch;
    file = Tool::getXMLTree(filename, "block");
}

BlockNode* NetAnalyzer::netConfig(){
    return netConfig("", arch);
}

BlockNode* NetAnalyzer::netConfig(string filename, Arch* arch){
    if(filename != "")
        file = Tool::getXMLTree(filename, "block");
    //Outer block info collection.
    map<string, pair<BlockNode*, GPort*>> globalPort;
    outBlock = new BlockNode(file);
    outBlock->loadPortfromTag("all");

    /*
        1.Construct the node structure
    */
    constructNode(outBlock, "", "");
    
    /*
        2.Connect the inner node
        Set the Delay in inner connection.
        Save the global port and node.
    */

    vector<BlockNode*> outSubNodes = outBlock->getSubBlocks();
    for(auto subIt = outSubNodes.begin();
        subIt < outSubNodes.end();
        subIt++){
        BlockNode* subNode = *subIt;
        map<string, vector<string>> outputMap = subNode->getTypePorts("outputs");
        //Connect inner output port to recentNode output port.
        for(auto it = outputMap.begin(); it != outputMap.end(); it++){
            string recentPort = it->first;
            vector<string> recentPins = it->second;
            for(int pinIndex = 0; pinIndex < recentPins.size(); pinIndex++){
                string pin = recentPins[pinIndex];
                GPort* port = new GPort("outputs", recentPort, pinIndex);
                string outputConnectPort = subNode->connectOutputPort(pin, port, arch);
                if(outputConnectPort != ""){
                    globalPort[outputConnectPort] = pair<BlockNode*, GPort*>(subNode, port);
                }
            }
        }
        subNode->setIsConnect("output");
        //Connect subNode input to inner port
        subNode->connectInputPort(arch);
        subNode->setIsConnect("input");
    }  

    /*
        3.Connect global node
    */
    map<string, vector<string>> fpgaInputPorts = outBlock->getTypePorts("inputs");
    map<string, vector<string>> fpgaOutputPorts = outBlock->getTypePorts("outputs");
    //Construct direct
    for(auto it = fpgaOutputPorts.begin(); it != fpgaOutputPorts.end(); it++){
        string portName = it->first;
        vector<string> pinList = it->second;
        for(int i = 0; i < pinList.size(); i++){
            string recentPin = pinList[i];
            if(recentPin == "open")
                continue;
            else if(recentPin.find("->") != string::npos)
                continue;
            else{   //Global
                for(auto item = outSubNodes.begin(); item < outSubNodes.end(); item++){
                    BlockNode* gNode = *item;
                    string gName = gNode->getName();
                    if(recentPin == gName){
                        if(recentPin.find("out:") != string::npos){//out connect
                            GPort* startPort = new GPort("outputs", portName, i);
                            GPort* fakeEndPort = new GPort("outputs", "outputs", -1);
                            ConnectNode* cnode = new ConnectNode(startPort, fakeEndPort, outBlock);
                            gNode->addCNode(cnode, "outer");
                        }
                    }
                }
            }
        }
    }
    for(auto it = fpgaInputPorts.begin(); it != fpgaInputPorts.end(); it++){
        string portName = it->first;
        vector<string> pinList = it->second;
        for(int i = 0; i < pinList.size(); i++){
            string recentPin = pinList[i];
            if(recentPin == "open")
                continue;
            else if(recentPin.find("->") != string::npos)
                continue;
            else{   //Global
                for(auto item = outSubNodes.begin(); item < outSubNodes.end(); item++){
                    BlockNode* gNode = *item;
                    string gName = gNode->getName();
                    if(recentPin == gName){
                        //input connect. The portName is set "inputs" specially
                        GPort* fakeStartPort = new GPort("inputs", "inputs", -1);
                        GPort* endPort = new GPort("inputs", "inputs", 0);
                        ConnectNode* cnode = new ConnectNode(fakeStartPort, endPort, gNode);
                        outBlock->addCNode(cnode, "inner");
                    }
                }
            }
        }
    }
    //Construct global
    for(auto subIt = outSubNodes.begin();
        subIt < outSubNodes.end();
        subIt++){
        BlockNode* subNode = *subIt;
        map<string, vector<string>> inputMap = subNode->getTypePorts("inputs");
        for(auto it = inputMap.begin(); it != inputMap.end(); it++){
            string recentPort = it->first;
            vector<string> recentPins = it->second;
            for(int pinIndex = 0; pinIndex < recentPins.size(); pinIndex++){
                string pin = recentPins[pinIndex];
                if(pin != "open" && pin.find("->") == string::npos){
                    GPort* port = new GPort("inputs", recentPort, pinIndex);
                    GPort* startPort = globalPort[pin].second;
                    BlockNode* startNode = globalPort[pin].first;

                    ConnectNode* cnode = new ConnectNode(startPort, port, subNode);
                    startNode->addCNode(cnode, "outer");
                }
            }
        }
    } 
    BlockNode* result = outBlock;
    return result;
}

bool NetAnalyzer::constructNode(BlockNode* root, string instance, string mode){
    TiXmlElement* xmlNode = root->getXMLNode();
        
    for(TiXmlElement* iNode = xmlNode->FirstChildElement("block");
        iNode != NULL && iNode->Value() != "block";
        iNode = iNode->NextSiblingElement("block")){
        //string subName = iNode->FirstAttribute()->Value();
        //skip open node
        // if(subName == "open")
        //     continue;

        BlockNode* subNode = new BlockNode(iNode);
        subNode->loadPortfromTag("all");

        string preIns = instance;
        string recentMode = mode;
        string point = instance == "" ? "" : ".";
        string recentIns = subNode->getInstance();

        preIns += point + recentIns;

        string modeStr = "default";
        string preMode = mode;
        if(iNode->Attribute("mode") != NULL)
            modeStr = iNode->Attribute("mode");//mode check and add
        //EArch has no mode about "lut5", change it specially.
        // if(modeStr == "lut5" && arch->getName() == "EArch")
        //     modeStr = "default";
        preMode += point + modeStr;

        subNode->setPreType(preIns);
        subNode->setPreMode(preMode);
        root->addSubBNode(subNode);
        constructNode(subNode, preIns, preMode);
        //Tail Port Collect
        // map<string, vector<string>> subOutPorts = subNode.getTypePorts("output");
        // for(auto it = subOutPorts.begin(); it != subOutPorts.end(); it++){
        //     vector<string> pins = it->second;
        //     for(auto pin = pins.begin(); pin < pins.end(); pin++){
        //         //Global pin collect
        //         if(pin->find("-&gt;") == string::npos){
        //             globalPort[*pin] = &subNode; 
        //         }
        //     }
        // }
    }
    return true;
}