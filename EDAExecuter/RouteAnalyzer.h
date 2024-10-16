/*
    RouteAnalyzer: get the root information especially delay for FPGA

    We found that the number of switches noted in the arch file is only 2.
    Maybe one is for channel transmission, another one is for input box.
*/

#pragma once

#include "../Discription/BlockNode.h"
#include "../Discription/SwitchInfo.h"
#include "../Discription/RouteInfo.h"
#include "PlaceAnalyzer.h"

#include <map>

// struct SwitchInfo{
//     const char* type = new char[20];
//     const char* name = new char[20];
//     const char* buf_size = new char[20];    //It maybe a double of string, use string to save.
//     double R;
//     double Cout;
//     double Cin;
//     double Tdel;
//     double mux_trans_size;
//     double delay = -1.0;
// };

// struct Segment{
//     const char* segmentType = new char[20];
//     int nodeID = -1;
//     pair<int, int> start;
//     pair<int, int> end;
//     int switchNum;
//     int track;
// };

// struct RouteInfo{
//     const char* id = new char[20];
//     pair<int, int> sourceLoc;
//     const char* sourcePort = new char[20];
//     int sourcePin;
//     vector<Segment*>* segments;
//     pair<int, int> sinkLoc;
//     const char* sinkPort = new char[20];
//     int sinkPin;
// };

class RouteAnalyzer{
private:
    map<string, SwitchInfo*> switchlist;
    vector<RouteInfo*> routeList;
    vector<int> rrRouteUsed;
public:
    RouteAnalyzer();
    // void getSwitchInfo(Arch* arch);
    void loadRoute(string filename);
    vector<RouteInfo*> getRouteList();
    //getRouteDelay use the way EArch explained
    static double getRouteDelay(RouteInfo* ri, Arch* arch);
    //get the rr_graph used by router
    vector<int> getRoutesRRUsed();
    //get single route rr node used
    static vector<int> getSingleRouteRRUsed(RouteInfo route);
    //get part delay of a route:
    static double getPartRouteDelay(RouteInfo* ri, Arch* arch, int startRRID, int endRRID);

    RouteInfo* getRouteByIO(string inpad, string outpad);
    // double getSwitchDelay(Arch* arch, int id);
    pair<pair<int, int>, int> getRouteOutPinLoc(string outpad);
};