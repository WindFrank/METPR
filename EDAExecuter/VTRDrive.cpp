#include "VTRDrive.h"
#include "../Tool.h"

string VTRDrive::executeRunFlow(string programPath, string archPath, string tempPath, string topModule){
    string programName = topModule;
    string run_vtr_flow = VTRRoutePath + "/vtr_flow/scripts/run_vtr_flow.py ";
    string program = programPath + " ";
    string arch = archPath + " ";
    string tempDir = "-temp_dir " + tempPath + " ";
    string startOdin = "-start odin ";
    string deviceFixed = "--device fixed ";
    string packingTimingSolute = "--alpha_clustering 1.0 ";
    string route_chan_width = "--route_chan_width 100 ";
    // string rr_graph = "--write_rr_graph " + programName + "_rr_graph.xml ";
    string nocPlaceWeight = "--noc_placement_weighting 0 ";
    string nocSwapPercentage = "--noc_swap_percentage 0 ";
    string nocLatencyWei = "--noc_latency_weighting 0 ";
    string place_quench_alg = "--place_quench_algorithm slack_timing ";
    string time_report_detail = "--timing_report_detail detailed ";
    string report_path_num = "--timing_report_npaths 100000 ";
    // string routeMaxCrit = "--max_criticality 1.00 ";

    // string sdcPath = string(getenv("VGENERATOR_WORK_DIR")) + "/" + writeSDCforExec(programPath, tempPath);
    // string sdcSet = "--sdc_file " + sdcPath + " ";
    // string seed = "--seed " + to_string(Tool::getRandomfromClosed(0, 1000));
    //source python environment
    // string pythonEnvCommand = "cd " + VTRRoutePath + ";source .venv/bin/activate;cd -;";
    string pythonEnvCommand = "";
    //Write seed file
    //Tool::fileWrite(tempPath + "/seed.txt", seed);
    string final_command = pythonEnvCommand + run_vtr_flow + program + arch + tempDir + startOdin + deviceFixed 
                           + packingTimingSolute + nocPlaceWeight + nocLatencyWei + place_quench_alg + route_chan_width 
                           + nocSwapPercentage + time_report_detail + report_path_num;
    int resultFlag = system(final_command.c_str());
    
    if(resultFlag != 0){
        Tool::logMessage("Error: VTRDrive execute " + final_command + " failed.");
        return "fail";
    }
    else
        timingRptCom(tempPath);
    return "success";
}

string VTRDrive::executeRandomRunFlow(string programPath, string archPath, string tempPath, string topModule, int seed)
{
    string programName = topModule;
    string run_vtr_flow = VTRRoutePath + "/vtr_flow/scripts/run_vtr_flow.py ";
    string program = programPath + " ";
    string arch = archPath + " ";
    string tempDir = "-temp_dir " + tempPath + " ";
    string startOdin = "-start odin ";
    string deviceFixed = "--device fixed ";
    string packingTimingSolute = "--alpha_clustering 1.0 ";
    string route_chan_width = "--route_chan_width 100 ";
    // string rr_graph = "--write_rr_graph " + programName + "_rr_graph.xml ";
    string nocPlaceWeight = "--noc_placement_weighting 0 ";
    string nocSwapPercentage = "--noc_swap_percentage 0 ";
    string nocLatencyWei = "--noc_latency_weighting 0 ";
    string place_quench_alg = "--place_quench_algorithm slack_timing ";
    string time_report_detail = "--timing_report_detail detailed ";
    string report_path_num = "--timing_report_npaths 100000 ";
    // string routeMaxCrit = "--max_criticality 1.00 ";

    string seedStr = "--seed " + to_string(seed) + " ";
    // string sdcPath = writeSDCforExec(programPath, string(getenv("VGENERATOR_WORK_DIR")) + "/" + tempPath);
    // string sdcSet = "--sdc_file " + sdcPath + " ";
    //source python environment
    // string pythonEnvCommand = "cd " + VTRRoutePath + ";source .venv/bin/activate;cd -;";
    string pythonEnvCommand = "";
    //Write seed file
    //Tool::fileWrite(tempPath + "/seed.txt", seed);
    string final_command = pythonEnvCommand + run_vtr_flow + program + arch + tempDir + startOdin + deviceFixed 
                           + packingTimingSolute + nocPlaceWeight + nocLatencyWei + place_quench_alg + route_chan_width 
                           + nocSwapPercentage + time_report_detail + report_path_num + seedStr;
    int resultFlag = system(final_command.c_str());
    
    if(resultFlag != 0){
        Tool::logMessage("Error: VTRDrive execute " + final_command + " failed.");
        return "fail";
    }
    else
        timingRptCom(tempPath);
    return "success";
}

string VTRDrive::runBlif(string blifPath, string archPath, string pinFixedPath, string tempPath, string topModule){
    string result = "";
    string vprPath = VTRRoutePath + "/vpr/vpr ";
    string archMPath = string(getenv("VGENERATOR_WORK_DIR")) + "/" + archPath + " ";
    string blifName = Tool::findFilefromPath(Tool::findFilefromPath(blifPath));
    string moduleName = blifName.substr(0, blifName.rfind('.'));
    string blifCommand = moduleName + " --circuit_file " + string(getenv("VGENERATOR_WORK_DIR")) + "/" + blifPath + " ";
    string deviceFixed = "--device fixed ";
    string route_chan_width = "--route_chan_width 100 ";
    string time_report_detail = "--timing_report_detail detailed ";
    string report_path_num = "--timing_report_npaths 100000 ";
    // string placeName = Tool::findFilefromPath(pinFixedPath);
    string fix_clusters = "--fix_clusters " + string(getenv("VGENERATOR_WORK_DIR")) + "/" + pinFixedPath + " ";
    string nocPlaceWeight = "--noc_placement_weighting 0 ";
    string nocLatencyWei = "--noc_latency_weighting 0 ";
    string nocSwapPercentage = "--noc_swap_percentage 0 ";
    string place_quench_alg = "--place_quench_algorithm slack_timing ";
    string packingTimingSolute = "--alpha_clustering 1 ";
    string blifContent = Tool::readFile(blifPath);
    // string sdcSet = "--sdc_file " + writeSDCforBlif(blifPath, string(getenv("VGENERATOR_WORK_DIR")) + "/" + tempPath) + " ";
    // string routeMaxCrit = "--max_criticality 1.00 ";
    // string seed = "--seed " + to_string(Tool::getRandomfromClosed(0, 1000));
    //write seed file
    // Tool::fileWrite(tempPath + "/seed.txt", seed);

    Tool::createDirectory(tempPath);
    string final_command = "cd " + tempPath + ";" + vprPath + archMPath + blifCommand + deviceFixed + route_chan_width
                            + packingTimingSolute + place_quench_alg + nocPlaceWeight + nocLatencyWei
                            + nocSwapPercentage + time_report_detail + report_path_num + fix_clusters;
    int resultFlag = system(final_command.c_str());
    if(resultFlag != 0){
        Tool::logMessage("Error: VTRDrive execute " + final_command + " failed.");
        result = "false";
    }
    
    if(resultFlag == 0)
        timingRptCom(tempPath);

    return result;
}

string VTRDrive::runSeedBlif(string blifPath, string archPath, string pinFixedPath, string tempPath, string topModule, int seed){
    string result = "success";
    string vprPath = VTRRoutePath + "/vpr/vpr ";
    string archMPath = string(getenv("VGENERATOR_WORK_DIR")) + "/" + archPath + " ";
    string blifName = Tool::findFilefromPath(Tool::findFilefromPath(blifPath));
    string moduleName = blifName.substr(0, blifName.rfind('.'));
    string blifCommand = moduleName + " --circuit_file " + string(getenv("VGENERATOR_WORK_DIR")) + "/" + blifPath + " ";
    string deviceFixed = "--device fixed ";
    string route_chan_width = "--route_chan_width 100 ";
    string time_report_detail = "--timing_report_detail detailed ";
    string report_path_num = "--timing_report_npaths 100000 ";
    // string placeName = Tool::findFilefromPath(pinFixedPath);
    string fix_clusters = "--fix_clusters " + string(getenv("VGENERATOR_WORK_DIR")) + "/" + pinFixedPath + " ";
    string nocPlaceWeight = "--noc_placement_weighting 0 ";
    string nocLatencyWei = "--noc_latency_weighting 0 ";
    string nocSwapPercentage = "--noc_swap_percentage 0 ";
    string place_quench_alg = "--place_quench_algorithm slack_timing ";
    string packingTimingSolute = "--alpha_clustering 1 ";
    // string sdcSet = "--sdc_file " + writeSDCforBlif(blifPath, string(getenv("VGENERATOR_WORK_DIR")) + "/" + tempPath) + " ";

    // string routeMaxCrit = "--max_criticality 1.00 ";
    string seedStr = "--seed " + to_string(seed) + " ";
    //write seed file
    // Tool::fileWrite(tempPath + "/seed.txt", seed);

    Tool::createDirectory(tempPath);
    string final_command = "cd " + tempPath + ";" + vprPath + archMPath + blifCommand + deviceFixed + route_chan_width
                            + packingTimingSolute + place_quench_alg + nocPlaceWeight + nocLatencyWei
                            + nocSwapPercentage + time_report_detail + report_path_num + fix_clusters + seedStr;
    int resultFlag = system(final_command.c_str());
    if(resultFlag != 0){
        Tool::logMessage("Error: VTRDrive execute " + final_command + " failed.");
        result = "false";
    }
    
    if(resultFlag == 0)
        timingRptCom(tempPath);

    return result;
}

DelayandArea VTRDrive::getCritDelayandAreafromLog(string logPath){
    string content = Tool::readFile(logPath);
    string critSign = "Final critical path delay (least slack):";
    string blockArea = "Total used logic block area:";
    string wireAreaSign = "Total wirelength:";
    string washContent = Tool::washString(content);
    DelayandArea daa;
    daa.blockArea = getValueBehindStr(washContent, blockArea);
    daa.wireArea = getValueBehindStr(washContent, wireAreaSign);
    daa.delay = getValueBehindStr(washContent, critSign);

    return daa;
}

double VTRDrive::getValueBehindStr(string content, string signStr){
    int index = content.find(signStr) + signStr.size() + 1;
    bool scientific = false;
    string num = "";
    int contentLen = content.size();
    for(int i = index; i < contentLen; i++){
        char c = content[i];
        if(c == ' ' || c == ',')
            break;
        num += c;
    }
    if(num.find('e') != string::npos)
        return stod(num);
    if(num == "nan")
        return 0.0;
    double result = atof(num.c_str());
    return result;
}

string VTRDrive::pinCommandFixed(string filepath1, string filepath2, string testDirectory){
    //read files
    string filename1 = Tool::findFilefromPath(filepath1);
    string filename2 = Tool::findFilefromPath(filepath2);
    ifstream file1(filepath1);
    if(!file1.is_open()){
        Tool::error("Error opening file: " + filename1);
        return "";
    }
    stringstream ss1;
    ss1 << file1.rdbuf();
    file1.close();

    ifstream file2(filepath2);
    if(!file2.is_open()){
        Tool::error("Error opening file: " + filename2);
        return "";
    }
    stringstream ss2;
    ss2 << file2.rdbuf();
    file2.close();

    //combine
    stringstream content;
    stringstream out1;
    stringstream in1;
    stringstream out2;
    stringstream in2;
    stringstream content1;
    stringstream content2;
    int arraySize1X, arraySize1Y;
    int arraySize2X, arraySize2Y;
    string line;
    while(getline(ss1, line)){
        if(line.find("#block name") != string::npos || line.find("#-") != string::npos) content << line << "\n";
        else if(line.find("Netlist_File") != string::npos) continue;
        else if(line.find("Array size:") != string::npos){
            istringstream iss(line);
            string token;
            vector<int> size;
            while(iss >> token){
                if(isdigit(token[0])){
                    size.push_back(stoi(token));
                }
            }
            if(size.size() >= 2) {arraySize1X = size[0]; arraySize1Y = size[1];}
            else Tool::error("unable to find valid size info");
        }
        else if(line.find("out:") != string::npos){
            out1 << line << "\n";
        }
        else{
            if(isInput(line, arraySize1X, arraySize1Y)) in1 << line << "\n";
        }
    }
    while(getline(ss2, line)){
        if(line.find("Array size:") != string::npos){
            istringstream iss(line);
            string token;
            vector<int> size;
            while(iss >> token){
                if(isdigit(token[0])){
                    size.push_back(stoi(token));
                }
            }
            if(size.size() >= 2) {arraySize2X = size[0]; arraySize2Y = size[1];}
            else Tool::error("unable to find valid size info");
        }
        else if(line.find("Netlist_File") != string::npos ||
        line.find("#block name") != string::npos || line.find("#-") != string::npos) continue;
        else if(line.find("out:") != string::npos){
            out2 << line << "\n";
        }
        else{
            if(isInput(line, arraySize2X, arraySize2Y)) in2 << line << "\n";
        }
    }
    content1 << out1.str();
    content1 << in1.str();
    content2 << out2.str();
    content2 << in2.str();

    //deal with same block name
    vector<string> blockname1 = getBlockNames(content1.str());
    vector<string> blockname2 = getBlockNames(content2.str());
    vector<string> commonBlockNames;
    unordered_set<string> set1(blockname1.begin(), blockname1.end());
    for(const auto& str : blockname2){
        if(set1.count(str) > 0){
            commonBlockNames.push_back(str);
        }
    }
    vector<string> modifiedBlockNames = commonBlockNames;
    for(auto& v : modifiedBlockNames){
        int suffix = 1;
        while(find(blockname1.begin(), blockname1.end(), v) != blockname1.end() ||
        find(blockname2.begin(), blockname2.end(), v) != blockname2.end() ||
        Tool::hasDuplicate(modifiedBlockNames)){
            if(isdigit(v[v.length()-1])){
                size_t pos = v.find_last_not_of("0123456789");
                if(pos != string::npos) {
                    size_t numericSuffixStart = pos + 1;
                    size_t numericSuffixLength = v.size() - numericSuffixStart;
                    string numericSuffix = v.substr(numericSuffixStart, numericSuffixLength);
                    suffix = stoi(numericSuffix) + 1;
                    v = v.substr(0, pos+1);
                    string completeSuffix = to_string(suffix);
                    if(numericSuffix[0] == '0' && numericSuffixLength != 1){
                        size_t count = numericSuffix.find_first_not_of('0');
                        count = (count == std::string::npos) ? numericSuffix.length() : count;
                        string leadingZeros(count, '0');
                        completeSuffix = leadingZeros + completeSuffix;
                    }
                    v += completeSuffix;
                }
            }
            else{
                v += to_string(suffix);
            }
        }
    }
    stringstream newOut2;
    stringstream newIn2;
    while(getline(out2, line)){
        for(size_t i = 0; i < commonBlockNames.size(); i++){
            size_t pos = line.find(commonBlockNames[i]);
            while(pos != string::npos){
                char nextChar = (pos + commonBlockNames[i].length() < line.length()) ? line[pos + commonBlockNames[i].length()] : ' ';
                if(!isdigit(nextChar)){
                    line.replace(pos, commonBlockNames[i].length(), modifiedBlockNames[i]);
                }
                pos = line.find(commonBlockNames[i], pos + modifiedBlockNames[i].length());
            }
        }
        newOut2 << line << '\n';
    }
    while(getline(in2, line)){
        for(size_t i = 0; i < commonBlockNames.size(); i++){
            size_t pos = line.find(commonBlockNames[i]);
            while(pos != string::npos){
                char nextChar = (pos + commonBlockNames[i].length() < line.length()) ? line[pos + commonBlockNames[i].length()] : ' ';
                if(!isdigit(nextChar)){
                    line.replace(pos, commonBlockNames[i].length(), modifiedBlockNames[i]);
                }
                pos = line.find(commonBlockNames[i], pos + modifiedBlockNames[i].length());
            }
        }
        newIn2 << line << '\n';
    }

    //check
    content << out1.str();
    content << newOut2.str();
    content << in1.str();
    content << newIn2.str();
    int lineCount = 0;
    stringstream result;
    vector<string> xysubblk;
    while(getline(content, line)){
        if(++lineCount <= 2){
            result << line << "\n";
            continue;
        }
        size_t hashPos = line.find('#');
        if (hashPos != string::npos) {
            line = line.substr(0, hashPos);
        }
        result << line << "\n";
        string combinedVal = "";
        istringstream line_stream(line);
        string block_name;
        int x, y, subblk;
        line_stream >> block_name >> x >> y >> subblk;
        combinedVal += to_string(x) + "_" + to_string(y) + "_" + to_string(subblk);
        xysubblk.push_back(combinedVal);
    }
    if(Tool::hasDuplicate(xysubblk)) 
        Tool::error("Error: Unchecked duplicate location.");
    
    //write into file
    string combinedFilename = filename1.substr(0, filename1.find_last_of('.')) + "_and_" + filename2.substr(0, filename2.find_last_of('.')) + "_fixed_pin.place";
    if(testDirectory != "")
        testDirectory += "/";
    ofstream outputFile(testDirectory + combinedFilename);
    if(!outputFile.is_open()){
        Tool::error("Error creating output file: " + combinedFilename);
        return "";
    }
    else{
        Tool::logMessage("Create file " + testDirectory  +  combinedFilename  + " successfully.");
    }
    outputFile << result.str();
    return combinedFilename;
}

string VTRDrive::blifCom(string filepath1, string filepath2, string testDirectory){
    //read files
    string filename1 = Tool::findFilefromPath(filepath1);
    string filename2 = Tool::findFilefromPath(filepath2);
    ifstream file1(filepath1);
    if(!file1.is_open()){
        Tool::error("Error opening file: " + filename1);
        return "";
    }
    stringstream ss1;
    ss1 << file1.rdbuf();
    string content1 = ss1.str();
    file1.close();

    ifstream file2(filepath2);
    if(!file2.is_open()){
        Tool::error("Error opening file: " + filename2);
        return "";
    }
    stringstream ss2;
    ss2 << file2.rdbuf();
    string content2 = ss2.str();
    file2.close();

    //deal with same model name
    stringstream result;
    stringstream models;
    filename1 = filename1.substr(0, filename1.find('.'));
    filename2 = filename2.substr(0, filename2.find('.'));
    string combinedFilename = filename1 + "_and_" + filename2;
    vector<string> modelNames1 = getAllModelNames(content1);
    vector<string> modelNames2 = getAllModelNames(content2);
    stringstream model1 = getModel(filepath1, modelNames1[0]);
    stringstream model2 = getModel(filepath2, modelNames2[0]);
    vector<string> commonModelNames;
    unordered_set<string> set(modelNames1.begin(), modelNames1.end());
    for(const auto& str : modelNames2){
        if(set.count(str) > 0){
            commonModelNames.push_back(str);
        }
    }
    for(const auto& name : commonModelNames){
        if(name == modelNames1[0] || name == modelNames2[0]){
            Tool::error("Error: Same main model name");
        }
        string result1;
        string result2;
        stringstream m1 = getModel(filepath1, name);
        stringstream m2 = getModel(filepath2, name);
        string preResult1 = m1.str();
        string preResult2 = m2.str();
        for(int i = 0; i < preResult1.size(); i++)
            if(preResult1[i] != '\\')
                result1 += preResult1[i];
        for(int i = 0; i < preResult2.size(); i++)
            if(preResult2[i] != '\\')
                result2 += preResult2[i];

        result1 = Tool::washString(result1);
        result2 = Tool::washString(result2);
        if(result1 != result2){
            Tool::error("Error: Same main module with inconsist function.");
        }
        else{
            models << m1.str();
        }
    }
    modelNames1.erase(modelNames1.begin());
    modelNames2.erase(modelNames2.begin());
    modelNames1.erase(
        remove_if(modelNames1.begin(), modelNames1.end(),
                       [&commonModelNames](const std::string& model) {
                           return std::find(commonModelNames.begin(), commonModelNames.end(), model) != commonModelNames.end();
                       }),
        modelNames1.end()
    );
    modelNames2.erase(
        remove_if(modelNames2.begin(), modelNames2.end(),
                       [&commonModelNames](const std::string& model) {
                           return std::find(commonModelNames.begin(), commonModelNames.end(), model) != commonModelNames.end();
                       }),
        modelNames2.end()
    );
    for(const auto& str : modelNames1) models << getModel(filepath1, str).str();
    for(const auto& str : modelNames2) models << getModel(filepath2, str).str();

    //establish the new main model
    result << ".model " + combinedFilename + "\n" + "\n";
    stringstream latch1;
    stringstream subckt1;
    stringstream names1;
    stringstream latch2;
    stringstream subckt2;
    stringstream names2;
    string line;
    while(getline(model1, line)){
        if(line.find(".inputs") != string::npos){
            if(line[line.length() - 1] == '\\') result << line + "\n";
            else result << line.substr(0, line.length()-1);
            while(!line.empty()){
                getline(model1, line);
                if(line[line.length()-1] == '\\') result << line + "\n";
                else result << line.substr(0, line.length()-1);
            }
            result << " \\";
            result << "\n";
            while(getline(model2, line)){
                if(line.find("inputs") != string::npos){
                    size_t pos = line.find(".inputs");
                    if (pos != std::string::npos) {
                        line.erase(pos, 7);
                        result << line + "\n";
                        while(!line.empty()){
                            getline(model2, line);
                            result << line + "\n";
                        }
                    }
                    break;
                }
            }
        }
        else if(line.find("outputs") != string::npos){
            if(line[line.length()-1] == '\\') result << line + "\n";
            else result << line.substr(0, line.length() - 1);
            while(!line.empty()){
                getline(model1, line);
                if(line[line.length()-1] == '\\') result << line + "\n";
                else result << line.substr(0, line.length()-1);
            }
            result << " \\";
            result << "\n";
            while(getline(model2, line)){
                if(line.find("outputs") != string::npos){
                    size_t pos = line.find(".outputs");
                    if (pos != std::string::npos) {
                        line.erase(pos, 8);
                        result << line + "\n";
                        while(!line.empty()){
                            getline(model2, line);
                            result << line + "\n";
                        }
                    }
                    break;
                }
            }
        }
        else if(line.find(".latch") != string::npos){
            latch1 << line + "\n";
            while(!line.empty()){
                getline(model1, line);
                latch1 << line + "\n";
            }
        }
        else if(line.find(".subckt") != string::npos){
            subckt1 << line + "\n";
            while(!line.empty()){
                getline(model1, line);
                subckt1 << line + "\n";
            }
        }
        else if(line.find(".names") != string::npos){
            names1 << line + "\n";
            while(!line.empty()){
                getline(model1, line);
                names1 << line + "\n";
            }
        }
    }
    while(getline(model2, line)){
        if(line.find(".latch") != string::npos){
            latch2 << line + "\n";
            while(!line.empty()){
                getline(model2, line);
                latch2 << line + "\n";
            }
        }
        else if(line.find(".subckt") != string::npos){
            subckt2 << line + "\n";
            while(!line.empty()){
                getline(model2, line);
                subckt2 << line + "\n";
            }
        }
        else if(line.find(".names") != string::npos){
            string tempNames = "";
            tempNames += line + "\n";
            while(!line.empty()){
                getline(model2, line);
                tempNames += line + "\n";
            }
            if(tempNames.find(".names vcc") != string::npos
               || tempNames.find(".names gnd") != string::npos
               || tempNames.find(".names unconn") != string::npos)
               tempNames = "";
            names2 << tempNames;
        }
    }
    vector<string> variables1 = getFirstVariables(names1.str());
    vector<string> variables2 = getFirstVariables(names2.str());
    vector<string> commonVariables;
    unordered_set<string> set1(variables1.begin(), variables1.end());
    for(const auto& str : variables2){
        if(set1.count(str) > 0){
            commonVariables.push_back(str);
        }
    }
    vector<string> modifiedVariables = commonVariables;
    for(auto& v : modifiedVariables){
        int suffix = 1;
        if(v == "gnd" || v == "vcc" || v == "unconn")
            continue;
        while(find(variables1.begin(), variables1.end(), v) != variables1.end() ||
        find(variables2.begin(), variables2.end(), v) != variables2.end() ||
        Tool::hasDuplicate(modifiedVariables)){
            if(isdigit(v[v.length()-1])){
                size_t pos = v.find_last_not_of("0123456789");
                if(pos != string::npos) {
                    size_t numericSuffixStart = pos + 1;
                    size_t numericSuffixLength = v.size() - numericSuffixStart;
                    string numericSuffix = v.substr(numericSuffixStart, numericSuffixLength);
                    suffix = stoi(numericSuffix) + 1;
                    v = v.substr(0, pos+1);
                    string completeSuffix = to_string(suffix);
                    if(numericSuffix[0] == '0' && numericSuffixLength != 1){
                        size_t count = numericSuffix.find_first_not_of('0');
                        count = (count == std::string::npos) ? numericSuffix.length() : count;
                        string leadingZeros(count, '0');
                        completeSuffix = leadingZeros + completeSuffix;
                    }
                    v += completeSuffix;
                }
            }
            else{
                v += to_string(suffix);
            }
        }
    }
    stringstream newLatch2;
    stringstream newsubckt2;
    stringstream newNames2;
    while(getline(latch2, line)){
        for(size_t i = 0; i < commonVariables.size(); i++){
            size_t pos = line.find(commonVariables[i]);
            while(pos != string::npos){
                char nextChar = (pos + commonVariables[i].length() < line.length()) ? line[pos + commonVariables[i].length()] : ' ';
                if(!isdigit(nextChar)){
                    line.replace(pos, commonVariables[i].length(), modifiedVariables[i]);
                }
                pos = line.find(commonVariables[i], pos + modifiedVariables[i].length());
            }
        }
        newLatch2 << line << '\n';
    }
    while(getline(subckt2, line)){
        for(size_t i = 0; i < commonVariables.size(); i++){
            size_t pos = line.find(commonVariables[i]);
            while(pos != string::npos){
                char nextChar = (pos + commonVariables[i].length() < line.length()) ? line[pos + commonVariables[i].length()] : ' ';
                if(!isdigit(nextChar)){
                    line.replace(pos, commonVariables[i].length(), modifiedVariables[i]);
                }
                pos = line.find(commonVariables[i], pos + modifiedVariables[i].length());
            }
        }
        newsubckt2 << line << '\n';
    }
    while(getline(names2, line)){
        for(size_t i = 0; i < commonVariables.size(); i++){
            size_t pos = line.find(commonVariables[i]);
            while(pos != string::npos){
                char nextChar = (pos + commonVariables[i].length() < line.length()) ? line[pos + commonVariables[i].length()] : ' ';
                if(!isdigit(nextChar)){
                    line.replace(pos, commonVariables[i].length(), modifiedVariables[i]);
                }
                pos = line.find(commonVariables[i], pos + modifiedVariables[i].length());
            }
        }
        newNames2 << line << '\n';
    }
    result << latch1.str();
    result << newLatch2.str();
    result << subckt1.str();
    result << newsubckt2.str();
    result << names1.str();
    result << newNames2.str();
    result << ".end";
    result << '\n';
    result << '\n';

    //write into file
    if(testDirectory != "")
        testDirectory += "/";
    ofstream outputFile(testDirectory + combinedFilename + ".pre-vpr.blif");
    if(!outputFile.is_open()){
        Tool::error("Error creating output file: " + combinedFilename);
        return "";
    }
    else{
        Tool::logMessage("Create file " + testDirectory  +  combinedFilename +  ".pre.blif" + " successfully.\n");
    }
    result << models.str();
    outputFile << result.str();
    return combinedFilename  + ".pre-vpr.blif";
}

vector<string> VTRDrive::getBlockNames(string content){
    istringstream iss(content);
    string line;
    vector<string> blockNames;
    while(getline(iss, line)) {
        istringstream lineStream(line);
        string blockName;
        lineStream >> blockName;
        blockNames.push_back(blockName);
    }
    return blockNames;
}

bool VTRDrive::isInput(string line, int arraySizeX, int arraySizeY){
    istringstream line_stream(line);
    string block_name;
    int x = -1;
    int y = -1;
    int subblk = -1;
    line_stream >> block_name >> x >> y >> subblk;
    if(x == 0 || x == arraySizeX - 1 || y == 0 || y == arraySizeY - 1) return true;
    else return false;
}

stringstream VTRDrive::getModel(string filepath, string modelName){
    ifstream file(filepath);
    string line;
    bool insideModel = false;
    stringstream model;
    while(getline(file, line)){
        if (line.find(".model " + modelName) != string::npos) insideModel = true;
        if (insideModel) {
            model << line << "\n";
            if (line.find(".end") != string::npos) {
                insideModel = false;
                break;
            }
        }
    }
    file.close();
    return model;
}

vector<string> VTRDrive::getAllModelNames(string blif){
    regex pattern("\\.model\\s+(\\w+)");
    sregex_iterator it(blif.begin(), blif.end(), pattern);
    sregex_iterator end;
    vector<string> modelNames;
    while (it != end) {
        modelNames.push_back((*it)[1]);
        ++it;
    }
    return modelNames;
}

vector<string> VTRDrive::getFirstVariables(string names) {
    vector<string> line = Tool::split(names, '\n');
    vector<string> variables;
    bool hasNextLine = false;
    for(string& l : line){
        vector<string> seg = Tool::split(l, ' ');
        if(seg[0] == ".names" || hasNextLine){
            hasNextLine = seg[seg.size() - 1] == "\\";
            if(!hasNextLine){
                variables.push_back(seg[seg.size() - 2]);
            }
        }
    }
    return variables;
}

double VTRDrive::getPartWrieLength(string partCirName, RouteAnalyzer allRA, string deleteWireOut, int outX, int outY, int pad){
    //Collect all route in partRA
    vector<RouteInfo*> routeList = allRA.getRouteList();
    string flagName = partCirName + "^";
    vector<int> usedNodeIDs;
    double wireLength = 0;
    for(RouteInfo* route : routeList){
        if(route->netName.find(flagName) != string::npos){
            if(route->netName == deleteWireOut)
                if(!(route->sinkLoc.first == outX && route->sinkLoc.second == outY && route->sinkPtcPad == pad))
                    continue;
            vector<Segment*> sl = route->segments;
            for(Segment* s : sl){
                if(find(usedNodeIDs.begin(), usedNodeIDs.end(), s->nodeID) == usedNodeIDs.end()){
                    if(s->end.first == 0 && s->end.second == 0)
                        wireLength += 1;
                    else
                        wireLength += Tool::twoPointsDistance(s->start, s->end) + 1;
                    usedNodeIDs.push_back(s->nodeID);
                }
            }
        }
    }
    return wireLength;
}

string VTRDrive::getFeaturesFromLog(string vproutPath){
    //io nums, blocks, total_nets, fanout_ave, fanout_max and occ
    string result = "";
    string content = Tool::readFile(vproutPath);
    content = Tool::washString(content);
    double inputNum = VTRDrive::getValueBehindStr(content, ".input :");
    double outputNum = 0.0;
    if(content.find(".output :") != string::npos)
        outputNum = VTRDrive::getValueBehindStr(content, ".output :");
    else
        outputNum = VTRDrive::getValueBehindStr(content, ".output:");
    string io_nums = to_string(inputNum + outputNum);
    string blocks = to_string(VTRDrive::getValueBehindStr(content, "Circuit Statistics: Blocks:"));
    string total_nets = to_string(VTRDrive::getValueBehindStr(content, "Nets :"));
    string fanout_ave = to_string(VTRDrive::getValueBehindStr(content, "Avg Fanout:"));
    string fanout_max = to_string(VTRDrive::getValueBehindStr(content, "Max Fanout:"));
    string occ = to_string(VTRDrive::getValueBehindStr(content, "Maximum routing channel utilization:"));

    result += io_nums + "," + blocks + "," + total_nets + "," + fanout_ave + "," + fanout_max;
    
    return result;
}

bool VTRDrive::timingRptCom(string runDirectory)
{
    string setupPath = runDirectory + "/report_timing.setup.rpt";
    string holdPath = runDirectory + "/report_timing.hold.rpt";
    if(!Tool::isFileExists(setupPath) || !Tool::isFileExists(holdPath)){
        Tool::error("Error: timingRptCom cannot find all report_timing.xxx.rpt from " + runDirectory);
    }
    string contentSetup = Tool::readFile(setupPath);
    string contentHold = Tool::readFile(holdPath);
    string newRptContent = contentSetup + contentHold;
    string newRptPath = runDirectory + "/report_timing.rpt";
    return Tool::fileWrite(newRptPath, newRptContent);
}

string VTRDrive::writeSDCforExec(string programPath, string outputDir)
{
    string sdcResult = "create_clock -period 5 *\n";
    string programName = Tool::findFilefromPath(programPath);
    programName = programName.substr(0, programName.find("."));
    string programContent = Tool::readFile(programPath);
    vector<string> lines = Tool::split(programContent, '\n');
    vector<string> clks;
    string repeatStr = "";
    for(string line : lines){
        vector<string> items = Tool::split(line, ' ');
        for(int i = 0; i < items.size(); i++){
            string item = items[i];
            if(item == "(posedge" || item == "posedge" || item == ",posedge" || item == "@(posedge"
              || item == "(negedge" || item == "negedge" || item == ",negedge" || item == "@(negedge"){
                i++;
                string clk = items[i];
                string blifName = programName + "^";
                if(clk.find("[") != string::npos && clk.find("]") != string::npos){
                    int leftIndex = clk.find("[");
                    string clkName = clk.substr(0, leftIndex);
                    int index = Tool::getNumFromClose(clk);
                    blifName += clkName + "~" + to_string(index);
                }
                else{
                    while(clk.back() == ')')
                        clk.pop_back();
                    blifName += clk;
                }
                if(repeatStr.find(blifName + ";") == string::npos){
                    clks.push_back(blifName);
                    repeatStr += blifName + ";";
                }
            }
        }
    }
    if(clks.size() == 0){
        sdcResult = "create_clock -period 5 -name virtual_PRUMR\n";
    }
    else{
        for(string clk : clks){
            sdcResult += "set_input_delay -clock " + clk + " -max 5 [get_ports {*}]\n";
            sdcResult += "set_output_delay -clock " + clk + " -max 5 [get_ports {*}]\n";
        }
    }
    if(!Tool::isDirectoryExists(outputDir)){
        vector<string> contentLevels = Tool::split(outputDir, '/');
        string recentPath = "";
        for(string content : contentLevels){
            if(recentPath != "")
                recentPath += "/" + content;
            else
                recentPath += content;
            Tool::createDirectory(recentPath);
        }
    }

    string sdcPath = outputDir + "/" + programName +".sdc";
    Tool::fileWrite(sdcPath, sdcResult);
    return sdcPath;
}

string VTRDrive::writeSDCforBlif(string blifPath, string outputDir)
{
    string sdcResult = "create_clock -period 5 *\n";
    vector<string> clks;

    BlifAnalyzer ba(blifPath);

    vector<Latch> candidateCLKs = ba.latchs;
    string repeatStr = "";
    for(Latch cand : candidateCLKs){
        string clk = cand.control;
        if(repeatStr.find(clk + ";") == string::npos){
            clks.push_back(clk);
            repeatStr += clk + ";";
        }
    }

    if(clks.size() == 0){
        sdcResult = "create_clock -period 5 -name virtual_PRUMR\n";
    }
    else{
        for(string clk : clks){
            sdcResult += "set_input_delay -clock " + clk + " -max 5 [get_ports {*}]\n";
            sdcResult += "set_output_delay -clock " + clk + " -max 5 [get_ports {*}]\n";
        }
    }

    if(!Tool::isDirectoryExists(outputDir)){
        vector<string> contentLevels = Tool::split(outputDir, '/');
        string recentPath = "";
        for(string content : contentLevels){
            if(recentPath != "")
                recentPath += "/" + content;
            else
                recentPath += content;
            Tool::createDirectory(recentPath);
        }
    }

    string sdcPath = outputDir + "/constraint.sdc";
    Tool::fileWrite(sdcPath, sdcResult);
    return sdcPath;
}
