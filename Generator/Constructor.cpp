#include "Constructor.h"
#include "../Tool.h"

Constructor::Constructor(string setPath){
    initHDLPath = setPath + "/init_hdl_program";
    treeHDLPath = setPath + "/hdl_tree";
    if(!(Tool::isDirectoryExists(initHDLPath) && Tool::isDirectoryExists(treeHDLPath)))
        Tool::error("Error: Constructor load invalid or uninitialized HDL set.");
    
    //Judge if the init_hdl_program content is empty.
    bool is_init_empty = Tool::isDirectoryEmpty(initHDLPath.c_str());
    if(is_init_empty)
        Tool::error("Error: Constructor load empty initial HDL set.");
}

bool Constructor::buildTree(string programSet, int recentDeepLevel, string connType, string updateType, bool simulate_check, double conRatio){
    if(conRatio < 0)
        Tool::error("Error: Constructor cannot generate tree program with ratio " + to_string(conRatio) + ".");
    else if(conRatio >= 1)
        Tool::logMessage("Warning: the ratio " + to_string(conRatio) + " may be a large number.");

    string formatName = programSet + "_" + to_string(recentDeepLevel) + "_";
    bool cleanInitTree = updateType == "overlap";
    string hdlPath = treeHDLPath + "/1";
    string lastHDLPath = initHDLPath;
    int suffix = 1;
    while(Tool::isDirectoryExists(hdlPath)){
        if(isdigit(hdlPath[hdlPath.length()-1])){
            size_t pos = hdlPath.find_last_not_of("0123456789");
            if(pos != string::npos) {
                lastHDLPath = hdlPath;
                size_t numericSuffixStart = pos + 1;
                size_t numericSuffixLength = hdlPath.size() - numericSuffixStart;
                string numericSuffix = hdlPath.substr(numericSuffixStart, numericSuffixLength);
                suffix = stoi(numericSuffix) + 1;
                hdlPath = hdlPath.substr(0, pos+1);
                string completeSuffix = to_string(suffix);
                if(numericSuffix[0] == '0' && numericSuffixLength != 1){
                    size_t count = numericSuffix.find_first_not_of('0');
                    count = (count == std::string::npos) ? numericSuffix.length() : count;
                    string leadingZeros(count, '0');
                    completeSuffix = leadingZeros + completeSuffix;
                }
                hdlPath += completeSuffix;
            }
        }
        else
            hdlPath += to_string(suffix);
    }
    if(!Tool::createDirectory(hdlPath)){
        Tool::error("Error: UserDrive canno create directory in " + hdlPath);
        return false;
    }

    //Copy program to recent dir
    vector<string> allPaths;
    vector<string> allProgramPath;
    vector<string> allProgramName;
    Tool::getAllFilefromDir(lastHDLPath, allPaths);
    for(auto it =allPaths.begin(); it < allPaths.end(); it++){
        string recentPath = *it;
        string recentFileName = Tool::findFilefromPath(recentPath);
        if(recentFileName.find(".") == string::npos){
            Tool::copyDirectory(recentPath, hdlPath + '/' + recentFileName);
            continue;
        }
        allProgramName.push_back(recentFileName);
        allProgramPath.push_back(recentPath);
        string newPath = hdlPath + '/' + recentFileName;
        if(!cleanInitTree)
            Tool::copyFile(recentPath, newPath);
    }
    int initProgramNum = allProgramPath.size();
    //Random combine
    int targetNum = allProgramPath.size() * conRatio;
    string operators[] = {"and", "next"};
    int repeatTimes = 0;
    for(int i = 0; i < targetNum; i++){
        //Choose two program
        int firstRand = Tool::getRandomfromClosed(0, initProgramNum - 1);
        int secondRand = Tool::getRandomfromClosed(0, initProgramNum - 1);
        string firstName = allProgramName[firstRand];
        firstName = firstName.substr(0, firstName.find("."));
        string secondName = allProgramName[secondRand];
        secondName = secondName.substr(0, secondName.find("."));
        string firstPath = allProgramPath[firstRand];
        string secondPath = allProgramPath[secondRand];
        int operateRand = Tool::getRandomfromClosed(0, 1);
        string operate = operators[operateRand];
        string newProgramName = "_" + firstName + "_" + operate + "_" + secondName + "_" + ".v";
        if(find(allProgramName.begin(), allProgramName.end(), newProgramName) != allProgramName.end()){
            repeatTimes += 1;
            if(repeatTimes >= 100){
                Tool::error("Error: Constructor faces high repeated rate challenge. You are supposed to decrease conRatio or improve the number of init programs.");
                return false;
            }
            i--;
            continue;
        }
        else{
            string newFileName = formatName + to_string(i);
            repeatTimes = 0;
            if(operate == "and")
                Tool::verilogCom(firstPath, secondPath, hdlPath, newFileName);
            else if(operate == "next")
                Tool::verilogSeiresCom(firstPath, secondPath, hdlPath, newFileName);
            if(simulate_check){
                stringstream ss = Tool::getModule(hdlPath + "/" + newFileName + ".v", newFileName);
                vector<Attribute*> outputs, inputs, inouts;
                Tool::extractInoutfromTextV(ss, outputs, inputs);
                inouts.insert(inouts.end(), outputs.begin(), outputs.end());
                inouts.insert(inouts.end(), inputs.begin(), inputs.end());
                vector<Attribute> newInout;
                for(auto it = inouts.begin(); it < inouts.end(); it++){
                    Attribute a = *(*it);
                    newInout.push_back(a);
                }
                Executer::generateTbandVerify(newFileName, newInout, hdlPath);
            }
        }
    }
    return true;
}