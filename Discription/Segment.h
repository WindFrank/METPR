#pragma once

#include <iostream>
#include <string>

using namespace std;

class Segment{
public:
    const char* segmentType = new char[20];
    int nodeID = -1;
    pair<int, int> start;
    pair<int, int> end;
    int switchID;
    int track;
    double CTotal = -0.1;
    double R = -1;

    Segment();
    Segment(const char* segmentType, int nodeID, 
            pair<int, int> start,
            pair<int, int> end,
            int switchID,
            int track,
            double CTotal,
            double R):
            segmentType(segmentType), nodeID(nodeID),
            start(start), end(end), switchID(switchID),
            track(track), CTotal(CTotal), R(R){};
};
