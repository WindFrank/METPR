#include "RouteAnalyzer.h"
#include "../Tool.h"

RouteAnalyzer::RouteAnalyzer()
{
    routeList = vector<RouteInfo*>();
}

// void RouteAnalyzer::getSwitchInfo(Arch *arch)
// {
//     vector<string> rawInfo = arch->getSwitchRawInfo();
//     for(auto it = rawInfo.begin(); it < rawInfo.end(); it++){
//         vector<string> sv = Tool::split(*it, ' ');
//         SwitchInfo* si = new SwitchInfo();
//         si->type = sv[0].c_str();
//         si->name = sv[1].c_str();
//         si->R = atof(sv[2].c_str());
//         si->Cout = atof(sv[3].c_str());
//         si->Cin = atof(sv[4].c_str());
//         si->Tdel = atof(sv[5].c_str());
//         si->mux_trans_size = atof(sv[6].c_str());
//         si->buf_size = sv[7].c_str();
//         si->delay = -1.0;
//         switchlist[sv[1]] = si;
//     }
// }

void RouteAnalyzer::loadRoute(string filename)
{
    string buf;
    ifstream ifs;
    ifs.open(filename, ios::in);
    if(!ifs.is_open()){
        Tool::error("Error: RouteAnalyzer open " + filename + " failed.");
        exit(1);
    }
    int recentId = 0;
    while(getline(ifs, buf)){
        if(buf.find("Net") != string::npos){
            RouteInfo* route = new RouteInfo();
            RouteInfo* bakRoute;
            string tempNetName = Tool::split(buf, ' ', "clean")[2];
            route->netName = tempNetName.substr(1, tempNetName.size() - 2);
            getline(ifs, buf);
            getline(ifs, buf);
            route->id = to_string(recentId).c_str();
            if(route->id == "72")
                int a = 0;
            bool deleteFrontSeg = false;
            while(buf != "" && buf != "\n"){
                vector<string> recentElements = Tool::split(buf, ' ', "clean");
                if(recentElements[0] == "Block"){//Seq connection, don't set delay
                    
                    break;
                }    
                string elemType = recentElements[2];
                if(elemType == "SOURCE"){
                    route->sourceLoc = Tool::getBracketPair(recentElements[3]);
                    route->sourceNodeID = atoi(recentElements[1].c_str());
                    route->sourcePtcPad = atoi(recentElements[5].c_str());
                }
                else if(elemType == "OPIN"){
                    route->sourcePinNodeID = atoi(recentElements[1].c_str());
                    route->sourcePin = atoi(recentElements[5].c_str());
                    route->sourcePort = recentElements[6].c_str();
                    if(route->sourcePort == "Switch:"){
                        route->sourcePort = "io.inpad[0]";
                        route->firstSwitchId = atoi(recentElements[7].c_str());
                    }
                    else
                        route->firstSwitchId = atoi(recentElements[8].c_str());
                }
                else if(elemType == "CHANX" || elemType == "CHANY"){
                    int nodeID = atoi(recentElements[1].c_str());
                    if(deleteFrontSeg){
                        deleteFrontSeg = false;
                        bool isNew = false;
                        while(true){    //deal with multi-branch net route.
                            if(route->segments.size() == 0){
                                isNew = true;
                                break;
                            }
                            else{ 
                                Segment* recentSegment = route->segments.back();
                                if(recentSegment->nodeID == nodeID){
                                    //Deep copy
                                    recentSegment = new Segment(*route->segments.back());
                                    route->segments.pop_back();
                                    //Reset the Switch
                                    int shortArr = 0;
                                    if(recentElements[4] != "to")
                                        shortArr = -2;
                                    recentSegment->switchID = atoi(recentElements[9 + shortArr].c_str());
                                    route->segments.push_back(recentSegment);
                                    getline(ifs, buf);
                                    break;
                                }
                                else
                                    route->segments.pop_back();
                            }
                        }
                        if(!isNew)
                            continue;
                    }
                    Segment* segment = new Segment();
                    int shortArr = 0;
                    if(recentElements[4] != "to")
                        shortArr = -2;
                    segment->nodeID = nodeID;
                    segment->segmentType = elemType.c_str();
                    segment->start = Tool::getBracketPair(recentElements[3]);
                    if(shortArr != -2)
                        segment->end = Tool::getBracketPair(recentElements[5]);
                    segment->track = atoi(recentElements[7 + shortArr].c_str());
                    segment->switchID = atoi(recentElements[9 + shortArr].c_str());
                    route->segments.push_back(segment);
                }
                else if(elemType == "SINK"){
                    route->sinkLoc = Tool::getBracketPair(recentElements[3]);
                    route->sinkNodeID = atoi(recentElements[1].c_str());
                    route->sinkPtcPad = atoi(recentElements[5].c_str());
                    routeList.push_back(route);
                    route = bakRoute;
                    route->segments = vector<Segment*>(bakRoute->segments);
                }
                else if(elemType == "IPIN"){
                    bakRoute = new RouteInfo(route);
                    route->sinkPinNodeID = atoi(recentElements[1].c_str());
                    route->sinkPin = atoi(recentElements[5].c_str());
                    route->sinkPort = recentElements[6].c_str();
                    if(route->sinkPort == "Switch:")
                        route->sinkPort = "io.outpad[0]";
                    deleteFrontSeg = true;
                }
                getline(ifs, buf);
            }
            recentId++;
        }
    }
    ifs.close();
}

vector<RouteInfo*> RouteAnalyzer::getRouteList()
{
    return routeList;
}

double RouteAnalyzer::getRouteDelay(RouteInfo* ri, Arch* arch)
{
    if(ri->delay != -0.1)
        return ri->delay;

    map<int, SwitchInfo> switches = arch->getSwitchInfo();
    int lastSwitchID = ri->firstSwitchId;
    double allDelay = 0.0;
    for(auto it = ri->segments.begin(); it < ri->segments.end(); it++){
        Segment* segment = *it;
        TileNode segmentNode = arch->getTileNode(segment->nodeID);
        SwitchInfo si = switches[lastSwitchID];
        double recentDelay = si.Tdel + si.R * segmentNode.C + 0.5 * segmentNode.R * segmentNode.C;
        allDelay += recentDelay;
        segment->CTotal = segmentNode.C;
        segment->R = segmentNode.R;
        lastSwitchID = segment->switchID;
    }

    //input connection last switches delay:
    SwitchInfo lastSwitch = switches[lastSwitchID];
    allDelay +=  lastSwitch.Tdel;
    
    ri->delay = allDelay;
    return allDelay;
}

vector<int> RouteAnalyzer::getRoutesRRUsed()
{
    if(rrRouteUsed.size() != 0)
        return rrRouteUsed;
    vector<int> result;
    for(auto it = routeList.begin(); it < routeList.end(); it++){
        RouteInfo* route = *it;
        vector<int> recentRRUsed = getSingleRouteRRUsed(*route);
        result.insert(result.end(), recentRRUsed.begin(), recentRRUsed.end());
    }
    rrRouteUsed = result;
    return result;
}

vector<int> RouteAnalyzer::getSingleRouteRRUsed(RouteInfo route)
{
    vector<int> result;
    result.push_back(route.sourcePinNodeID);
    vector<Segment*> chans = route.segments;
    for(auto it = chans.begin(); it < chans.end(); it++){
        Segment* s = *it;
        result.push_back(s->nodeID);
    }
    result.push_back(route.sinkPinNodeID);
    return result;
}

double RouteAnalyzer::getPartRouteDelay(RouteInfo* ri, Arch* arch, int startRRID, int endRRID){
    map<int, SwitchInfo> switches = arch->getSwitchInfo();
    int lastSwitchID = ri->firstSwitchId;
    double allDelay = 0.0;
    bool startCompute = false;
    bool addLastSwitchTdel = false;
    for(auto it = ri->segments.begin(); it < ri->segments.end(); it++){
        Segment* segment = *it;
        TileNode segmentNode = arch->getTileNode(segment->nodeID);
        if(startCompute || segment->nodeID == startRRID){
            SwitchInfo si = switches[lastSwitchID];
            double recentDelay = si.Tdel + si.R * segmentNode.C + 0.5 * segmentNode.R * segmentNode.C;
            allDelay += recentDelay;
        }
        segment->CTotal = segmentNode.C;
        segment->R = segmentNode.R;
        lastSwitchID = segment->switchID;
        if(segment->nodeID == endRRID)
            startCompute = false;
        if(it + 1 == ri->segments.end())
            addLastSwitchTdel = true;
    }
    if(addLastSwitchTdel){
        //input connection last switches delay:
        SwitchInfo lastSwitch = switches[lastSwitchID];
        allDelay +=  lastSwitch.Tdel;
    }
    
    return allDelay;
}

pair<pair<int, int>, int> RouteAnalyzer::getRouteOutPinLoc(string outpad){
    pair<pair<int, int>, int> result;
    for(RouteInfo* ri : routeList){
        if(outpad == ri->netName){
            result = pair<pair<int, int>, int>(ri->sinkLoc, ri->sinkPtcPad);
            return result;
        }
    }
    return result;
}

// double RouteAnalyzer::getSwitchDelay(Arch* arch, int id)
// {
//     //Get switch delay
//     if(ri->delay < 0){   //First calculate
//         if(ri->name == "0")
//             ri->delay = ri->Tdel + ri->Cout * ri->R;
//         else if(ri->name == "ipin_cblock"){
//             if(type == "in")
//                 ri->delay = ri->Tdel;
//             else
//                 ri->delay = ri->Tdel + ri->Cout * ri->R;
//         }
//     }
//     return ri->delay;
// }