#pragma once

#include "Segment.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class RouteInfo{
public:
    string netName;
    string id;
    pair<int, int> sourceLoc;
    int sourceNodeID;
    int sourcePtcPad;
    string sourcePort;
    int sourcePin;
    int sourcePinNodeID;
    vector<Segment*> segments;
    pair<int, int> sinkLoc;
    int sinkNodeID;
    int sinkPtcPad;
    string sinkPort;
    int sinkPin;
    int sinkPinNodeID;
    double delay = -0.1;
    int firstSwitchId = -1;

    RouteInfo();
    RouteInfo(RouteInfo* anotherRoute) :
        netName(anotherRoute->netName),
        id(anotherRoute->id),
        sourceLoc(anotherRoute->sourceLoc),
        sourceNodeID(anotherRoute->sourceNodeID),
        sourcePtcPad(anotherRoute->sourcePtcPad),
        sourcePort(anotherRoute->sourcePort),
        sourcePin(anotherRoute->sourcePin),
        sourcePinNodeID(anotherRoute->sourcePinNodeID),
        segments(anotherRoute->segments),
        sinkLoc(anotherRoute->sinkLoc),
        sinkNodeID(anotherRoute->sinkNodeID),
        sinkPtcPad(anotherRoute->sinkPtcPad),
        sinkPort(anotherRoute->sinkPort),
        sinkPin(anotherRoute->sinkPin),
        sinkPinNodeID(anotherRoute->sinkPinNodeID),
        delay(anotherRoute->delay),
        firstSwitchId(anotherRoute->firstSwitchId){}
};