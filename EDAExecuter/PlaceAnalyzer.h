/*
    PlaceAnalyzer: Get the info from placement file.
*/

#pragma once

#include "../Discription/PlaceInfo.h"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>

using namespace std;

class PlaceAnalyzer{
private:
    string filename;
    pair<int, int> size;
    map<string, PlaceInfo> mapSP;
    vector<PlaceInfo> allPlaceInfo;
    vector<PlaceInfo> ioPlaceInfo;
public:
    PlaceAnalyzer();
    ~PlaceAnalyzer();
    PlaceAnalyzer(string filename);
    pair<int, int> getSize();
    bool setPlaceInfo();
    PlaceInfo getPlaceInfo(string blockName);
    // vector<pair<int, int>> getAllLocation();
    bool hasDuplicateLoc(PlaceAnalyzer& anotherPA);
    /*
        hasPinDuplicate: judge if the two placement use the same pin.
    */
    bool hasPinDuplicate(PlaceAnalyzer& anotherPA);
    /*
        ifIO: judge if the PlaceInfo is io
    */
    bool ifIO(PlaceInfo pi);
    bool ifIO(pair<int, int> point);
    vector<PlaceInfo> getAllPlaceInfo();
    vector<PlaceInfo> getIOPlaceInfo();
};