#pragma once
#include "../EDAExecuter/VTRDrive.h"
#include "../Arch.h"
#include "../EDAExecuter/PlaceAnalyzer.h"
#include "../EDAExecuter/RouteAnalyzer.h"
#include "MREntity.h"
#include "../ActionMutateLead/Evaluate.h"
#include "../ActionMutateLead/VarietyEvaluation.h"

#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <unordered_set>

using namespace std;

struct sameSegment{
    int firstStart = -1;
    int secondStart = -1;
    int firstEnd = -1;
    int secondEnd = -1;
    int distance;
};

class MRPreRoad : public MREntity{
private:
    VTRDrive vtr;
    string MRName = "MRPreRoad";
    string treatProgram;
    string treeHDLPath;
    string productPath;
    vector<string> programs;
    vector<int> treatRRUsed;
    Arch arch;
    PlaceAnalyzer treatPA;
    RouteAnalyzer treatRA = RouteAnalyzer();
    //find the longest distance in first vec between same start and end in two vec.
    sameSegment findPartialLongest(vector<int> firstVec, vector<int> secondVec);
public:
    int verify(int seq);
    string getMRName();
    MRPreRoad(VTRDrive& vtr, string programName, string treeHDLPath, vector<string> allPrograms, Arch arch) :
    vtr(vtr),
    treatProgram(programName),
    treeHDLPath(treeHDLPath),
    programs(allPrograms),
    productPath(treeHDLPath + "/product"),
    arch(arch){
        treatPA = PlaceAnalyzer(treeHDLPath + "/product/" + treatProgram + "/" + treatProgram + ".place");
        treatRA.loadRoute(treeHDLPath + "/product/" + treatProgram + "/" + treatProgram + ".route");
        treatRRUsed = treatRA.getRoutesRRUsed();
    }
    bool isThisMR(string tag);
};