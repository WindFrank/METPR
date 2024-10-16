#include "Arch.h"
#include "Tool.h"



Arch::Arch(){}

Arch::Arch(string archName, string filename){
    archSet(archName, filename);
}

Arch::Arch(string archName, string filename, string rr_graph_filename){
    archSet(archName, filename);
    rrGraphSet(rr_graph_filename);
}

bool Arch::archSet(string archName, string filename){
    this->archName = archName;
    archPath = filename;
    TiXmlDocument* doc = new TiXmlDocument();
    doc->LoadFile(filename);
    if(!doc->LoadFile()){
        Tool::error("Error: getXMLTree cannot load " + filename);
        exit(1);
    }
    root = doc->FirstChildElement();

    //Save the capacity of io
    ioNum = -1; //initial
    TiXmlElement* tileElement = root->FirstChildElement("tiles");
    tileElement = tileElement->FirstChildElement();
    while(tileElement != NULL){
        TiXmlElement* subTileElement = tileElement->FirstChildElement();
        if(subTileElement == NULL)
            continue;
        if(subTileElement->Attribute("name") != NULL){
            string tileElementName = subTileElement->Attribute("name");
            if(tileElementName != "io")
                continue;
        }
        else{
            Tool::error("Error: Arch file " + archName + " has no named tile.");
        }
        if(subTileElement->Attribute("capacity") != NULL){
            ioNum = atoi(subTileElement->Attribute("capacity"));
            break;
        }
        tileElement = tileElement->NextSiblingElement();
    }
    if(ioNum == -1)
        Tool::error("Error: Arch file " + archName + " cannot find io capacity.");

    return 1;
}

bool Arch::rrGraphSet(string rr_graph_filename)
{
    //load rr_graph.xml
    if(rr_graph_filename != ""){
        TiXmlDocument* rrDoc = new TiXmlDocument();
        rrDoc->LoadFile(rr_graph_filename);
        if(!rrDoc->LoadFile()){
            Tool::error("Error: getXMLTree cannot load " + rr_graph_filename);
            exit(1);
        }
        if(rr_graph_root != NULL){
            rr_graph_root = NULL;
        }

        rr_graph_root = rrDoc->FirstChildElement();
    }
    
    //Save TileNode
    tileNodes.clear();  //tileNodes initial
    for(TiXmlElement* firstTileNode = rr_graph_root->FirstChildElement("rr_nodes")->FirstChildElement();
        firstTileNode != NULL;
        firstTileNode = firstTileNode->NextSiblingElement()){
        TileNode tileNode;
        int capacity, nodeID, pin, xhigh, xlow, yhigh, ylow;
        double C, R;
        string type;
        for(TiXmlAttribute* attr = firstTileNode->FirstAttribute();
            attr != NULL;
            attr = attr->Next()){
            string recentName = attr->Name();
            auto value = attr->Value();
            if(recentName == "capacity")
                capacity = atoi(value);
            else if(recentName == "id")
                nodeID = atoi(value);
            else if(recentName == "type")
                type = value;
        }
        //loc Node
        TiXmlElement* locNode = firstTileNode->FirstChildElement();
        for(TiXmlAttribute* attr = locNode->FirstAttribute();
            attr != NULL;
            attr = attr->Next()){
            string recentName = attr->Name();
            int value = atoi(attr->Value());
            if(recentName == "ptc")
                pin = value;
            else if(recentName == "xhigh")
                xhigh = value;
            else if(recentName == "xlow")
                xlow = value;
            else if(recentName == "yhigh")
                yhigh = value;
            else if(recentName == "ylow")
                ylow = value;
        }
        //timing Node
        TiXmlElement* timingNode = locNode->NextSiblingElement();
        for(TiXmlAttribute* attr = timingNode->FirstAttribute();
            attr != NULL;
            attr = attr->Next()){
            string recentName = attr->Name();
            double value = atof(attr->Value());
            if(recentName == "C")
                C = value;
            else if(recentName == "R")
                R = value;
        }
        tileNode = TileNode(capacity, nodeID, type, pin, xhigh, xlow, yhigh, ylow, C, R);
        tileNodes[nodeID] = tileNode;
    }

    //Save TileEdge
    tileEdges.clear();
    for(TiXmlElement* firstTileEdge = rr_graph_root->FirstChildElement("rr_edges")->FirstChildElement();
        firstTileEdge != NULL;
        firstTileEdge = firstTileEdge->NextSiblingElement()){
        int sinkID, sourceID, switchID;
        for(TiXmlAttribute* attr = firstTileEdge->FirstAttribute();
            attr != NULL;
            attr = attr->Next()){
            string recentName = attr->Name();
            auto value = attr->Value();
            if(recentName == "sink_node")
                sinkID = atoi(value);
            else if(recentName == "src_node")
                sourceID = atoi(value);
            else if(recentName == "switch_id")
                switchID = atoi(value);
        }
        TileEdge tileEdge(sourceID, sinkID, switchID);
        if(tileEdges.count(sourceID) == 0){
            vector<TileEdge> newTiles;
            newTiles.push_back(tileEdge);
            tileEdges[sourceID] = newTiles;
        }
        else
            tileEdges[sourceID].push_back(tileEdge);
    }

    return true;
}

bool Arch::isSet(){
    if(archName == ""){
        Tool::error("Error: Arch unset.");
        return 0;
    }
    return 1;
}

/*
    getDelay: get the delay of target module.
*/
double Arch::getDelay(string preType, string preMode, string firstPin, string endPin){
    isSet();
    for(auto it = delayV.begin(); it < delayV.end(); it++){ //load exist delay
        if(it->isMatch(preType, preMode, firstPin, endPin))
            return it->getValue();
    }

    //Locate
    TiXmlElement* complexblockList = root->FirstChildElement("complexblocklist");
    vector<string> typeList = Tool::split(preType, '.');
    if(typeList.back() == "arithmetic")
        int a = 0;
    vector<string> modeList = Tool::split(preMode, '.');
    string targetType, targetMode;
    if(typeList.size() == 0){
        Tool::error("Error:: getDelay locate the top level node.");
        exit(1);
    }
    string connectType = firstPin.substr(firstPin.find(">") + 1, firstPin.size() - firstPin.find(">") - 1);
    TiXmlElement* typeNode = complexblockList;
    int depth = 0;
    int maxDepth = modeList.size();
    while(depth != maxDepth){
        string recentType = typeList[depth];
        string recentMode = modeList[depth];
        for(typeNode = typeNode->FirstChildElement("pb_type");
            typeNode != NULL;
            typeNode = typeNode->NextSiblingElement()){
            string firstAtt = typeNode->FirstAttribute()->Value();
            if(firstAtt == recentType){
                if(recentMode != "default"){
                    for(TiXmlElement* probaTypeNode = typeNode->FirstChildElement("mode");
                        probaTypeNode != NULL;
                        probaTypeNode = probaTypeNode->NextSiblingElement()){
                        string modeStr = probaTypeNode->Attribute("name");
                        if(modeStr == recentMode){
                            typeNode = probaTypeNode;
                            break;
                        }
                    }
                }
                break;
            }
        }
        depth++;
    }

    //Get the numbers of input pins and output pins
    int inputNumPins = 0;
    int outputNumPins = 0;

    //Target delay father node find
    //If is a full inner connection, getNodeDelay
    string firstType = firstPin.substr(0, firstPin.find("."));
    string endType = endPin.substr(0, endPin.find("."));
    // if(typeNode != NULL && firstType != endType && firstType != "lut[0]" && endType != "lut[0]")
    //     typeNode = Tool::getFirstElementByName(typeNode, "interconnect");
    TiXmlElement* connTypeNode = typeNode->FirstChildElement("interconnect");

    if(connTypeNode != NULL && connectType.find("complete:") == string::npos && connectType != firstPin){
    //if(connTypeNode != NULL && firstType != endType){
        typeNode = Tool::getFirstElementByName(typeNode, "interconnect");
    }
    else{
        if(connTypeNode != NULL)
            int a = 0;
        string firstPinName = firstPin.substr(firstPin.find(".") + 1, firstPin.find("[") - firstPin.find(".") - 1);
        firstPinName = firstPinName.substr(0, firstPinName.find("["));
        string endPinName = endPin.substr(endPin.find(".") + 1, endPin.find("[") - endPin.find(".") - 1);
        endPinName = endPinName.substr(0, endPinName.find("["));
        bool inputGet = false;
        bool outputGet = false;

        string innerNodeType = firstPin.substr(0, firstPin.find("[") == string::npos ? firstPin.find(".") : firstPin.find("["));
        // if(typeNode == NULL || typeNode->FirstChildElement("pb_type") == NULL){ // It may because unexpected structure occurs. Return constant delay.
        //     if(typeNode == NULL || typeList.back() == "lut5")   //lut5, return 
        //         return 235e-12;
        //     else if(typeList.back() == "lut4")
        //         return 195e-12;
        //     else if(typeList.back() == "lut6")
        //         return 261e-12;
        //     else
        //         Tool::error("Arch getDelay cannot found target Node Type.");
        // }
        // else{
        TiXmlElement* probaTypeNode = typeNode->FirstChildElement("pb_type");
        for(probaTypeNode = probaTypeNode == NULL ? typeNode : probaTypeNode;
            probaTypeNode != NULL;
            probaTypeNode = probaTypeNode->NextSiblingElement()){
            string typeStr = probaTypeNode->Attribute("name");
            string className = probaTypeNode->Attribute("class") != NULL ? probaTypeNode->Attribute("class") : "";
            int firstTypeRB = firstType.find("[") != string::npos ? firstType.find("[") : firstType.size();
            int endTypeRB = endType.find("[") != string::npos ? endType.find("[") : endType.size();
            //find target node. 1.type match real node; 2. class node vitrual node; 3. from real to virtual
            if(typeStr == innerNodeType || className == firstType.substr(0, firstTypeRB) || className == endType.substr(0, endTypeRB)){
                if(firstType != endType)    // from real to virtual, no delay.
                    return 0.0;
                TiXmlElement* recentElem = probaTypeNode->FirstChildElement();
                bool inputGet = false;
                bool outputGet = false;
                while(!inputGet || !outputGet){
                    string recentArr = recentElem->Attribute("name");
                    if(recentArr == firstPinName){
                        inputNumPins = atoi(recentElem->Attribute("num_pins"));
                        inputGet = true;
                    }
                    else if(recentArr == endPinName){
                        outputNumPins = atoi(recentElem->Attribute("num_pins"));
                        outputGet = true;
                    }
                    recentElem = recentElem->NextSiblingElement();
                }
                typeNode = probaTypeNode;
                if(typeStr != innerNodeType){   //class node: change the pin to the father pin and get delay
                    int pointIndex = firstPin.find(".");
                    int rightIndex = firstPin.find("[") > pointIndex ? pointIndex : firstPin.find("[");
                    firstPin = typeList.back() + firstPin.substr(rightIndex, firstPin.size() - rightIndex);
                    endPin = typeList.back() + endPin.substr(rightIndex, endPin.size() - rightIndex);
                }
                break;
            }
        }
        // }

    }

    typeNode = typeNode->FirstChildElement();

    //Delay search
    int seqFlag = 0;
    double seqDelay = 0.0;
    TiXmlElement* tempNode;
    while(typeNode != NULL){
        string rNodeType = typeNode->Value();
        if(!checkType(rNodeType)){
            typeNode = typeNode->NextSiblingElement();
            //complete seq strategy
            if(typeNode == NULL && seqFlag)
                typeNode = tempNode;
            continue;
        }
        string firstA = typeNode->FirstAttribute()->Value();
        TiXmlAttribute* tempAtt = typeNode->FirstAttribute()->Next();
        string acceptStart = typeNode->FirstAttribute()->Next()->Value();
        string acceptEnd = tempAtt->Next() == NULL ? "" : tempAtt->Next()->Value();
        if(firstPin.find("-") != string::npos)
            firstPin = firstPin.substr(0, firstPin.find("-"));
        if(rNodeType == "T_setup" || rNodeType == "T_clock_to_Q"){//Seq Circuit label
            seqFlag = 1;
            if(isPortMatch(acceptStart, firstPin) || isPortMatch(acceptStart, endPin)){ // input and output
                double recentDelay = atof(typeNode->Attribute("value"));
                string attrPort = typeNode->Attribute("port");
                string attrPortName = attrPort.substr(attrPort.find(".") + 1, attrPort.size() - attrPort.find(".") - 1);
                string firstPinPortName = firstPin.substr(firstPin.find(".") + 1, firstPin.size() - firstPin.find(".") - 1);
                if(!(attrPortName != firstPinPortName && rNodeType == "T_setup"))
                    seqDelay += recentDelay;
            }
            typeNode->NextSiblingElement();
        }
        else if(((firstA != connectType) && !(acceptEnd.find(".") != string::npos && isPortMatch(acceptStart, firstPin) && isPortMatch(acceptEnd, endPin)))
                || (rNodeType == "pack_pattern")
                || (rNodeType == "input")
                || (rNodeType == "output")
                || (rNodeType == "clock")){
            typeNode = typeNode->NextSiblingElement();
        }
        else if(rNodeType == "direct"){
            TiXmlElement* subDNode = typeNode->FirstChildElement();
            if(subDNode == NULL)
                return 0.0;
            else
                typeNode = subDNode;
        }
        else if((rNodeType == "complete" || rNodeType == "mux")){
            if(seqFlag)
                tempNode = typeNode->NextSiblingElement();
            typeNode = typeNode->FirstChildElement();
        }
        else if(rNodeType == "delay_constant"){
            if(seqFlag){
                double recentDelay = atof(typeNode->Attribute("max"));
                seqDelay += recentDelay;
                typeNode->NextSiblingElement();
            }
            else
                return atof(firstA.c_str());
        }
        else if(rNodeType == "delay_matrix"){
            string delayText = typeNode->GetText();
            vector<string> matrix = Tool::split(delayText, ' ');
            int firstPointIndex = firstPin.find(".");
            int endPointIndex = endPin.find(".");
            int row = Tool::getNumFromClose(firstPin.substr(firstPointIndex + 1, firstPin.size() - firstPointIndex - 1));
            int col = Tool::getNumFromClose(endPin.substr(endPointIndex + 1, endPin.size() - endPointIndex - 1));
            if(seqFlag){
                double recentDelay = atof(typeNode->Attribute("max"));
                seqDelay += recentDelay;
                typeNode->NextSiblingElement();
            }
            else
                return atof(matrix[row * outputNumPins + col].c_str());
        }
        else if(rNodeType.find("T") != string::npos){
            Tool::error("Arch getDelay: complex seq function not support");
            exit(1);
        }
        else{
            Tool::error("Arch getDelay: cannot find corresponding arch");
            exit(1);
        }

        //complete seq strategy
        if(typeNode == NULL && seqFlag)
            typeNode = tempNode;
    }
    return seqDelay;
}

bool Arch::isPortMatch(string fpgaPin, string connPin)
{
    if(fpgaPin.find(" ") != string::npos){
        vector<string> optionPins = Tool::split(fpgaPin, ' ');
        for(auto it = optionPins.begin(); it < optionPins.end(); it++)
            if(isPortMatch(*it, connPin))
                return 1;
        return 0;
    }
    string fpgaNodefullName = fpgaPin.substr(0, fpgaPin.find("."));
    string fpgaPinfullName = fpgaPin.substr(fpgaPin.find(".") + 1, fpgaPin.size() - fpgaPin.find(".") - 1);
    string connNodefullName = connPin.substr(0, connPin.find("."));
    string connPinfullName = connPin.substr(connPin.find(".") + 1, connPin.size() - connPin.find(".") - 1);
    string fpgaNodeName, fpgaPinName, connNodeName, connPinName;
    int fpgaNodeMinIndex = -1;
    int fpgaNodeMaxIndex = INT_MAX;
    int fpgaPinMinIndex = -1;
    int fpgaPinMaxIndex = INT_MAX;
    int connNodeIndex = 0;
    int connPinIndex = 0;
    isolateNameandIndex(fpgaNodefullName, &fpgaNodeName, &fpgaNodeMinIndex, &fpgaNodeMaxIndex);
    isolateNameandIndex(fpgaPinfullName, &fpgaPinName, &fpgaPinMinIndex, &fpgaPinMaxIndex);
    isolateNameandIndex(connNodefullName, &connNodeName, &connNodeIndex, &connNodeIndex);
    isolateNameandIndex(connPinfullName, &connPinName, &connPinIndex, &connPinIndex);

    bool isNodeMatch = fpgaNodeName == connNodeName && connNodeIndex >= fpgaNodeMinIndex
                        && connNodeIndex <= fpgaNodeMaxIndex;
    bool isPinMatch = fpgaPinName == connPinName && connPinIndex >= fpgaPinMinIndex
                        && connPinIndex <= fpgaPinMaxIndex;
    return isNodeMatch && isPinMatch;
}

void Arch::isolateNameandIndex(string nameandPin, string* name, int* minIndex, int* maxIndex)
{
    int fpgaCloseIndex = nameandPin.find("[");
    string newName;
    if(fpgaCloseIndex != string::npos)
        newName = nameandPin.substr(0, fpgaCloseIndex);
    else 
        newName = nameandPin;
    *name = newName;
    int colonIndex = nameandPin.find(":");
    if(fpgaCloseIndex != string::npos){
        if(colonIndex != string::npos){
            string leftTemp = nameandPin.substr(0, colonIndex) + "]";
            string rightTemp = "[" + nameandPin.substr(colonIndex + 1, nameandPin.size() - colonIndex - 1);
            *maxIndex = Tool::getNumFromClose(leftTemp);
            *minIndex = Tool::getNumFromClose(rightTemp);
        }
        else{
            *minIndex = Tool::getNumFromClose(nameandPin);
            *maxIndex = *minIndex;
        }
    }
}

double Arch::getNodeDelay(TiXmlElement* xmlNode)
{
    vector<string> inputs;
    vector<string> outputs;
    map<string, double> channelDelay;
    map<string, string> inandOut;

    TiXmlElement* typeNode = xmlNode;
    double maxOutDelay = 0.0;
    typeNode = typeNode->FirstChildElement();
    while(typeNode != NULL){
        string rNodeType = typeNode->Value();

        if( (rNodeType == "input") || (rNodeType == "output") || (rNodeType == "clock")){
            if(rNodeType == "input")
                inputs.push_back(typeNode->FirstAttribute()->Value());
            else  
                outputs.push_back(typeNode->FirstAttribute()->Value());
            typeNode = typeNode->NextSiblingElement();
        }
        else{
            string port = typeNode->FirstAttribute()->Next()->Value();
            port = port.substr(port.find(".") + 1, port.size() - port.find(".") - 1);
            double setDelay = 0.0;
            string uncertainValue = typeNode->FirstAttribute()->Value();
            if(rNodeType == "delay_matrix"){
                vector<vector<string>> matrix = Tool::twoDSplit(typeNode->GetText(), ' ', '\n');

            }
            else
                setDelay = atof(uncertainValue.c_str());
            bool isInput = 0;
            for(auto it = inputs.begin(); it < inputs.end(); it++){
                if(*it == port){
                    isInput = 1;
                    break;
                }
            }
            if(isInput){    //Input
                if(channelDelay.count(port))
                    channelDelay[port] = setDelay;
                else
                    channelDelay[port] += setDelay;
            }
            else{   //output
                for(auto it = inandOut.begin(); it != inandOut.end(); it++){    //find relation in inandout
                    if(it->second == port)
                        channelDelay[it->first] += setDelay;
                }
                if(maxOutDelay < setDelay)
                    maxOutDelay = setDelay;
            }

            if(rNodeType == "delay_constant"){
                string out_port = typeNode->FirstAttribute()->Next()->Next()->Value();
                out_port = out_port.substr(out_port.find(".") + 1, out_port.size() - out_port.find(".") - 1);
                inandOut[port] = out_port;
            }
        }
    }

    //Find the longest path
    double longestPath = 0.0;
    for(auto it = inputs.begin(); it != inputs.end(); it++){
        double recentPath = channelDelay[*it];
        if(inandOut.count(*it) == 0)
            recentPath += maxOutDelay;
        if(recentPath > longestPath)
            longestPath = recentPath;
    }
    return longestPath;
}

string Arch::getName()
{
    return archName;
}

bool Arch::checkType(string s)
{
    for(int i = 0; i < sizeof(allElements)/sizeof(allElements[0]); i++)
        if(s == allElements[i])
            return true;
    return false;
}

map<int, SwitchInfo> Arch::getSwitchInfo()
{
    if(finalSwitches .size() != 0)
        return finalSwitches;
    map<int, SwitchInfo> result;
    TiXmlElement* switchlist;
    switchlist = rr_graph_root->FirstChildElement("switches");
    for(TiXmlElement* recentSwitch = switchlist->FirstChildElement();
        recentSwitch != NULL;
        recentSwitch = recentSwitch->NextSiblingElement()){
        SwitchInfo si;
        int id = -1;
        string name = "", type = "";
        double Cin = 0, Cout = 0, R = 0, Tdel = 0, buf_size = 0, mux_trans_size = 0;
        for(TiXmlAttribute* recentAttr = recentSwitch->FirstAttribute();
            recentAttr != NULL;
            recentAttr = recentAttr->Next()){
            string attrName = recentAttr->Name();
            auto rawValue = recentAttr->Value();
            if(attrName == "id"){
                int idValue = atoi(rawValue);
                id = idValue;
            }
            else if(attrName == "name")
                name = rawValue;
            else if(attrName == "type")
                type = rawValue;
        }
        //Timing info save
        TiXmlElement* timing = recentSwitch->FirstChildElement();
        if(timing != NULL && timing->FirstAttribute() != NULL){
            for(TiXmlAttribute* recentAttr = timing->FirstAttribute();
                recentAttr != NULL;
                recentAttr = recentAttr->Next()){
                string attrName = recentAttr->Name();
                double value = atof(recentAttr->Value());
                if(attrName == "Cin")
                    Cin = value;
                else if(attrName == "Cout")
                    Cout = value;
                else if(attrName == "Tdel")
                    Tdel = value;
                else if(attrName == "R")
                    R = value;
            }
        }
        //Size info save
        if(timing != NULL && timing->NextSiblingElement() != NULL){
            TiXmlElement* size = timing->NextSiblingElement();
            for(TiXmlAttribute* recentAttr = size->FirstAttribute();
                recentAttr != NULL;
                recentAttr = recentAttr->Next()){
                string attrName = recentAttr->Name();
                double value = atof(recentAttr->Value());
                if(attrName == "buf_size")
                    buf_size = value;
                else if(attrName == "mux_trans_size")
                    mux_trans_size = value;
            }
        }

        si = SwitchInfo(id, type, name, R, Cout, Cin, Tdel, mux_trans_size, buf_size);
        result[id] = si;
    }
    // if(fileType == "EArch"){
    //     switchlist = root->FirstChildElement("switchlist");
    //     int times = 0;
    //     for(auto recentSwitch = switchlist->FirstChildElement("switch");
    //         recentSwitch != NULL;
    //         recentSwitch = recentSwitch->NextSiblingElement("switch")){
    //         times++;
    //         if(times > 2){
    //             Tool::error("Error: Arch getSwitchRawInfo get more than 2 types of switches.");
    //             return result;
    //         }
    //         string switchRawInfo = "";;
    //         for(auto recentAttr = recentSwitch->FirstAttribute();
    //             recentAttr != NULL;
    //             recentAttr = recentAttr->Next()){
    //             if(switchRawInfo == "")
    //                 switchRawInfo += recentAttr->Value();
    //             else{
    //                 switchRawInfo += " ";
    //                 switchRawInfo += recentAttr->Value();
    //             }
    //         }
    //         result.push_back(switchRawInfo);
    //     }
    // }
    //get switch from rr_graph.xml
   
    finalSwitches = result;
    return finalSwitches;
}

TileNode Arch::getTileNode(int id)
{
    return tileNodes[id];
}

vector<TileEdge> Arch::getTileEdges(int sourceID)
{
    return tileEdges[sourceID];
}

int Arch::getIONum()
{
    return ioNum;
}

vector<int> Arch::getRRUsed(){
    vector<int> result;
    for(const auto& pair : tileNodes)
        result.push_back(pair.first);
    return result;
}

string Arch::getArchPath(){
    return archPath;
}

// double Arch::getDelay(string nameSequence, string leftName, string input, string output, string targetType, rapidxml::xml_node<>* recentNode){
//     int end = leftName.find('.');
//     string name;    //the name of model or pb_type
//     string type;    //Mode or pb_type
//     double delay = 0;
//     xml_node<>* targetNode;
//     xml_node<>* combineDelayNode;
//     xml_node<>* node;

//     isSet();
//     if(end == string::npos)
//         name = leftName;
//     else{
//         name = leftName.substr(0, end);
//         leftName = leftName.substr(end + 1, leftName.size() - end - 1);
//     }

//     //Get Location
//     for(node = recentNode->first_node(); node != NULL; node = node->next_sibling()){
//         string nodeName = node->first_attribute()->value();
//         string nodeType = node->name();
//         if(nodeName == name && nodeType == targetType){
//             targetNode = node;
//             if(leftName != "")
//                 return getDelay(nameSequence, leftName, input, output, type, targetNode);
//         }
//     }

//     //Calculate Delay
//     type = targetNode->name();
//     combineDelayNode = targetNode->first_node("interconnect");
//     for(xml_node<>* cnode = combineDelayNode->first_node();
//         cnode != NULL;
//         cnode = cnode->next_sibling()){
//         //Travelsal attributes
//         bool isInputMatch = 0;
//         bool isOutputMatch = 0;
//         for(xml_attribute<>* anode = cnode->first_attribute();
//             anode != NULL;
//             anode = anode->next_attribute()){
//             /////////////The matching of input and output need to be more detail.
//             if(anode->name() == "input" && anode->value() == input)
//                 isInputMatch = 1;
//             else if(anode->name() == "output" && anode->value() == output)
//                 isOutputMatch = 1;
//             if(isInputMatch && isOutputMatch)
//                 break;
//         }

//         //Save info
//         if(isInputMatch && isOutputMatch){
//              //Direct: no time save
//             if(cnode->name() == "direct"){
//                 delayV.push_back(Delay(nameSequence, 0, input, output, "direct"));
//                 return 0;
//             }
//         }
//     }
//     Tool::error("Error: Arch cannot found corresponding delay info.");
//     exit(1);
// }