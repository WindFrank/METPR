#include "PlaceAnalyzer.h"
#include "../Tool.h"

PlaceAnalyzer::PlaceAnalyzer(){}

PlaceAnalyzer::PlaceAnalyzer(string filename){
    this->filename = filename;
    getSize();
    setPlaceInfo();
}

PlaceAnalyzer::~PlaceAnalyzer(){

}

pair<int, int> PlaceAnalyzer::getSize(){
    string buf;

    if(filename == ""){
        Tool::error("Error: PlaceAnalyzer unset filename.");
        exit(1);
    }

    if(size.first != 0)
        return size;
    //Find size from file
    ifstream ifs;
    ifs.open(filename, ios::in);
    if(!ifs.is_open()){
        Tool::error("Error: PlaceAnalyzer open " + filename + " failed.");
        exit(1);
    }
    while(getline(ifs, buf)){
        if(buf.find("x") != string::npos){
            vector<string> line = Tool::split(buf, ' ');
            int row = atoi(line[2].c_str());
            int col = atoi(line[4].c_str());
            size.first = row;
            size.second = col;
            ifs.close();
            return size;
        }
    }
    ifs.close();
    return pair<int, int>(0, 0);
}

bool PlaceAnalyzer::setPlaceInfo(){
    string buf;
    int i = 5;

    if(filename == ""){
        Tool::error("Error: PlaceAnalyzer unset filename.");
        exit(1);
    }

    if(mapSP.size() != 0)
        return true;
    //Find size from file
    ifstream ifs;
    ifs.open(filename, ios::in);
    if(!ifs.is_open()){
        Tool::error("Error: PlaceAnalyzer open " + filename + " failed.");
        exit(1);
    }
    while(0 < i--)
        getline(ifs, buf);
    while(getline(ifs, buf)){
        string preBuf  = "";
        int tagFlag = 0;
        for(int si = 0; si < buf.size(); si++){
            char c = buf[si];
            if(c == '\t'){
                if(!tagFlag){
                    tagFlag = 1;
                }
                else
                    continue;
            }
            else
                tagFlag = 0;
            preBuf += c;
        }
        vector<string> line = Tool::split(preBuf, '\t');
        PlaceInfo p;
        p.blockName = line[0];
        p.x = atoi(line[1].c_str());
        p.y = atoi(line[2].c_str());
        p.subblk = atoi(line[3].c_str());
        string blockNum = line[4].substr(1, line[4].size() - 1);
        p.blockNum = atoi(blockNum.c_str());
        mapSP[p.blockName] = p;
        allPlaceInfo.push_back(p);
        if(ifIO(p))
            ioPlaceInfo.push_back(p);
    }
    ifs.close();
    return true;
}

PlaceInfo PlaceAnalyzer::getPlaceInfo(string blockName){
    return mapSP[blockName];
}

bool PlaceAnalyzer::hasDuplicateLoc(PlaceAnalyzer& anotherPA){
    vector<PlaceInfo> anotherPlaceInfos = anotherPA.getAllPlaceInfo();
    for(auto it = allPlaceInfo.begin(); it < allPlaceInfo.end(); it++){
        PlaceInfo recentP = *it;
        for(auto item = anotherPlaceInfos.begin(); item < anotherPlaceInfos.end(); item++){
            PlaceInfo anRecentP = *item;
            if(recentP.x == anRecentP.x && recentP.y == anRecentP.y && recentP.subblk == anRecentP.subblk)
                return true;
        }
    }
    return false;
}

vector<PlaceInfo> PlaceAnalyzer::getAllPlaceInfo(){
    return allPlaceInfo;
}

bool PlaceAnalyzer::hasPinDuplicate(PlaceAnalyzer& anotherPA){
    vector<PlaceInfo> anotherPlaceInfos = anotherPA.getIOPlaceInfo();
    for(auto it = ioPlaceInfo.begin(); it < ioPlaceInfo.end(); it++){
        PlaceInfo recentP = *it;
        for(auto item = anotherPlaceInfos.begin(); item < anotherPlaceInfos.end(); item++){
            PlaceInfo anRecentP = *item;
            if(recentP.x == anRecentP.x && recentP.y == anRecentP.y && recentP.subblk == anRecentP.subblk)
                return true;
        }
    }
    return false;
}

bool PlaceAnalyzer::ifIO(PlaceInfo pi){
    bool isXCorner = pi.x == 0 || pi.x == size.second - 1;
    bool isYCorner = pi.y == 0 || pi.y == size.first - 1;
    return isXCorner || isYCorner;
}

bool PlaceAnalyzer::ifIO(pair<int, int> point){
    return (point.first == 0 || point.first == size.second -1 || point.second == 0 || point.second == size.first - 1);
}

vector<PlaceInfo> PlaceAnalyzer::getIOPlaceInfo(){
    return ioPlaceInfo;
}

// vector<pair<int, int>> PlaceAnalyzer::getAllLocation(){
//     vector<pair<int, int>> result;
//     for(auto it = mapSP.begin(); it != mapSP.end(); it++){
//         result.push_back(pair<int, int>(it->second.x, it->second.y));
//     }
//     return result;
// }

