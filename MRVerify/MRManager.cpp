#include "MRManager.h"
#include "../Tool.h"

MRManager::MRManager(vector<string> callMRs, VTRDrive& vtr, string programName, 
        string treeHDLPath, vector<string> allPrograms, Arch arch, string errorReportPath) :
    callMRs(callMRs),
    vtr(vtr),
    treatProgram(programName),
    treeHDLPath(treeHDLPath),
    programs(allPrograms),
    productPath(treeHDLPath + "/product"),
    arch(arch),
    errorReportPath(errorReportPath)
{
    // arch.rrGraphSet(treeHDLPath + "/product/" + treatProgram + "/" + treatProgram + "_rr_graph.xml");
    // rrGraphUsed = arch.getRRUsed();
    for(auto it = callMRs.begin(); it < callMRs.end(); it++){
        string recentCall = *it;
        if(recentCall == "MRConsistLayout"){
            targetMRs.push_back(new MRConsistLayout(vtr, treatProgram, treeHDLPath, allPrograms, arch));
        }
        else if(recentCall == "MRAddRouteConsist"){
            targetMRs.push_back(new MRAddRouteConsist(vtr, treatProgram, treeHDLPath, allPrograms, arch));
        }
    }
};

bool MRManager::MRVerify(int seq, int& allCaseNum, double& defect, string defectCaseDirectory)
{
    bool result = true;
    bool onceVerify = false;
    int caseNum = 0;
    Tool::createDirectory(errorReportPath);
    for(auto it = targetMRs.begin(); it < targetMRs.end(); it++){
        string errorReport = "";
        int recentCaseNum = 0;
        int recentCommand = (*it)->verify(seq, onceVerify, errorReport, recentCaseNum, defectCaseDirectory);
        caseNum += recentCaseNum;
        if(0 == recentCommand){
            Tool::logMessage("MR rules violate: " + (*it)->getMRName());
            string recentErrorReport = errorReportPath + "/" + (*it)->getMRName() + "_" +(*it)->getRecentTestName() + ".rpt";
            Tool::logMessage("Error report is saved in: " + errorReportPath);
            Tool::fileWrite(recentErrorReport, errorReport);
            result = false;
            defect += 1;
        }
        else if(-1 == recentCommand){
            Tool::logMessage("Low quality case, skip remain MRs.\n");
            result = true;
            break;
        }
        else if(3 == recentCommand){
            Tool::logMessage("Detect Single Verilog error!");
            string recentErrorReport = errorReportPath + "/" + (*it)->getMRName() + "_" +(*it)->getRecentTestName() + ".rpt";
            Tool::logMessage("Error report is saved in: " + errorReportPath);
            Tool::fileWrite(recentErrorReport, errorReport);
            result = false;
            defect += 1;
        }
        else{
            onceVerify = true;
        }
    }

    allCaseNum += caseNum;
    return result | onceVerify;
}


// bool MRManager::MRConsistLayout(int seq){
//     //Random find another verilog exists
//     vector<string> recentVerilog;
//     Tool::getAllFilefromDir(productPath, recentVerilog);
//     string anotherProgram = "";
//     while(true){
//         while(anotherProgram == "" || anotherProgram == treatProgram){
//             int anotherIndex = Tool::getRandomfromClosed(0, programs.size());
//             anotherProgram = programs[anotherIndex];
//         }

//         //Execute another program
//         if(find(recentVerilog.begin(), recentVerilog.end(), anotherProgram) == recentVerilog.end()){
//             Tool::createDirectory(productPath +"/" + anotherProgram);
//             vtr.executeRunVTRFlow(treeHDLPath + "/" + anotherProgram, arch.getName(), productPath +"/" + anotherProgram);
//         }
//         arch.rrGraphSet(treeHDLPath + "/product/" + anotherProgram + "/" + anotherProgram + "_rr_graph.xml");
//         vector<int> recentRRUsed = arch.getRRUsed();
//         if(Tool::same_vec(rrGraphUsed, recentRRUsed)){
//             //if rrused has duplicated
//             anotherProgram = "";
//             for(auto it = recentVerilog.begin(); it < recentVerilog.end(); it++){
//                 if(*it == anotherProgram){
//                     recentVerilog.erase(it);
//                     break;
//                 }
//             }
//         }
//         else{   //Find two separeted layout
//             break;
//         }
//     }

//     //Combined layout
//     string productPathTreatProgram = productPath + "/" + treatProgram;
//     string productPathAnotherProgram = productPath + "/" + anotherProgram;
//     string blifPathTreatProgram = productPathTreatProgram + "/" + treatProgram + ".pre-vpr.blif";
//     string blifPathAnotherProgram = productPathAnotherProgram + "/" + anotherProgram + ".pre-vpr.blif";
//     string placePathTreatProgram = productPathTreatProgram + "/" + treatProgram + ".place";
//     string placePathAnotherProgram = productPathAnotherProgram + "/" + anotherProgram + ".place";
//     string testDirectory = treeHDLPath + "/Test/test" + to_string(seq) + "/test_consistLayout/" + Tool::getRecentTime();
//     Tool::createDirectory(testDirectory);
    
//     //pin fixed
//     string combinePinFixed = pinCommandFixed(placePathTreatProgram, placePathAnotherProgram, testDirectory);
//     //blif combined
//     string combineBlif = blifCom(blifPathTreatProgram, blifPathAnotherProgram, testDirectory);
//     //Execute pack, place and route step on the combination of two programs' .blif file.
//     Tool::logMessage("Test " + to_string(seq));
//     string comLog = vtr.runBlif(testDirectory + "/" + combineBlif, arch.getArchPath(), testDirectory + "/" + combinePinFixed, testDirectory);

//     //Get delay of all critical path and area and compare
//     DelayandArea treatDandA = vtr.getCritDelayandAreafromLog(productPathTreatProgram + "/vpr_stdout.log");
//     DelayandArea anotherDandA = vtr.getCritDelayandAreafromLog(productPathAnotherProgram + "/vpr_stdout.log");
//     DelayandArea comDandA = vtr.getCritDelayandAreafromLog(testDirectory + "/vpr_stdout.log");
//     double separeteDelay = max(treatDandA.delay, anotherDandA.delay);
//     double separeteBlockArea = treatDandA.blockArea + anotherDandA.blockArea;
//     double separeteWireArea = treatDandA.wireArea + anotherDandA.wireArea;
//     if(separeteDelay < comDandA.delay && separeteBlockArea < comDandA.blockArea && separeteWireArea < comDandA.wireArea){
//         //One defect
//         Tool::logMessage("Defect: all the delay and area are less than the ones on their combination.");
//         Tool::logMessage("Program 1: " + productPathTreatProgram + ".");
//         Tool::logMessage("Delay: " + to_string(treatDandA.delay) + ".");
//         Tool::logMessage("Block area: " + to_string(treatDandA.blockArea) + ".");
//         Tool::logMessage("Wire area: " + to_string(treatDandA.wireArea) + ".\n");
//         Tool::logMessage("Program 2: " + productPathAnotherProgram + ".");
//         Tool::logMessage("Delay: " + to_string(anotherDandA.delay) + ".");
//         Tool::logMessage("Block area: " + to_string(anotherDandA.blockArea) + ".");
//         Tool::logMessage("Wire area: " + to_string(anotherDandA.wireArea) + ".\n");
//         Tool::logMessage("Combine " + treatProgram + " and " + anotherProgram + ".");
//         Tool::logMessage("Delay: " + to_string(comDandA.delay) + ".");
//         Tool::logMessage("Block area: " + to_string(comDandA.blockArea) + ".");
//         Tool::logMessage("Wire area: " + to_string(comDandA.wireArea) + ".\n");
//         return false;
//     }
//     else if(separeteDelay > comDandA.delay && separeteBlockArea > comDandA.blockArea && separeteWireArea > comDandA.wireArea){
//         //another defect
//         Tool::logMessage("Defect: all the delay and area are larger than the ones on their combination.");
//         Tool::logMessage("Program 1: " + productPathTreatProgram + ".");
//         Tool::logMessage("Delay: " + to_string(treatDandA.delay) + ".");
//         Tool::logMessage("Block area: " + to_string(treatDandA.blockArea) + ".");
//         Tool::logMessage("Wire area: " + to_string(treatDandA.wireArea) + ".");
//         Tool::logMessage("Program 2: " + productPathAnotherProgram + ".");
//         Tool::logMessage("Delay: " + to_string(anotherDandA.delay) + ".");
//         Tool::logMessage("Block area: " + to_string(anotherDandA.blockArea) + ".");
//         Tool::logMessage("Wire area: " + to_string(anotherDandA.wireArea) + ".");
//         Tool::logMessage("Combine " + treatProgram + " and " + anotherProgram + ".");
//         Tool::logMessage("Delay: " + to_string(comDandA.delay) + ".");
//         Tool::logMessage("Block area: " + to_string(comDandA.blockArea) + ".");
//         Tool::logMessage("Wire area: " + to_string(comDandA.wireArea) + ".\n");
//         return false;
//     }
//     else{
//         //normal
//         Tool::logMessage("No defect");
//         Tool::logMessage("Program 1: " + productPathTreatProgram + ".");
//         Tool::logMessage("Program 2: " + productPathAnotherProgram + ".\n");
//         return true;
//     }
// }

// string MRManager::blifCom(string filepath1, string filepath2, string outPath){
//     return "";
// }

// string MRManager::pinCommandFixed(string filepath1, string filepath2, string outPath){
//     return "";
// }