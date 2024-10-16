#include "MRCommonGraphConsist.h"
#include "../Tool.h"

int MRCommonGraphConsist::verify(int seq, bool& onceVerify, string& errorReport, int& caseNum, string defectCaseDirectory){
    if(treatRRUsed.size() == 0){
        Tool::logMessage("Warning: State -1. MRCommonGraphConsist found program " + treatProgram + " has no rr used. Maybe it has dead code.");
        return -1;
    }
    Tool::logMessage("MRCommonGraphConsist handle treat program: " + treatProgram);
    //Random find another verilog exists
    vector<string> recentVerilog;
    vector<string> usedVerilog;
    Tool::getAllFilefromDir(productPath, recentVerilog);
    string anotherProgram = "";
    //find a separeted program
    int repeatTimes = 0;   //if repeateTimes >= 10, jump out of the recent loop; if caseNum >= 5, jump out of the recent loop
    int recentCaseNum = 0;
    while(repeatTimes < 10 && recentCaseNum < 5){
        Tool::logMessage("Test " + to_string(seq) + ": common graph consist");
        Tool::logMessage("Recent times: " + to_string(recentCaseNum + 1) + ".");
        int programsLen = programs.size();
        if(programsLen == 1){
            Tool::logMessage("State 2, seq " + to_string(seq) + "MRCommonGraphConsist failed to find a separeted program with " + treatProgram + " .");
            onceVerify = true;
            return 2;
        }
        bool isUsed = find(usedVerilog.begin(), usedVerilog.end(), anotherProgram) != usedVerilog.end();
        int findRepeatTimes = -1;
        while((anotherProgram == "" || anotherProgram == treatProgram || isUsed) && findRepeatTimes < 50){
            findRepeatTimes++;
            if(!isUsed && anotherProgram != "")
                usedVerilog.push_back(anotherProgram);
            int anotherIndex = Tool::getRandomfromClosed(0, programs.size() - 1);
            anotherProgram = programs[anotherIndex];
            isUsed = find(usedVerilog.begin(), usedVerilog.end(), anotherProgram) != usedVerilog.end();
        }
        if(findRepeatTimes >= 10){
            Tool::logMessage("State 2. Verilog " + treatProgram + " found more than 10 times used programs, skip");
            return 2;
        }
        usedVerilog.push_back(anotherProgram);

        //Execute another program
        string newProgramContentVTRPath = productPath +"/" + anotherProgram;
        if(find(recentVerilog.begin(), recentVerilog.end(), newProgramContentVTRPath) == recentVerilog.end()){
            Tool::createDirectory(newProgramContentVTRPath);
            string anotherProgramPath = treeHDLPath + "/" + anotherProgram + ".v";
            string isSucc = vtr.executeRunFlow(anotherProgramPath, arch.getArchPath(), newProgramContentVTRPath, anotherProgram);
            if(isSucc != "success"){
                recentTestName = "VTR_crash_" + anotherProgram;
                errorReport += "State 0: Error program detect";
                errorReport += "Program path: " + newProgramContentVTRPath + "/" + anotherProgram + ".v\n";
                Tool::logMessage(errorReport);
                string errorProductPath = "singleVerilogError";
                Tool::createDirectory(errorProductPath);
                string cp_r_comand = "cp -r " + newProgramContentVTRPath + " " + errorProductPath + ";rm -r " + newProgramContentVTRPath + ";rm " + anotherProgramPath;
                system(cp_r_comand.c_str());
                caseNum++;
                recentCaseNum++;
                return 3;
            }
        }
        recentVerilog.push_back(newProgramContentVTRPath);
        
        //Judgement of PlaceInfo
        PlaceAnalyzer anotherPA(treeHDLPath + "/product/" + anotherProgram + "/" + anotherProgram + ".place");
        RouteAnalyzer anotherRA = RouteAnalyzer();
        anotherRA.loadRoute(treeHDLPath + "/product/" + anotherProgram + "/" + anotherProgram + ".route");
        vector<int> anotherRRUsed = anotherRA.getRoutesRRUsed();
        for(auto it = programs.begin(); it < programs.end();){  //Delete selected program to avoid to be selected again.
            if(*it == anotherProgram){
                programs.erase(it++);
                break;
            }
            else
                it++;
        }
        caseNum++;
        recentCaseNum++;
        onceVerify = true;
        if(anotherRRUsed.size() == 0){
            //if rrused has duplicated
            Tool::logMessage("MRCommonGraphConsist finds " + anotherProgram + " has no rr used, which doesn't satisfy condition, skip it.\n");
            repeatTimes++;
        }
        else{   //Common graph consist test begin
            string productPathTreatProgram = productPath + "/" + treatProgram;
            string productPathAnotherProgram = productPath + "/" + anotherProgram;
            string blifPathTreatProgram = productPathTreatProgram + "/" + treatProgram + ".pre-vpr.blif";
            string blifPathAnotherProgram = productPathAnotherProgram + "/" + anotherProgram + ".pre-vpr.blif";
            string placePathTreatProgram = productPathTreatProgram + "/" + treatProgram + ".place";
            string placePathAnotherProgram = productPathAnotherProgram + "/" + anotherProgram + ".place";
            recentTestName = Tool::getRecentTime() + "_" + treatProgram + "_with_" + anotherProgram;
            string testDirectory = treeHDLPath + "/Test/test" + to_string(seq) + "/test_consistLayout/" + recentTestName;
            Tool::createDirectory(treeHDLPath + "/Test");
            Tool::createDirectory(treeHDLPath + "/Test/test" + to_string(seq));
            Tool::createDirectory(treeHDLPath + "/Test/test" + to_string(seq)  + "/test_consistLayout");
            Tool::createDirectory(testDirectory);

            BlifAnalyzer treatBA(blifPathTreatProgram);
            BlifAnalyzer anotherBA(blifPathAnotherProgram);

            string treatGraph = treatBA.toGraph();
            string anotherGraph = anotherBA.toGraph();

            pair<string, string> commonPair = Tool::getMaxSubGraph(treatGraph, anotherGraph);
            string treatCommonGraph = commonPair.first;
            string anotherCommonGraph = commonPair.second;

            BlifAnalyzer modifyTreatBA(treatBA);
            BlifAnalyzer modifyAnotherBA(anotherBA);

            bool isTreatSucc = modifyTreatBA.commonGraphChange(treatCommonGraph);
            bool isAnotherSucc = modifyAnotherBA.commonGraphChange(anotherCommonGraph);
            if(!isTreatSucc || !isAnotherSucc){
                Tool::logMessage("MRCommonGraphConsist change common graph failed, skip it.\n");
                repeatTimes++;
            }
            repeatTimes = 0;
            Tool::createDirectory(testDirectory + "/" + treatProgram);
            Tool::createDirectory(testDirectory + "/" + anotherProgram);
            string modifyTreatBlifContent = modifyTreatBA.getBlifContent();
            string modifyAnotherBlifContent = modifyAnotherBA.getBlifContent();
            string treatModifyBlifPath = testDirectory + "/" + treatProgram + "/" + treatProgram + ".pre-vpr.blif";
            string anotherModifyBlifPath = testDirectory + "/" + anotherProgram + "/" + anotherProgram + ".pre-vpr.blif";
            Tool::fileWrite(treatModifyBlifPath, modifyTreatBlifContent);
            Tool::fileWrite(anotherModifyBlifPath, modifyAnotherBlifContent);

            string treatLog = vtr.runBlif(treatModifyBlifPath, arch.getArchPath(), testDirectory + "/" + treatProgram + ".pin_fixed.place", testDirectory + "/" + treatProgram);
            if(treatLog == "false"){
                recentTestName = "VTR_common_graph_crash_" + recentTestName;
                errorReport += "State 0: Error program detect: " + treatModifyBlifPath + "\n";
                Tool::logMessage(errorReport);
                Tool::createDirectory("DefectTestCases");
                //copy test cases
                Drive::copyTestCases(productPathTreatProgram + "/" + treatProgram + ".blif", productPathAnotherProgram + "/" + anotherProgram + ".blif",
                                        testDirectory, defectCaseDirectory);
                return 3;
            }

            string anotherLog = vtr.runBlif(anotherModifyBlifPath, arch.getArchPath(), testDirectory + "/" + anotherProgram + ".pin_fixed.place", testDirectory + "/" + anotherProgram);
            if(anotherLog == "false"){
                recentTestName = "VTR_common_graph_crash_" + recentTestName;
                errorReport += "State 0: Error program detect: " + anotherModifyBlifPath + "\n";
                Tool::logMessage(errorReport);
                Tool::createDirectory("DefectTestCases");
                //copy test cases
                Drive::copyTestCases(productPathTreatProgram + "/" + treatProgram + ".blif", productPathAnotherProgram + "/" + anotherProgram + ".blif",
                                        testDirectory, defectCaseDirectory);
                return 3;
            }

            //Get delay of all critical path and area and compare
            DelayandArea modifyTreatDandA = vtr.getCritDelayandAreafromLog(testDirectory + "/" + treatProgram + "/vpr_stdout.log");
            DelayandArea modifyAnotherDandA = vtr.getCritDelayandAreafromLog(testDirectory + "/" + anotherProgram + "/vpr_stdout.log");

            if((modifyTreatDandA.delay < modifyAnotherDandA.delay && modifyTreatDandA.wireArea <= modifyAnotherDandA.wireArea)
                || (modifyTreatDandA.delay > modifyTreatDandA.delay && modifyTreatDandA.wireArea >= modifyTreatDandA.wireArea)){
                //One defect
                errorReport += "MRCommonGraphConsist Defect: common graph case are not consist on their delay and wire area.\n";
                errorReport += "Program 1: " + productPathTreatProgram + ".\n";
                errorReport += "Common Delay: " + to_string(modifyTreatDandA.delay) + ".\n";
                errorReport += "Common wire area: " + to_string(modifyTreatDandA.wireArea) + ".\n\n";
                errorReport += "Program 2: " + productPathAnotherProgram + ".\n";
                errorReport += "Common Delay: " + to_string(modifyAnotherDandA.delay) + ".\n";
                errorReport += "Common wire area: " + to_string(modifyAnotherDandA.wireArea) + ".\n\n";
                errorReport += "Test directory: " + testDirectory + "\n";
                Tool::logMessage(errorReport);
                Tool::fileWrite(testDirectory + "/test.rpt", errorReport);
                Tool::createDirectory("DefectTestCases");
                //copy test cases
                Drive::copyTestCases(productPathTreatProgram + "/" + treatProgram + ".pre-vpr.blif", productPathAnotherProgram + "/" + anotherProgram + ".pre-vpr.blif",
                                        testDirectory, defectCaseDirectory);
                return 0;
            }
            else{
                //normal
                string testReport = "";
                testReport += "MRCommonGraphConsist Defect: common graph case are not consist on their delay and wire area.\n";
                testReport += "Program 1: " + productPathTreatProgram + ".\n";
                testReport += "Common Delay: " + to_string(modifyTreatDandA.delay) + ".\n";
                testReport += "Common wire area: " + to_string(modifyTreatDandA.wireArea) + ".\n\n";
                testReport += "Program 2: " + productPathAnotherProgram + ".\n";
                testReport += "Common Delay: " + to_string(modifyAnotherDandA.delay) + ".\n";
                testReport += "Common wire area: " + to_string(modifyAnotherDandA.wireArea) + ".\n\n";
                testReport += "Test directory: " + testDirectory + "\n";
                Tool::logMessage("State 1: MRConsistLayout finds no defect.");
                Tool::logMessage("Program 1: " + productPathTreatProgram + ".");
                Tool::logMessage("Program 2: " + productPathAnotherProgram + ".\n");
                Tool::fileWrite(testDirectory + "/test.rpt", testReport);
            }
        }
        anotherProgram = "";
    }

    //find more than 10 programs: skip directly
    if(repeatTimes >= 10){
        Tool::logMessage("State 2. Verilog " + treatProgram + " found more than 10 unsepareted programs, skip");
        return 2;
    }
    else if(recentCaseNum >= 5){
        Tool::logMessage("State 2. Verilog " + treatProgram + " found more than 5 programs but not violate MRConsistLayout, skip");
        return 2;
    }

    return 1;
}

bool MRCommonGraphConsist::isThisMR(string tag){
    if(tag == MRName)
        return true;
    return false;
}

string MRCommonGraphConsist::getRecentTestName(){
    return recentTestName;
}

string MRCommonGraphConsist::getMRName(){
    return MRName;
}


