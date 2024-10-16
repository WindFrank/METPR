#pragma once
#include "../EDAExecuter/VTRDrive.h"
#include "../Arch.h"
#include "../EDAExecuter/PlaceAnalyzer.h"
#include "../EDAExecuter/RouteAnalyzer.h"
#include "../EDAExecuter/TimingAnalyzer.h"
#include "MREntity.h"

#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <unordered_set>

using namespace std;

class MRConsistLayout : public MREntity{
private:
    VTRDrive vtr;
    string MRName = "MRConsistLayout";
    string treatProgram;
    string treeHDLPath;
    string productPath;
    vector<string> programs;
    Arch arch;
    vector<int> treatRRUsed;
    string recentTestName;
    PlaceAnalyzer treatPA;
    RouteAnalyzer treatRA = RouteAnalyzer();
    
public:
    int verify(int seq, bool& onceVerify, string& errorReport, int& caseNum, string defectCaseDirectory);
    string getMRName();
    MRConsistLayout(VTRDrive& vtr, string programName, string treeHDLPath, vector<string> allPrograms, Arch arch) :
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
    string getRecentTestName();
};