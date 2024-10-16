#include "MRConsistLayout.h"
#include "../Tool.h"

int MRConsistLayout::verify(int seq, bool& onceVerify, string& errorReport, int& caseNum, string defectCaseDirectory){
    if(treatRRUsed.size() == 0){
        Tool::logMessage("Warning: State -1. MRConsistLayout found program " + treatProgram + " has no rr used. Maybe it has dead code.");
        return -1;
    }
    Tool::logMessage("MRConsistLayout handle treat program: " + treatProgram);
    //Random find another verilog exists
    vector<string> usedVerilog;
    string anotherProgram = "";
    //find a separeted program
    int repeatTimes = 0;   //if repeateTimes >= 10, jump out of the recent loop; if caseNum >= 5, jump out of the recent loop
    int recentCaseNum = 0;
    while(repeatTimes < 10 && recentCaseNum < 5){
        Tool::logMessage("Test " + to_string(seq) + ": consist layout");
        Tool::logMessage("Recent times: " + to_string(recentCaseNum + 1) + ".");
        int programsLen = programs.size();
        if(programsLen == 1){
            Tool::logMessage("State 2, seq " + to_string(seq) + "MRConsistLayout failed to find a separeted program with " + treatProgram + " .");
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

        //debug 8 20
        // anotherProgram = "condition_program_first1590";

        //Execute another program
        string newProgramContentVTRPath = productPath +"/" + anotherProgram;
        if(!Tool::isDirectoryExists(newProgramContentVTRPath)){
            Tool::createDirectory(newProgramContentVTRPath);
            string anotherProgramPath = treeHDLPath + "/" + anotherProgram + ".v";
            string isSucc = vtr.executeRunFlow(anotherProgramPath, arch.getArchPath(), newProgramContentVTRPath, anotherProgram);
            if(isSucc != "success"){
                recentTestName = "VTR_crash_" + anotherProgram;
                errorReport += "State 0: Error program detect";
                errorReport += "Program path: " + newProgramContentVTRPath + "/" + anotherProgram + ".v\n";
                Tool::logMessage(errorReport);
                string errorProductPath = "singleVerilogError/singleVerilogError_" + Tool::getLogFileTime();
                Tool::createDirectory(errorProductPath);
                
                if(Tool::isFileExists(newProgramContentVTRPath)){
                    string cp_r_comand = "cp -r " + newProgramContentVTRPath + " " + errorProductPath + ";rm -r " + newProgramContentVTRPath + ";rm " + anotherProgramPath;
                    system(cp_r_comand.c_str());
                }
                else
                    errorReport += ". File lost.";
                caseNum++;
                recentCaseNum++;
                return 3;
            }
        }
        else{
            Tool::logMessage("Directory exist, waiting for report_timing.rpt generated.");
            if(!Tool::isFileExists(newProgramContentVTRPath + "/report_timing.rpt")){
                recentTestName = "VTR_crash_" + anotherProgram;
                errorReport += "State 0: Error program detect, product file lost.";
                errorReport += "Program path: " + newProgramContentVTRPath + "/" + anotherProgram + ".v\n";
                Tool::logMessage(errorReport);
                caseNum++;
                return 3;
            }
            Tool::logMessage("report_timing.rpt got, all file generated.");
        }
        
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
        bool isLocDuplicate = treatPA.hasDuplicateLoc(anotherPA);
        onceVerify = true;
        if(isLocDuplicate || Tool::same_vec(treatRRUsed, anotherRRUsed)){
            //if rrused has duplicated
            Tool::logMessage("MRConsistLayout finds " + anotherProgram + " not satisfy condition with " + treatProgram + ", skip it.\n");
            repeatTimes++;
        }
        else{   //Find two separeted layout: start testing
            //Combined layout
            repeatTimes = 0;
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

            //pin fixed
            string combinePinFixed = VTRDrive::pinCommandFixed(placePathTreatProgram, placePathAnotherProgram, testDirectory);
            //blif combined
            string combineBlif = VTRDrive::blifCom(blifPathTreatProgram, blifPathAnotherProgram, testDirectory);
            //Execute pack, place and route step on the combination of two programs' .blif file.
            string comLog = vtr.runBlif(testDirectory + "/" + combineBlif, arch.getArchPath(), testDirectory + "/" + combinePinFixed, testDirectory);
            if(comLog == "false"){
                recentTestName = "VTR_com_crash_" + recentTestName;
                errorReport += "State 0: Error program detect: " + testDirectory + "\n";
                Tool::logMessage(errorReport);
                Tool::createDirectory("DefectTestCases");
                //copy test cases
                Drive::copyTestCases(productPathTreatProgram + "/" + treatProgram + ".blif", productPathAnotherProgram + "/" + anotherProgram + ".blif",
                                        testDirectory, defectCaseDirectory);
                return 3;
            }

            //Get delay of all critical path and area and compare
            double deadRatio = 0.1;
            DelayandArea treatDandA = vtr.getCritDelayandAreafromLog(productPathTreatProgram + "/vpr_stdout.log");
            DelayandArea anotherDandA = vtr.getCritDelayandAreafromLog(productPathAnotherProgram + "/vpr_stdout.log");
            DelayandArea comDandA = vtr.getCritDelayandAreafromLog(testDirectory + "/vpr_stdout.log");
            double separeteDelay = max(treatDandA.delay, anotherDandA.delay);
            double separeteBlockArea = treatDandA.blockArea + anotherDandA.blockArea;
            double separeteWireArea = treatDandA.wireArea + anotherDandA.wireArea;
            TimingAnalyzer treatTA; treatTA.loadTimingRpt(productPath + "/" + treatProgram + "/report_timing.rpt");
            TimingAnalyzer anotherTA; anotherTA.loadTimingRpt(productPath + "/" + anotherProgram + "/report_timing.rpt");
            double treatSlack = treatTA.worstSlack;
            double anotherSlack = anotherTA.worstSlack;
            TimingAnalyzer comTA; comTA.loadTimingRpt(testDirectory + "/report_timing.rpt");
            double comSlack = comTA.worstSlack;
            double comparePreferRatio = (separeteDelay - comDandA.delay) / separeteDelay;
            double wirePreferRatio = (treatDandA.wireArea + anotherDandA.wireArea - comDandA.wireArea) / (treatDandA.wireArea + anotherDandA.wireArea); 
            double slackDelta = min(treatSlack, anotherSlack) - comSlack;
            if(abs(comparePreferRatio) > deadRatio || abs(wirePreferRatio) > deadRatio || abs(slackDelta) > 10 * deadRatio){
                //One defect
                errorReport += "MRConsistLayout Defect: compare prefer ratio too large.\n";
                errorReport += "Program 1: " + productPathTreatProgram + "\n";
                errorReport += "Delay: " + to_string(treatDandA.delay) + "\n";
                errorReport += "Wire length: " + to_string(treatDandA.wireArea) + "\n";
                errorReport += "Worst slack: " + to_string(treatSlack) + "\n";
                errorReport += "Program 2: " + productPathAnotherProgram + "\n";
                errorReport += "Delay: " + to_string(anotherDandA.delay) + "\n";
                errorReport += "Wire length: " + to_string(anotherDandA.wireArea) + "\n";
                errorReport += "Worst slack: " + to_string(anotherSlack) + "\n";
                errorReport += "Wire length sum: " + to_string(treatDandA.wireArea + anotherDandA.wireArea) + "\n";
                errorReport += "Combine " + treatProgram + " and " + anotherProgram + "\n";
                errorReport += "Delay: " + to_string(comDandA.delay) + "\n";
                errorReport += "Wire length: " + to_string(comDandA.wireArea) + "\n";
                errorReport += "Worst slack: " + to_string(comSlack) + "\n";
                errorReport += "Compare prefer ratio: " + to_string(comparePreferRatio) + "\n";
                errorReport += "Wire prefer ratio: " + to_string(wirePreferRatio) + "\n";
                errorReport += "Slack delta: " + to_string(slackDelta) + "\n";
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
                testReport = "State 1. MRConsistLayout finds no defect.\n";
                testReport += "Program 1: " + productPathTreatProgram + "\n";
                testReport += "Delay: " + to_string(treatDandA.delay) + "\n";
                testReport += "Wire length: " + to_string(treatDandA.wireArea) + "\n";
                testReport += "Worst slack: " + to_string(treatSlack) + "\n";
                testReport += "Program 2: " + productPathAnotherProgram + "\n";
                testReport += "Delay: " + to_string(anotherDandA.delay) + "\n";
                testReport += "Wire length: " + to_string(anotherDandA.wireArea) + "\n";
                testReport += "Worst slack: " + to_string(anotherSlack) + "\n";
                testReport += "Wire length sum: " + to_string(treatDandA.wireArea + anotherDandA.wireArea) + "\n";
                testReport += "Combine " + treatProgram + " and " + anotherProgram + "\n";
                testReport += "Delay: " + to_string(comDandA.delay) + "\n";
                testReport += "Wire length: " + to_string(comDandA.wireArea) + "\n";
                testReport += "Worst slack: " + to_string(comSlack) + "\n";
                testReport += "Compare prefer ratio: " + to_string(comparePreferRatio) + "\n";
                testReport += "Wire prefer ratio: " + to_string(wirePreferRatio) + "\n";
                testReport += "Slack delta: " + to_string(slackDelta) + "\n";
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

bool MRConsistLayout::isThisMR(string tag){
    if(tag == MRName)
        return true;
    return false;
}

string MRConsistLayout::getRecentTestName(){
    return recentTestName;
}

string MRConsistLayout::getMRName(){
    return MRName;
}


