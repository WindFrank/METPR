#include "MRAddRouteConsist.h"
#include "../Tool.h"

int MRAddRouteConsist::verify(int seq, bool& onceVerify, string& errorReport, int& caseNum, string defectCaseDirectory){
    if(treatRRUsed.size() == 0){
        Tool::logMessage("Warning: State -1. MRAddRouteConsist found program " + treatProgram + " has no rr used. Maybe it has dead code.");
        return -1;
    }
    else if(treatTAGlobal.critPathDelay == 0){
        Tool::logMessage("Warning: State -1. MRAddRouteConsist found program " + treatProgram + " has no delay.");
        return -1;
    }
    Tool::logMessage("MRAddRouteConsist handle treat program: " + treatProgram);
    //Random find another verilog exists
    vector<string> usedVerilog;
    TimingAnalyzer treatTA;
    treatTA.loadTimingRpt(productPath + "/" + treatProgram + "/report_timing.rpt");
    int recentCaseNum = 0;
    TimingPath* treatCritPath = treatTA.critPath;
    TimingPath* treatFinalPath = treatCritPath;

    //Treat Route check
    for(int i = 0; treatFinalPath->isEndRegister && i < treatTA.paths.size(); treatFinalPath = &treatTA.paths[i++]){
        if(i + 1 >= treatTA.paths.size()){
            Tool::logMessage("State -1. MRAddRouteConsist found program " + treatProgram + " has no path end by combinational semaphore. skip.");
            return -1;
        }
    }

    string anotherProgram = "";
    //find a separeted program
    int repeatTimes = 0;   //if repeateTimes >= 10, jump out of the recent loop, if caseNum >= 10, jump out of the recent loop
    while(repeatTimes < 10 && caseNum < 5){
        Tool::logMessage("Test " + to_string(seq) + ": add route consist");
        Tool::logMessage("Recent times: " + to_string(recentCaseNum + 1) + ".");
        int programsLen = programs.size();
        if(programsLen == 1){
            Tool::logMessage("State 2, seq " + to_string(seq) + "MRAddRouteConsist failed to find a separeted program with " + treatProgram + " .");
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
        if(findRepeatTimes >= 50){
            Tool::logMessage("State 2. Verilog " + treatProgram + " found more than 10 times used programs, skip");
            return 2;
        }
        usedVerilog.push_back(anotherProgram);

        //debug 8 20
        // anotherProgram = "condition_program_first5812";

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
                if(Tool::isDirectoryExists(newProgramContentVTRPath)){
                    string cp_r_comand = "cp -r " + newProgramContentVTRPath + " " + errorProductPath + ";rm -r " + newProgramContentVTRPath + ";rm " + anotherProgramPath;
                    system(cp_r_comand.c_str());
                }
                else
                    errorReport += ". File lost.";
                caseNum++;
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
        TimingAnalyzer anotherTAGlobal;
        anotherTAGlobal.loadTimingRpt(productPath + "/" + anotherProgram + "/report_timing.rpt");
        TimingPath* anotherFinalPath = anotherTAGlobal.critPath;

        //Another route check
        bool isAnotherOK = true;
        for(int i = 0; anotherFinalPath->isStartRegister && i < anotherTAGlobal.paths.size(); anotherFinalPath = &anotherTAGlobal.paths[i++]){
            if(i + 1 >= anotherTAGlobal.paths.size()){
                isAnotherOK = false;
                break;
            }
        }
        if(!isAnotherOK){
            Tool::logMessage("State 2. MRAddRouteConsist found program " + anotherProgram + " has no path start by combinational semaphore. Find next another program.\n");
            repeatTimes++;
        }
        else if(isLocDuplicate || anotherTAGlobal.critPathDelay == 0 || anotherRRUsed.size() == 0 || Tool::same_vec(treatRRUsed, anotherRRUsed)){
            //if rrused has duplicated, or has no RR used.
            Tool::logMessage("State 2. MRAddRouteConsist finds " + anotherProgram + " not satisfy condition with " + treatProgram + ": repeat location, skip it.\n");
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
            string comName = treatProgram + "_and_" + anotherProgram;
            recentTestName = Tool::getRecentTime() + "_" + treatProgram + "_with_" + anotherProgram;
            string testDirectory = treeHDLPath + "/Test/test" + to_string(seq) + "/test_addRouteConsist/" + recentTestName;
            Tool::createDirectory(treeHDLPath + "/Test");
            Tool::createDirectory(treeHDLPath + "/Test/test" + to_string(seq));
            Tool::createDirectory(treeHDLPath + "/Test/test" + to_string(seq)  + "/test_addRouteConsist");
            Tool::createDirectory(testDirectory);

            //pin fixed
            string combinePinFixed = VTRDrive::pinCommandFixed(placePathTreatProgram, placePathAnotherProgram, testDirectory);
            //blif combined
            BlifAnalyzer treatBA(blifPathTreatProgram);
            BlifAnalyzer anotherBA(blifPathAnotherProgram);
            treatBA.addModuleTag(); anotherBA.addModuleTag();
            Tool::fileWrite(blifPathTreatProgram, treatBA.getBlifContent());
            Tool::fileWrite(blifPathAnotherProgram, anotherBA.getBlifContent());
            string combineBlif = VTRDrive::blifCom(blifPathTreatProgram, blifPathAnotherProgram, testDirectory);

            BlifAnalyzer ba(testDirectory + "/" + combineBlif);
            TimingAnalyzer anotherTA;
            anotherTA.loadTimingRpt(productPathAnotherProgram + "/report_timing.rpt");
            TimingPath* mainPath = treatFinalPath;
            TimingPath* subPath = anotherFinalPath;
            string mainPathTreatProgram = productPathTreatProgram;
            string mainProgram = treatProgram;
            TimingAnalyzer mainTA = treatTA;
            TimingAnalyzer subTA = anotherTA;
            RouteAnalyzer mainRA = treatRA;

            string subCritInput = subPath->startPointandClk.first;
            string mainCritOut = mainPath->endPointandClk.first;
            int mainColon = mainCritOut.find(":");
            mainCritOut = mainCritOut.substr(mainColon + 1, mainCritOut.size() - mainColon - 1);
            string mainCritIn = mainPath->startPointandClk.first;
            mainCritIn = mainCritIn.substr(0, mainCritIn.find("."));
            mainCritOut = mainCritOut.substr(0, mainCritOut.find("."));
            subCritInput = subCritInput.substr(0, subCritInput.find("."));

            bool isConnectSucc = ba.connectCrit(mainCritOut, subCritInput);
            if(!isConnectSucc){  //connect error
                recentTestName = "VTR_data_structure_error_" + recentTestName;
                errorReport += "State 0: Error connect structure detect: " + testDirectory + "\n";
                errorReport += "Test directory: " + testDirectory + "\n";
                errorReport += "change first out: " + mainCritOut + "\n";
                errorReport += "change second input: " + subCritInput + "\n";
                Tool::logMessage(errorReport);
                return 3;
            }
            string newBlifContent = ba.getBlifContent();
            string rmOriginFile = "rm " + testDirectory + "/" + combineBlif;
            system(rmOriginFile.c_str());
            Tool::fileWrite(testDirectory + "/" + combineBlif, newBlifContent);

            //Execute pack, place and route step on the combination of two programs' .blif file.
            string comLog = vtr.runBlif(testDirectory + "/" + combineBlif, arch.getArchPath(), testDirectory + "/" + combinePinFixed, testDirectory);
            if(comLog == "false"){
                recentTestName = "VTR_com_crash_" + recentTestName;
                errorReport += "State 0: Error program detect: " + testDirectory + "\n";
                errorReport += "Test directory: " + testDirectory + "\n";
                errorReport += "change first out: " + mainCritOut + "\n";
                errorReport += "change second input: " + subCritInput + "\n";
                Tool::logMessage(errorReport);
                Tool::createDirectory("DefectTestCases");
                //copy test cases
                Drive::copyTestCases(productPathTreatProgram + "/" + treatProgram + ".blif", productPathAnotherProgram + "/" + anotherProgram + ".blif",
                                        testDirectory, defectCaseDirectory);
                return 3;
            }

            //Get delay of critical path delay, part path delay and wire length.
            double deadLine = 0.1;
            DelayandArea mainDandA = vtr.getCritDelayandAreafromLog(mainPathTreatProgram + "/vpr_stdout.log");
            TimingAnalyzer comTA; comTA.loadTimingRpt(testDirectory + "/report_timing.rpt");
            double comPartDelay = comTA.getAssignedPartDelay(mainProgram);
            RouteAnalyzer comRA; comRA.loadRoute(testDirectory + "/" + comName + ".pre-vpr.route");
            if(comPartDelay == 0){
                Tool::logMessage("Warning: State 2, MRAddRouteConsist cannot find main part route on combined solution, which may caused by brief timing report, skip.");
                anotherProgram = "";
                continue;
            }

            pair<pair<int, int>, int> targetOutPinLoc = mainRA.getRouteOutPinLoc(mainCritOut);
            double comPartPartWire = vtr.getPartWrieLength(mainProgram, comRA, mainCritOut, targetOutPinLoc.first.first, targetOutPinLoc.first.second, targetOutPinLoc.second);
            double comPartSlack = comTA.getAssignedPartSlack(mainProgram);
            
            double delayPreferRatio = (mainDandA.delay - comPartDelay) / mainDandA.delay;
            double wirePreferRatio = (mainDandA.wireArea - comPartPartWire) / mainDandA.wireArea;
            double slackDelta = mainTA.worstSlack - comPartSlack;

            if((comPartDelay < mainDandA.delay && comPartPartWire < mainDandA.wireArea && slackDelta < 0) 
                && (delayPreferRatio >= deadLine || wirePreferRatio >= deadLine || slackDelta <= -10 * deadLine)){
                //One defect
                errorReport += "MRAddRouteConsist Defect: Prefer solution of " + mainProgram + " occurred in combine solution.\n";
                errorReport += "Program 1: " + productPathTreatProgram + ".\n";
                errorReport += "Program 2: " + productPathAnotherProgram + ".\n";
                errorReport += "Main Program: " + mainProgram + ".\n";
                errorReport += "Max Delay: " + to_string(mainDandA.delay) + ".\n";
                errorReport += "Wire area: " + to_string(mainDandA.wireArea) + ".\n\n";
                errorReport += "Worst slack: " + to_string(mainTA.worstSlack) + "\n\n";

                errorReport += "Combine (Prefer Solution Occurred): " + treatProgram + " and " + anotherProgram + ".\n";
                errorReport += "change treat start: " + mainCritOut + "\n";
                errorReport += "change another end: " + subCritInput + "\n";
                errorReport += "Target Route Delay: " + to_string(comPartDelay) + ".\n";
                errorReport += "Target Part Wire area: " + to_string(comPartPartWire) + ".\n\n";
                errorReport += "Target Part slack: " + to_string(comPartSlack) + "\n\n";
                errorReport += "Compare prefer ratio: " + to_string(delayPreferRatio) + "\n";
                errorReport += "Wire prefer ratio: " + to_string(wirePreferRatio) + "\n";
                errorReport += "Slack delta: " + to_string(slackDelta) + "\n\n";
                errorReport += "Test directory: " + testDirectory + "\n";
                Tool::logMessage(errorReport);
                Tool::createDirectory("DefectTestCases");
                Tool::fileWrite(testDirectory + "/test.rpt", errorReport);
                //copy test cases
                Drive::copyTestCases(productPathTreatProgram + "/" + treatProgram + ".pre-vpr.blif", productPathAnotherProgram + "/" + anotherProgram + ".pre-vpr.blif",
                                        testDirectory, defectCaseDirectory);
                return 0;
            }
            else{
                //normal
                string testReport = "State 1: MRAddRouteConsist finds no defect.\n";
                testReport += "Program 1: " + productPathTreatProgram + ".\n";
                testReport += "Program 2: " + productPathAnotherProgram + ".\n";
                testReport += "Main Program: " + mainProgram + ".\n";
                testReport += "Max Delay: " + to_string(mainDandA.delay) + ".\n";
                testReport += "Wire area: " + to_string(mainDandA.wireArea) + ".\n\n";
                testReport += "Worst slack: " + to_string(mainTA.worstSlack) + "\n\n";

                testReport += "Combine (No prefer solution): " + treatProgram + " and " + anotherProgram + ".\n";
                testReport += "change treat start: " + mainCritOut + "\n";
                testReport += "change another end: " + subCritInput + "\n";
                testReport += "Target Route Delay: " + to_string(comPartDelay) + ".\n";
                testReport += "Target Part Wire area: " + to_string(comPartPartWire) + ".\n\n";
                testReport += "Target Part slack: " + to_string(comPartSlack) + "\n\n";
                testReport += "Compare prefer ratio: " + to_string(delayPreferRatio) + "\n";
                testReport += "Wire prefer ratio: " + to_string(wirePreferRatio) + "\n";
                testReport += "Slack delta: " + to_string(slackDelta) + "\n\n";
                testReport += "Test directory: " + testDirectory + "\n";
                Tool::logMessage(testReport);
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
        Tool::logMessage("State 2. Verilog " + treatProgram + " found more than 5 programs but not violate MRAddRouteConsist, skip");
        return 2;
    }

    return 1;
}

bool MRAddRouteConsist::isThisMR(string tag){
    if(tag == MRName)
        return true;
    return false;
}

string MRAddRouteConsist::getRecentTestName(){
    return recentTestName;
}

string MRAddRouteConsist::getMRName(){
    return MRName;
}