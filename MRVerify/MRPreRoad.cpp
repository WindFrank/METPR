#include "MRPreRoad.h"
#include "../Tool.h"

int MRPreRoad::verify(int seq){
    if(treatRRUsed.size() == 0){
        Tool::logMessage("Warning: State -1. MRPreRoad found program " + treatProgram + " has no rr used. Maybe it has dead code.");
        return -1;
    }
    Tool::logMessage("MRPreRoad handle treat program: " + treatProgram);
    int returnFLag = 1;

    string treatProgramPath = treeHDLPath + "/" + treatProgram + ".v";
    string treatGraph = VarietyEvaluation::fromVtoFormularG(treatProgramPath, treatProgram);
    vector<RouteInfo*> treatRoutes = treatRA.getRouteList();
    //Random find another verilog exists. Select 5 program having product. If not enough, select all existed
    vector<string> recentVerilog;
    Tool::getAllFilefromDir(productPath, recentVerilog);
    recentVerilog.erase(find(recentVerilog.begin(), recentVerilog.end(), productPath + "/" + treatProgram));
    vector<string> candiProductPath;
    vector<double> candiDistance;
    if(recentVerilog.size() == 0){
        Tool::logMessage("State2: No pre program producted, skip.");
        return 2;
    }
    else if(recentVerilog.size() <= 5)
        candiProductPath.insert(candiProductPath.end(), recentVerilog.begin(), recentVerilog.end());
    else{
        //random choose 25, choose 5 programs having shortest distance. If not enough, select by ratio.
        int candiTimes = recentVerilog.size() > 25 ? 25 : recentVerilog.size();
        int selectNum = candiTimes / 5;
        for(int i = 0; i < candiTimes; i++){
            int randIndex = Tool::getRandomfromClosed(0, recentVerilog.size());
            string recentProduct = recentVerilog[randIndex];
            string recentName = Tool::findFilefromPath(recentProduct);
            string recentProgramPath = recentProduct + "/" + recentName + ".v";
            string recentGraph = VarietyEvaluation::fromVtoFormularG(recentProgramPath, recentName);
            double recentDistance = Evaluate::graphEditDistance(treatGraph, recentGraph, 30) + Evaluate::getJaccard(treatProgramPath, recentProgramPath, 1);
            auto it = recentVerilog.begin() + randIndex;
            recentVerilog.erase(it);
            //insert productPath and distance lower than exist
            int position = 0;
            for(int j = 0; j < candiProductPath.size(); j++){
                double compareDistance = candiDistance[j];
                if(recentDistance > compareDistance)
                    position++;
                else
                    break;
            }
            candiProductPath.insert(candiProductPath.begin() + position, recentProduct);
            candiDistance.insert(candiDistance.begin() + position, recentDistance);
            if(candiProductPath.size() > selectNum){
                candiProductPath.pop_back();
                candiDistance.pop_back();
            }
        }
    }

    string testDirectory = treeHDLPath + "/Test/test" + to_string(seq) + "/test_preRoad/" + Tool::getRecentTime() + treatProgram;
    Tool::createDirectory(treeHDLPath + "/Test");
    Tool::createDirectory(treeHDLPath + "/Test/test" + to_string(seq));
    Tool::createDirectory(treeHDLPath + "/Test/test" + to_string(seq)  + "/test_preRoad");
    Tool::createDirectory(testDirectory);

    //Route check
    int i = 0;
    for(auto it = candiProductPath.begin(); it < candiProductPath.end(); it++){
        string recentCandiProPath = *it;
        string candiProgramName = Tool::findFilefromPath(recentCandiProPath);
        Tool::logMessage("MRPreRoad: candidate " + to_string(i) + " " + candiProgramName);
        RouteAnalyzer candiRA = RouteAnalyzer();
        candiRA.loadRoute(recentCandiProPath);
        vector<int> candiRRUsed = candiRA.getRoutesRRUsed();
        if(!Tool::same_vec(treatRRUsed, candiRRUsed))
            Tool::logMessage("No collision road resource, skip.");
        else{
            vector<RouteInfo*> candiRoutes = candiRA.getRouteList();
            for(auto it = treatRoutes.begin(); it < treatRoutes.end(); it++){
                RouteInfo recentTreatRoute = *(*it);
                vector<int> recentTreatRRUsed = RouteAnalyzer::getSingleRouteRRUsed(recentTreatRoute);
                struct sameSegment finalSegment;
                for(auto item = candiRoutes.begin(); item < candiRoutes.end(); item++){
                    RouteInfo recentCandiRoute = *item;
                    vector<int> recentCandiRRUsed = RouteAnalyzer::getSingleRouteRRUsed(recentCandiRoute);
                    //furthest road match
                    struct sameSegment treatSegment = findPartialLongest(recentTreatRRUsed, recentCandiRRUsed);
                    struct sameSegment candiSegment = findPartialLongest(recentCandiRRUsed, recentTreatRRUsed);
                    if(treatSegment.distance == -1 && candiSegment.distance == -1)
                        continue;
                    finalSegment = treatSegment;
                    if(treatSegment.distance < candiSegment.distance){
                        finalSegment.firstStart = candiSegment.secondStart;
                        finalSegment.firstEnd = candiSegment.secondEnd;
                        finalSegment.secondStart = candiSegment.firstStart;
                        finalSegment.secondEnd = candiSegment.firstEnd;
                        finalSegment.distance = candiSegment.distance;
                    }
                    //Detect same start and end, judge if they are same roads.
                    bool sameTrace = true;
                    if(finalSegment.firstEnd - finalSegment.firstStart != finalSegment.secondEnd - finalSegment.secondStart){
                        sameTrace = false;
                    }
                    else{
                        int treatIndex = finalSegment.firstStart;
                        int candiIndex = finalSegment.secondStart;
                        while(treatIndex <= finalSegment.firstEnd){
                            if(recentTreatRRUsed[treatIndex] != recentCandiRRUsed[candiIndex]){
                                sameTrace = false;
                                break;
                            }
                            treatIndex++;
                            candiIndex++;
                        }
                    }
                    //Not same trace, check:
                    //1. If the rr wanted occupied by other road. Only check larger one, and if both of them have the same delay, skip. 
                    //2. If not satisfy slack (VTR now no need to consider it).
                    if(!sameTrace){
                        double treatDelay = RouteAnalyzer::getPartRouteDelay(&recentTreatRoute, &arch, finalSegment.firstStart, finalSegment.firstEnd);
                        double candiDelay = RouteAnalyzer::getPartRouteDelay(&recentCandiRoute, &arch, finalSegment.secondStart, finalSegment.secondEnd);
                        if(treatDelay == candiDelay)
                            continue;
                        else{
                            vector<int> shortRRUsed;
                            vector<int> longRRUsed;
                            vector<int> longAllRRused;
                            int shortStart, shortEnd;
                            if(treatDelay > candiDelay){
                                shortRRUsed = recentCandiRRUsed;
                                longRRUsed = recentTreatRRUsed;
                                shortStart = finalSegment.secondStart;
                                shortEnd = finalSegment.secondEnd;
                                longAllRRused = treatRRUsed;
                            }
                            else{
                                shortRRUsed = recentTreatRRUsed;
                                longRRUsed = recentCandiRRUsed;
                                shortStart = finalSegment.firstStart;
                                shortEnd = finalSegment.firstEnd;
                                longAllRRused = candiRRUsed;
                            }
                            for(int index = shortStart + 1; index < shortEnd; index++){
                                int recentRRID = shortRRUsed[index];
                                if(find(longAllRRused.begin(), longAllRRused.end(), recentRRID) == longAllRRused.end()){
                                    //No rr occupied but choose longer path, defect detected.
                                    Tool::logMessage("MRPreRoad defect: long path detected.");
                                    Tool::logMessage("Path 1: " + Tool::arrayToStr(recentTreatRRUsed));
                                    Tool::logMessage("Path 2: " + Tool::arrayToStr(recentCandiRRUsed));
                                    returnFLag = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        i++;
    }
    if(returnFLag == 1)
        Tool::logMessage("State 1: No MRPreRoad violated.");
    return returnFLag;
}

bool MRPreRoad::isThisMR(string tag){
    if(tag == MRName)
        return true;
    return false;
}

sameSegment MRPreRoad::findPartialLongest(vector<int> firstVec, vector<int> secondVec){
    struct sameSegment treatSegment;
    if(!Tool::same_vec(firstVec, secondVec))
        return treatSegment;
    for(int treatIndex = 0; treatIndex < firstVec.size(); treatIndex++){   //treat travel
        int recentTreatRRID = firstVec[treatIndex];
        int candiIndexStart = 0;
        for(int candiIndex = 0; candiIndex < secondVec.size(); candiIndex++){
            int recentCandiRRID = secondVec[candiIndex];
            if(recentTreatRRID == recentCandiRRID){
                candiIndexStart = candiIndex + 1;
                if(treatSegment.firstStart == -1){
                    treatSegment.firstStart = treatIndex;
                    treatSegment.secondStart = candiIndex;
                }
                else if(candiIndex > treatSegment.secondStart){
                    treatSegment.distance = treatIndex - treatSegment.firstStart;
                    treatSegment.firstEnd = treatIndex;
                    treatSegment.secondEnd = candiIndex;
                }                               
            }
        }
    }
    return treatSegment;
}