#include "UserDrive.h"
#include "Tool.h"

UserDrive::UserDrive()
{
    //work path confirm:
    char* workPathDir = getenv("VGENERATOR_WORK_DIR");
    if(workPathDir == nullptr || workPathDir == ""){
        Tool::error("Error: VGENERATOR_WORK_DIR not found. Please check env.");
    }
    Tool::logMessage("Work dir: VGENERATOR_WORK_DIR=" + string(workPathDir));
};

bool UserDrive::loadHDLSet(int num, int stateIter, string setPathName, string sdc_type, string writeType, int varMaxW, int deepLevel, double conRatio, bool activeCheck)
{
    Py_Initialize();
    this->varMaxW = varMaxW;
    if(num < 1)
        Tool::error("Error: Num is not positive: " + to_string(num));
    else if(stateIter < 1)
        Tool::error("Error: stateIter is not positive: " + to_string(stateIter));
    
    //Check if the program set is existing.
    //Check initial program path.
    string setName = Tool::findFilefromPath(setPathName);

    if(!Tool::isDirectoryExists(setPathName) || !Tool::isDirectoryExists(setPathName + "/init_hdl_program") || !Tool::isDirectoryExists(setPathName + "/hdl_tree")){
        Tool::createDirectory(setPathName);
        initHDLSetContent(setPathName);
        //Generate new HDL set
        generateHDLinitSet(setName, setPathName, num, activeCheck, stateIter, varMaxW, sdc_type);
    }

    //HDL Construct
    Constructor constructor(setPathName);
    for(int i = 0; i < deepLevel; i++){
        constructor.buildTree(setName, i + 1, "random", writeType, activeCheck, 0.5);
    }
    Py_Finalize();

    return true;
}

bool UserDrive::initHDLSetContent(string setPath){
    bool result = true;
    string initProgramPath = setPath + "/init_hdl_program";
    string treeProgramPath = setPath + "/hdl_tree";
    result &= Tool::createDirectory(initProgramPath);
    result &= Tool::createDirectory(treeProgramPath);
    return result;
}

bool UserDrive::generateHDLinitSet(string setName, string setPath, int num, bool activeCheck, int stateIter, int varMaxW, string sdc_type){
    int seed;
    int addCommand = 0;
    int stateAdjust = 0;
    int rejectTimes = 0;
    int acceptTimes = 0;
    bool ifIncrement = true;
    if(staticAdjustIOs != 0 || staticAdjustStates != 0){
        stateAdjust = staticAdjustStates;
        addCommand = staticAdjustIOs;
        ifIncrement = false;
    }
    for(int init_index = 0; init_index < num; init_index++){
        seed = rand();
        srand(seed);
        cout << "dealing seed: " << seed << endl;
        string recentName = setName + to_string(init_index);
        string vName = recentName + ".v";
        Builder build(recentName, addCommand, varMaxW);
        build.setGenerateType(generateType);
        int i;
        for(i = 0; i < 20; i++){
            build.randomAddStatement();
        }
        for(; i < stateIter * 1.2 + stateAdjust; i++){
            build.randomAddStatement();
        }
        int inValid = build.checker(0, stateIter);

        if(Tool::split(build.getSummaryCode(), '\n').size() < stateIter ){
            init_index--;
            rejectTimes++;
            acceptTimes = 0;
            if(rejectTimes >= 5 && ifIncrement){
                stateAdjust += 5;
                addCommand++;
            }
            continue;
        }
        else if(rejectTimes >= 5){
            rejectTimes = 4;
        }
        else if(rejectTimes > 0){
            rejectTimes--;
        }

        acceptTimes++;
        if(acceptTimes == 5 && ifIncrement){
            stateAdjust -= 5;
            addCommand--;
            acceptTimes = 0;
        }

        while(setPath.back() == '/' || setPath.back() == '.')
            setPath = setPath.substr(0, setPath.size() - 1);
        
        if(!inValid){
            string savePath = setPath + "/init_hdl_program/" + vName;
            Tool::logMessage("Program " + vName + " saved in " + setPath + "/init_hdl_program.");
            Tool::fileWrite(setPath + "/init_hdl_program/" + vName, build.getSummaryCode());
            if(activeCheck){
                stringstream ss = Tool::getModule(setPath + "/init_hdl_program/" + vName, recentName);
                vector<Attribute*> outputs, inputs, inouts;
                Tool::extractInoutfromTextV(ss, outputs, inputs);
                inouts.insert(inouts.end(), outputs.begin(), outputs.end());
                inouts.insert(inouts.end(), inputs.begin(), inputs.end());
                vector<Attribute> newInout;
                for(auto it = inouts.begin(); it < inouts.end(); it++){
                    Attribute a = *(*it);
                    newInout.push_back(a);
                }
                Executer::generateTbandVerify(recentName, newInout, setPath + "/init_hdl_program/");
            }
            build.info();
            Tool::logMessage("seed: " + to_string(seed));
            Tool::logMessage("IOs add adjust: " + to_string(addCommand));
            Tool::logMessage("Statements add adjust: " + to_string(stateAdjust));
            Tool::logMessage("\n");
        }
        else{
            Tool::logMessage("Program " + vName + " checked failed.\n");
        }
        remove(vName.c_str());
    }
    return true;
}

bool UserDrive::VTRrunTesting(string vtrPath, string treeHDLPath, vector<string> MRs, int turns, string diverV){
    if(archPath == ""){
        Tool::error("Error: UserDrive haven't loaded Arch.");
        return false;
    }
    VTRDrive vtr(vtrPath);

    //Start Testing
    //Create dir in the program set
    string productPath = treeHDLPath + "/product";
    Tool::createDirectory(productPath);
    string lastProgramPathandName = "";
    string lastProgramName = "";
    vector<string> usedPrograms;
    double maxDistance = 0.0;
    Py_Initialize();
    string errorReportPath = "Error_reports/Error_report_" + Tool::getLogFileTime();
    string errorProductPath = "singleVerilogError/singleVerilogError_" + Tool::getLogFileTime();
    Tool::createDirectory("Error_reports");
    Tool::createDirectory("DefectTestCases");
    Tool::createDirectory("singleVerilogError");
    Tool::createDirectory(errorProductPath);
    Tool::createDirectory(errorReportPath);
    int allCaseNum = 0;
    double defect = 0;
    string defectTestDirectory = "DefectTestCases/DefectTestCase_" + Tool::getLogFileTime();
    for(int i = 0; i < turns; i++){
        vector<string> programsPath;
        vector<string> allPath;
        vector<string> programsName;
        Tool::getAllFilefromDir(treeHDLPath, allPath);
        for(auto it = allPath.begin(); it < allPath.end(); it++){
            string recentPath = *it;
            string filename = Tool::findFilefromPath(recentPath);
            if(filename.find(".") == string::npos){ //product or other directory, delete the path in programsPath
                continue;
            }
            string programName = "";
            int bound = recentPath.rfind('/');
            string archName = "";
            if(bound == string::npos){
                programName = recentPath.substr(0, recentPath.find('.'));
            }
            else{
                programName = recentPath.substr(bound + 1, recentPath.size() - bound - 1);
                programName = programName.substr(0, programName.find('.'));
            }
            if(find(usedPrograms.begin(), usedPrograms.end(), recentPath) == usedPrograms.end()){
                programsName.push_back(programName);
                programsPath.push_back(recentPath);
            }
        }


        Tool::logMessage("Seletct Program Turn: " + to_string(i + 1) + ".");
        int randIndex = -1;
        if(lastProgramName == ""){
            randIndex = Tool::getRandomfromClosed(0, programsName.size() - 1);
        }
        else{
            //Random choose 5 seed program.
            vector<int> randomSelectedIndex;
            if(programsName.size() <= 1){
                Tool::logMessage("UserDrive: Too less programs left, break. Program number: " + to_string(programsName.size()));
                break;
            }
            else if(programsName.size() <= 5){
                for(int j = 0; j < programsName.size(); j++){
                    randomSelectedIndex.push_back(j);
                }
            }
            else{
                for(int j = 0; j < 5; j++){
                    int recentIndex = Tool::getRandomfromClosed(0, programsName.size() - 1);
                    if(find(randomSelectedIndex.begin(), randomSelectedIndex.end(), recentIndex) == randomSelectedIndex.end())
                        randomSelectedIndex.push_back(recentIndex);
                    else
                        j--;
                }
            }
            Tool::logMessage("choose programs: " + to_string(randomSelectedIndex.size()) + ".");
            string lastGraph = VarietyEvaluation::fromVtoFormularG(lastProgramPathandName, lastProgramName);
            for(int selectIndex = 0; selectIndex < randomSelectedIndex.size(); selectIndex++){
                int recentIndex = randomSelectedIndex[selectIndex];
                string recentProgramName = programsName[recentIndex];
                string recentProgramPathandName = programsPath[recentIndex];
                string recentGraph = VarietyEvaluation::fromVtoFormularG(recentProgramPathandName, recentProgramName);
                int numThisNodes = Tool::split(recentGraph, ';').size();
                double recentDistance = Evaluate::graphEditDistance(lastGraph, recentGraph, 5) 
                                + Evaluate::getJaccard(lastProgramPathandName, recentProgramPathandName, 1);
                if(recentDistance > maxDistance){
                    maxDistance = recentDistance;
                    randIndex = recentIndex;
                    Tool::logMessage("Program " + to_string(selectIndex + 1) + " Name: " + recentProgramName + " distance: " +to_string(recentDistance) + "; accept.");
                }
                else{
                    Tool::logMessage("Program " + to_string(selectIndex + 1) + " Name: " + recentProgramName + " distance: " +to_string(recentDistance) + "; reject.");
                }
            }
        }
        double finalDistance = maxDistance;
        maxDistance = 0.0;
        string programName = programsName[randIndex];
        string programPath = programsPath[randIndex];

        usedPrograms.push_back(programPath);
        Tool::logMessage("Final program: " + programName + "; program distance: " + to_string(finalDistance) + ".");

        if(diverV == "True"){
            Tool::logMessage("The log is used to diversity-guided evaluation, skip MT.");
            lastProgramName = programName;
            lastProgramPathandName = programPath;
            continue;
        }

        //Check if the program has been executed.
        string programProductPath = productPath + "/" + programName;
        if(!Tool::isDirectoryExists(programProductPath)){
            Tool::createDirectory(programProductPath);
            string isSucc = vtr.executeRunFlow(programPath, archPath, programProductPath, programName);
            if(isSucc != "success"){
                string errorReportName = "VTR_crash_" + programName;
                string errorReport = "State 0: Error program detect: " + programProductPath + "/" + programName + ".v\n";
                errorReport += "Program path: " + programPath + "\n";
                errorReport += "Concrete error: " + isSucc;
                Tool::logMessage(errorReport);
                if(Tool::isFileExists(programProductPath)){
                    string cp_r_comand = "cp -r " + programProductPath + " " + errorProductPath + ";rm -r " + programProductPath + ";rm " + programPath;
                    system(cp_r_comand.c_str());
                }
                else
                    errorReport += ". File lost.";
                Tool::fileWrite(errorReportPath + "/" + errorReportName + ".rpt", errorReport);
                i--;
                allCaseNum++;
                defect += 1;
                Tool::logMessage("All cases num: " + to_string(allCaseNum));
                Tool::logMessage("Defect: " + to_string((int)defect));
                Tool::logMessage("False detection rate: " + to_string(defect / allCaseNum));
                continue;
            }
        }
        else{
            Tool::logMessage("Directory exist, waiting for report_timing.rpt generated.");
            bool isWaitSucc = true;
            while(!Tool::isFileExists(programProductPath + "/report_timing.rpt")){
                if(!Tool::isDirectoryExists(programProductPath)){
                    string errorReportName = "VTR_crash_" + programName;
                    string errorReport = "State 0: Error program detect: " + programProductPath + "/" + programName + ".v\n";
                    errorReport += "Program path: " + programPath + "\n";
                    errorReport += "Concrete error: product lost";
                    Tool::logMessage(errorReport);
                    Tool::fileWrite(errorReportPath + "/" + errorReportName + ".rpt", errorReport);
                    i--;
                    allCaseNum++;
                    defect += 1;
                    Tool::logMessage("All cases num: " + to_string(allCaseNum));
                    Tool::logMessage("Defect: " + to_string((int)defect));
                    Tool::logMessage("False detection rate: " + to_string(defect / allCaseNum));
                    isWaitSucc = false;
                    break;
                }
            }
            if(!isWaitSucc)
                continue;
            Tool::logMessage("report_timing.rpt got, all file generated.");
        }
        //Execute MRVerify module according to different comand.
        MRManager mrm(MRs, vtr, programName, treeHDLPath, programsName, *arch, errorReportPath);
        bool isSucc =  mrm.MRVerify(i + 1, allCaseNum, defect, defectTestDirectory);
        if(isSucc){
            lastProgramName = programName;
            lastProgramPathandName = programPath;
        }
        Tool::logMessage("All cases num: " + to_string(allCaseNum));
        Tool::logMessage("Defect: " + to_string((int)defect));
        Tool::logMessage("False detection rate: " + to_string(defect / allCaseNum) + "\n");
    }
    Py_Finalize();
    Tool::logMessage("\nTest finish.");

    return true;
}

string UserDrive::generateAProgram(string programName, int stateIter, int varMaxW){
    Builder build(programName, varMaxW);
    for(int i = 0; i < stateIter; i++){
        build.randomAddStatement();
    }
    int inValid = build.checker(100, stateIter);
    build.info();
    return build.getSummaryCode();
}

bool UserDrive::VTRFeatureExtract(string treeHDLPath, vector<string> MRs, int judgeBound, bool if_remove_high_leverage, bool testAnalysis, string extractFeature){
    string productDirectory = treeHDLPath + "/product";
    string featureDirectory = treeHDLPath + "/features_" + Tool::getRecentTime();
    string testFatherDirectory = treeHDLPath + "/Test";

    for(int i = 0; i < MRs.size(); i++){
        if(MRs[i] == "MRConsistLayout")
            MRs[i] = "test_consistLayout";
        else if(MRs[i] == "MRAddRouteConsist")
            MRs[i] = "test_addRouteConsist";
    }

    if(extractFeature == ""){
        vector<string> featureTab = {"test_name", "test_seq", "MR", "main_blif", "io_nums", "blocks", "total_nets", "fanout_ave", 
                                    "fanout_max", "logic_level", "overlen_wire", "prefer_ratio", "wire_ratio", "slack_delta"};
        string tabLine = "";
        for(string tab : featureTab)
            tabLine += tab + ",";
        tabLine.pop_back();
        

        //content create
        Tool::createDirectory(featureDirectory);
        string content = tabLine;

        //traverse all test content
        vector<string> testNVec;
        Tool::getAllFilefromDir(testFatherDirectory, testNVec);
        int seq = 0;
        for(string testNDirectory : testNVec){
            string testN = Tool::findFilefromPath(testNDirectory);
            vector<string> testMRs;
            Tool::getAllFilefromDir(testNDirectory, testMRs);
            for(string testMRDirectory : testMRs){
                string testMR = Tool::findFilefromPath(testMRDirectory);
                if(find(MRs.begin(), MRs.end(), testMR) != MRs.end()){
                    vector<string> testInfos;
                    Tool::getAllFilefromDir(testMRDirectory, testInfos);
                    for(string testInfoDirectory : testInfos){
                        string testInfo = Tool::findFilefromPath(testInfoDirectory);

                        //get two module
                        string testReportDirectory = testInfoDirectory + "/test.rpt";
                        if(!Tool::isFileExists(testReportDirectory))
                            continue;
                        vector<string> rptLines = Tool::split(Tool::readFile(testReportDirectory), '\n');
                        string module1 = "";
                        string module2 = "";
                        for(string rptLine : rptLines){
                            if(rptLine.find("Program 1:") != string::npos){
                                module1 = Tool::findFilefromPath(rptLine);
                                if(module1.back() == '.')
                                    module1.pop_back();
                            }
                            else if(rptLine.find("Program 2:") != string::npos){
                                module2 = Tool::findFilefromPath(rptLine);
                                if(module2.back() == '.')
                                    module2.pop_back();
                                break;
                            }
                        }
                        string vprOutPath = testInfoDirectory + "/vpr.out";
                        string logic_level = "";

                        string long_wire_num = "invalid";
                        string prefer_ratio = "invalid";
                        string wire_ratio = "invalid";
                        string slack_delta = "invalid";
                        string mainModule = "";
                        
                        //confirm main module and get ratios
                        TimingAnalyzer mainTA;
                        if(testMR == "test_consistLayout"){
                            string testReportPath = testInfoDirectory + "/test.rpt";
                            string rptContent = Tool::washString(Tool::readFile(testReportPath));
                            DelayandArea module1DA = VTRDrive::getCritDelayandAreafromLog(productDirectory + "/" + module1 + "/vpr_stdout.log");
                            DelayandArea module2DA = VTRDrive::getCritDelayandAreafromLog(productDirectory + "/" + module2 + "/vpr_stdout.log");
                            if(module1DA.delay >= module2DA.delay){
                                mainModule = module1;
                            }
                            else{
                                mainModule = module2;
                            }
                            string mainRptPath = productDirectory + "/" + mainModule + "/report_timing.rpt";
                            if(Tool::isFileExists(mainRptPath))
                                mainTA.loadTimingRpt(mainRptPath);
                            else
                                Tool::error("Error: FeatureAnalyze cannot find: " + mainRptPath);

                            if(Tool::isFileExists(testReportPath)){
                                //symbol revise
                                int program2Index = rptContent.find("Program 2");
                                int combineIndex = rptContent.find("Combine");
                                string program1 = rptContent.substr(0, program2Index);
                                string program2 = rptContent.substr(program2Index, combineIndex - program2Index);
                                string combinePart = rptContent.substr(combineIndex, rptContent.size() - combineIndex);

                                double program1Delay = VTRDrive::getValueBehindStr(program1, "Delay:");
                                double program2Delay = VTRDrive::getValueBehindStr(program2, "Delay:");
                                double combineDelay = VTRDrive::getValueBehindStr(combinePart, "Delay:");
                                double program12WireSum = VTRDrive::getValueBehindStr(program2, "Wire length sum:");
                                double combineWire = VTRDrive::getValueBehindStr(combinePart, "Wire length:");

                                double prefer_value = VTRDrive::getValueBehindStr(rptContent, "Compare prefer ratio:");
                                double wire_value = VTRDrive::getValueBehindStr(rptContent, "Wire prefer ratio:");
                                slack_delta = to_string(VTRDrive::getValueBehindStr(rptContent, "Slack delta:"));
                                prefer_ratio = to_string((max(program1Delay, program2Delay) - combineDelay) / max(program1Delay, program2Delay));
                                wire_ratio = to_string((program12WireSum - combineWire) / program12WireSum);
                                
                            }
                            else
                                continue;
                        }
                        else if(testMR == "test_addRouteConsist"){
                            string module1Path = productDirectory + "/" + module1;
                            mainModule = module1;

                            string testReportPath = testInfoDirectory + "/test.rpt";
                            string mainRptPath = productDirectory + "/" + mainModule + "/report_timing.rpt";
                            if(Tool::isFileExists(mainRptPath))
                                mainTA.loadTimingRpt(mainRptPath);
                            else
                                Tool::error("Error: FeatureAnalyze cannot find: " + mainRptPath);

                            if(Tool::isFileExists(testReportPath)){
                                string rptContent = Tool::washString(Tool::readFile(testReportPath));
                                prefer_ratio = to_string(VTRDrive::getValueBehindStr(rptContent, "Compare prefer ratio:") );
                                wire_ratio = to_string(VTRDrive::getValueBehindStr(rptContent, "Wire prefer ratio:"));
                                slack_delta = to_string(VTRDrive::getValueBehindStr(rptContent, "Slack delta:"));
                            }
                            else
                                continue;
                        }

                        logic_level = to_string(mainTA.critPath->logic_levels);

                        string mainModuleProductPath = productDirectory + "/" + mainModule;
                        string logFeatures = VTRDrive::getFeaturesFromLog(mainModuleProductPath + "/vpr.out");
                        
                        //long wire num statictics
                        int overlen_wire = 0;
                        // PlaceAnalyzer pa(mainModuleProductPath + "/" + mainModule + ".place");
                        // double longWireBound = ((double)pa.getSize().first + pa.getSize().second) / 4;
                        // RouteAnalyzer ra;
                        // ra.loadRoute(mainModuleProductPath + "/" + mainModule + ".route");
                        // vector<RouteInfo*> routes = ra.getRouteList();
                        // for(RouteInfo* ri : routes){
                        //     int manhadun = abs(ri->sourceLoc.first - ri->sinkLoc.first) + abs(ri->sourceLoc.second - ri->sinkLoc.second);
                        //     if(manhadun >= longWireBound)
                        //         overlen_wire++;
                        // }
                        // long_wire_num = to_string(overlen_wire);

                        string recentLine = testInfo + "," + testN + "," + testMR + "," + mainModule + "," + logFeatures + "," 
                                            + logic_level + "," + long_wire_num + "," + prefer_ratio + "," + wire_ratio + "," + slack_delta;
                        content += "\n" + recentLine;
                        seq++;
                        Tool::logMessage("Test Feature Extract: " + to_string(seq));
                    }
                }
            }
            Tool::fileWrite(featureDirectory + "/features.csv", content);
        }
    }
    else{
        //use exist file to analyze
        Tool::createDirectory(featureDirectory);
        string cpCommand = "cp " + extractFeature + " " + featureDirectory;
        system(cpCommand.c_str());
    }

    //Data Analysis
    if(testAnalysis){
        Tool::logMessage("Python env initializing...");
        Py_Initialize();
    }
    if (!Py_IsInitialized()) {
        cout << "python initialize failed!" << endl;
        return 0;
    }
    //run without cmake please replace this line
    string pythonCode = string("import sys\nsys.path.append('") + getenv("VGENERATOR_WORK_DIR") + "/lib')\nimport pyximport\npyximport.install()";
    // string pythonCode = string("import sys\nsys.path.append('./lib')\nimport pyximport\npyximport.install()");
    PyRun_SimpleString(pythonCode.c_str());
    PyObject* pModule = PyImport_ImportModule("eda_data_analysis");
    if (pModule == NULL) {
        PyErr_Print();
        Tool::error("load eda_data_analysis fail. Please check the env.");
        Py_XDECREF(pModule);
        return 0;
    }
    else {
        PyObject* pFunc = PyObject_GetAttrString(pModule, "eda_data_analysis");
        PyObject* pyValue;
        if (pFunc && PyCallable_Check(pFunc)) {
            PyObject* pArgs = PyTuple_New(4);
            string filepath = featureDirectory + "/features.csv";
            string output_path = featureDirectory;
            PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(filepath.c_str()));
            PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(output_path.c_str()));
            PyTuple_SetItem(pArgs, 2, PyLong_FromLong(judgeBound));
            PyTuple_SetItem(pArgs, 3, PyBool_FromLong(if_remove_high_leverage));
            PyTuple_SetItem(pArgs, 4, PyBool_FromLong(0));

            pyValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if (pyValue != nullptr) {
                Py_DECREF(pyValue);
            } 
            else {
                PyErr_Print();
            }
        } 
        else {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Cannot find function 'test_function'\n");
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    if(testAnalysis)
        Py_Finalize();

    return true;
}

bool UserDrive::arrangeData(string treeHDLPath, string targetDir, vector<string> dataTypes)
{
    bool allTypes = dataTypes.size() == 0;
    if(!Tool::isDirectoryExists(targetDir))
        Tool::createDirectory(targetDir);
    vector<string> commands;
    if(allTypes || find(dataTypes.begin(), dataTypes.end(), "singleVerilogError") != dataTypes.end()){
        string targetPath = targetDir + "/singleVerilogError";
        string cmd = "mkdir " + targetPath + ";mv singleVerilogError/singleVerilogError* " + targetPath;
        commands.push_back(cmd);
    }
    if(allTypes || find(dataTypes.begin(), dataTypes.end(), "ErrorReports") != dataTypes.end()){
        string targetPath = targetDir + "/Error_reports";
        string cmd = "mkdir " + targetPath + ";mv Error_reports/Error_report* " + targetPath;
        commands.push_back(cmd);
    }
    if(allTypes || find(dataTypes.begin(), dataTypes.end(), "setout") != dataTypes.end()){
        string targetPath = targetDir + "/setout";
        string cmd = "mkdir " + targetPath + ";mv setout/*.* " + targetPath;
        commands.push_back(cmd);
    }
    if(allTypes || find(dataTypes.begin(), dataTypes.end(), "DefectCases") != dataTypes.end()){
        string targetPath = targetDir + "/defectCases";
        string cmd = "mkdir " + targetPath + ";mv DefectTestCases/DefectTestCase* " + targetPath;
        commands.push_back(cmd);
    }
    if(allTypes || find(dataTypes.begin(), dataTypes.end(), "product") != dataTypes.end()){
        string cmd = "mv " + treeHDLPath + "/product " + targetDir;
        commands.push_back(cmd);
    }
    if(allTypes || find(dataTypes.begin(), dataTypes.end(), "Test") != dataTypes.end()){
        string cmd = "mv " + treeHDLPath + "/Test " + targetDir;
        commands.push_back(cmd);
    }
    if(allTypes || find(dataTypes.begin(), dataTypes.end(), "features") != dataTypes.end()){
        string targetPath = targetDir + "/features";
        string cmd = "mkdir " + targetPath + ";mv " + treeHDLPath + "/feature* " + targetPath;
        commands.push_back(cmd);
    }
    if(allTypes || find(dataTypes.begin(), dataTypes.end(), "log") != dataTypes.end()){
        string targetPath = targetDir + "/log";
        string cmd = "mkdir " + targetPath + ";mv *.log " + targetPath;
        commands.push_back(cmd);
    }

    //performance case arrange
    if(allTypes || find(dataTypes.begin(), dataTypes.end(), "performance") != dataTypes.end()){
        Tool::createDirectory(targetDir + "/performance");
        vector<string> allDirsUnderTree;
        Tool::getAllFilefromDir(treeHDLPath, allDirsUnderTree);
        string featurePath = "";
        for(string dir : allDirsUnderTree){
            string fileorDirName = Tool::findFilefromPath(dir);
            if(fileorDirName.find(".") != string::npos)
                continue;
            else if(fileorDirName.find("features") != string::npos){
                featurePath = dir;
            }
        }
        if(featurePath == ""){
            Tool::error("Error: arrangeData cannot find features directory.");
        }
        vector<string> csvs = {"final_outliers_route_test_addRouteConsist.csv",
                               "final_outliers_route_test_consistLayout.csv",
                               "final_outliers_test_addRouteConsist.csv",
                               "final_outliers_test_consistLayout.csv"};
        for(string csv : csvs){
            string csvPath = featurePath + "/" + csv;
            Tool::logMessage("performance csv: " + csvPath);
            string csvContent = Tool::readFile(csvPath);
            vector<string> lines = Tool::split(csvContent, '\n');
            for(string line : lines){
                vector<string> items = Tool::split(line, ',');
                string module1, module2;
                string testName = items[0];
                if(testName == "test_name" || testName == "")
                    continue;
                int charIndex = 0;
                for(int i = 0; i < testName.size(); i++){
                    if(testName[i] >= '0' && testName[i] <= '9')
                        charIndex = i + 1;
                    else if(testName[i] == '_')
                        charIndex = i + 1;
                    else{
                        break;
                    }
                }
                if(charIndex >= testName.size())
                    Tool::error("Error: arrangeData charIndex out of bounds: index " + to_string(charIndex) + ", size " + to_string(testName.size()));
                int _and_index = testName.find("_and_");
                string flag = "_and_";
                if(_and_index == string::npos){
                    _and_index = testName.find("_with_");
                    flag = "_with_";
                }
                module1 = testName.substr(charIndex, _and_index - charIndex);
                module2 = testName.substr(_and_index + flag.size(), testName.size() - _and_index - flag.size());
                string testSeq = items[1];
                string mrType = items[2];
                string testDirectory = treeHDLPath + "/Test/" + testSeq + "/" + mrType + "/" + testName;
                if(!Tool::isDirectoryExists(testDirectory + "/" + module1)){
                    string cmd = "cp -r " + treeHDLPath + "/product/" + module1 + " " + testDirectory;
                    system(cmd.c_str());
                }
                if(!Tool::isDirectoryExists(testDirectory + "/" + module2)){
                    string cmd = "cp -r " + treeHDLPath + "/product/" + module2 + " " + testDirectory;
                    system(cmd.c_str());
                }
                string finalCmd = "cp -r " + testDirectory + " " + targetDir + "/performance/.";
                Tool::logMessage("copy: " + testDirectory);
                system(finalCmd.c_str());
            }
        }
    }

    for(string cmd : commands){
        system(cmd.c_str());
    }

    return true;
}



bool UserDrive::VTRsimpleRun(string treeHDLPath, string archPath, string VTR_ROOT)
{
    string productPath = treeHDLPath + "/test_product";
    vector<string> programsName;
    Tool::getAllFilefromDir(treeHDLPath, programsName);
    Tool::createDirectory(productPath);
    VTRDrive vtr(VTR_ROOT);
    for(string program : programsName){
        string recentProgramFile = Tool::findFilefromPath(program);
        string recentModule = recentProgramFile.substr(0, recentProgramFile.find("."));
        vtr.executeRunFlow(program, archPath, productPath + "/" + recentModule, recentModule);
    }
    return true;
}


bool UserDrive::VTRRTLRandomPerformTest(string treeHDLPath, string rootPath, string output, int turns, int seed)
{
    Tool::logMessage("VTR random performance test begin.");
    Tool::logMessage("seed: " + to_string(seed));
    Tool::logMessage("program set: " + treeHDLPath);
    Tool::createDirectory(output);
    if(archPath == ""){
        Tool::logMessage("Warning: UserDrive haven't loaded Arch. Default arch will be used.");
    }
    VTRDrive vtr = VTRDrive(rootPath);
    string programSetName = Tool::findFilefromPath(treeHDLPath);
    string rootTestDirectory = output;
    Tool::createDirectory(rootTestDirectory);
    string productPath = rootTestDirectory + "/product";
    Tool::createDirectory(productPath);
    string errorReportPath = rootTestDirectory + "/Error_reports";
    string csvPath = rootTestDirectory + "/csv";
    Tool::createDirectory(csvPath);
    int crashNum = 0;
    int performanceNum = 0;
    //Test begin
    srand(seed);
    vector<string> allPath;
    Tool::getAllFilefromDir(treeHDLPath, allPath);
    for(int i = 0; i < turns; i++){
        if(allPath.size() == 0){
            Tool::logMessage("No left program, random performance test end.");
            return true;
        }
        //program select
        string selectProgramName, selectProgramPath;

        int randIndex = Tool::getRandomfromClosed(0, allPath.size() - 1);
        string programPath = allPath[randIndex];
        string filename = Tool::findFilefromPath(programPath);
        string programName = filename.substr(0, filename.find('.'));
        string recentProductPath = productPath + "/" + programName;
        string firstRunPath = recentProductPath + "/first";
        string secondRunPath = recentProductPath + "/second";
        if(!Tool::isDirectoryExists(recentProductPath) 
        || !Tool::isFileExists(firstRunPath + "/report_timing.rpt")
        || !Tool::isFileExists(secondRunPath + "/report_timing.rpt")){
            Tool::createDirectory(recentProductPath);
            Tool::createDirectory(firstRunPath);
            Tool::createDirectory(secondRunPath);
            
            Tool::logMessage("Program " + to_string(i + 1) + ": " + programName);
            
            //random test
            string isSucc = vtr.executeRunFlow(programPath, archPath, firstRunPath, programName);
            if(isSucc != "success"){
                string errorReportName = "VTR_crash_first_" + programName;
                string errorReport = "State 0: Error program detect: " + firstRunPath + "/" + programName + ".v\n";
                errorReport += "Program path: " + programPath + "\n";
                errorReport += "Concrete error: " + isSucc;
                Tool::logMessage(errorReport);

                Tool::fileWrite(errorReportPath + "/" + errorReportName + ".rpt", errorReport);
                crashNum++;
                Tool::logMessage("All cases num: " + to_string(i + 1));
                Tool::logMessage("Crash: " + to_string(crashNum));
                Tool::logMessage("Crash rate: " + to_string(((double)(crashNum)) / (i + 1)));
                Tool::logMessage("Performance detect: " + to_string(performanceNum));
                Tool::logMessage("Performance rate: " + to_string(((double)(performanceNum)) / (i + 1)));
                Tool::logMessage("\n");
                continue;
            }

            int secondSeed = Tool::getRandomfromClosed(1, 10000000);

            string firstTclPath = firstRunPath + "/" + programName + ".place";
            string firstNetlist = firstRunPath + "/" + programName + ".pre-vpr.blif";
            string secondNetlist = secondRunPath + "/" + programName + ".pre-vpr.blif";
            Tool::copyFile(firstNetlist, secondNetlist);
            string empytPath = secondRunPath + "/empty.place";
            Tool::fileWrite(empytPath, "");
            string combinedName = vtr.pinCommandFixed(firstTclPath, empytPath, secondRunPath);
            
            isSucc = vtr.runSeedBlif(secondNetlist, archPath, secondRunPath + "/" + combinedName, secondRunPath, programName, secondSeed);
            if(isSucc != "success"){
                string errorReportName = "VTR_crash_second_" + programName;
                string errorReport = "State 0: Error program detect: " + secondNetlist + "\n";
                errorReport += "Program path: " + programPath + "\n";
                errorReport += "Concrete error: " + isSucc;
                Tool::logMessage(errorReport);
                Tool::fileWrite(errorReportPath + "/" + errorReportName + ".rpt", errorReport);
                crashNum++;
                Tool::logMessage("All cases num: " + to_string(i + 1));
                Tool::logMessage("Crash: " + to_string(crashNum));
                Tool::logMessage("Crash rate: " + to_string(((double)(crashNum)) / (i + 1)));
                Tool::logMessage("Performance detect: " + to_string(performanceNum));
                Tool::logMessage("Performance rate: " + to_string(((double)(performanceNum)) / (i + 1)));
                Tool::logMessage("\n");
                continue;
            }
        }

        string testInfoPath = recentProductPath + "/testInfo.csv";
        if(Tool::isFileExists(testInfoPath)){
            i--; allPath.erase(allPath.begin() + randIndex);
            continue;
        }
        

        DelayandArea firstDA = vtr.getCritDelayandAreafromLog(firstRunPath + "/vpr_stdout.log");
        double firstDelay = firstDA.delay;
        double firstWire = firstDA.wireArea;
        TimingAnalyzer firstTA; firstTA.loadTimingRpt(firstRunPath + "/report_timing.rpt");
        double firstSlack = firstTA.worstSlack;

        DelayandArea secondDA = vtr.getCritDelayandAreafromLog(secondRunPath + "/vpr_stdout.log");
        double secondDelay = secondDA.delay;
        double secondWire = secondDA.wireArea;
        TimingAnalyzer secondTA; secondTA.loadTimingRpt(secondRunPath + "/report_timing.rpt");
        double secondSlack = secondTA.worstSlack;

        double delayRatio = abs(firstDelay - secondDelay) / firstDelay;
        double wireRatio = abs(firstWire - secondWire) / firstWire;
        double slackDelta = abs(firstSlack - secondSlack);

        bool isDefect = ((firstDelay >= secondDelay && firstWire >= secondWire && firstSlack <= secondSlack)
                        || (firstDelay <= secondDelay && firstWire <= secondWire && firstSlack >= secondSlack))
                        && (delayRatio >= 0.1 || wireRatio >= 0.1 || slackDelta >= 1);
        
        if(programName == "condition_program_first1052"){
            Tool::logMessage("syntax positive: " + to_string(firstDelay >= secondDelay && firstWire >= secondWire && firstSlack <= secondSlack));
            Tool::logMessage("syntax negative: " + to_string(firstDelay <= secondDelay && firstWire <= secondWire && firstSlack >= secondSlack));
            Tool::logMessage("over defect: " + to_string(delayRatio >= 0.1 || wireRatio >= 0.1 || slackDelta >= 1));
            Tool::logMessage("isDefect: " + to_string(isDefect));
        }
        
        string isDefectStr = "false";
        if(isDefect){
            performanceNum++;
            isDefectStr = "true";
        }

        string testInfo = programName + "," + to_string(firstDelay) + "," + to_string(firstWire) + "," + to_string(firstSlack) + ","
                + to_string(secondDelay) + "," + to_string(secondWire) + "," + to_string(secondSlack)+ ","
                + to_string(delayRatio) + "," + to_string(wireRatio) + "," + to_string(slackDelta) + "," + isDefectStr + "\n";

        Tool::fileWrite(testInfoPath, testInfo);

        //recent turn end
        allPath.erase(allPath.begin() + randIndex); //remove recent path to avoid selected again
        Tool::logMessage("All cases num: " + to_string(i + 1));
        Tool::logMessage("Crash: " + to_string(crashNum));
        Tool::logMessage("Crash rate: " + to_string(((double)(crashNum)) / (i + 1)));
        Tool::logMessage("Performance detect: " + to_string(performanceNum));
        Tool::logMessage("Performance rate: " + to_string(((double)(performanceNum)) / (i + 1)));
        Tool::logMessage("\n");
    }
    return true;
}

bool UserDrive::GetPerformData(string randomTestPath)
{
    string productPath = randomTestPath + "/product";
    vector<string> allProgramProduct;
    Tool::getAllFilefromDir(productPath, allProgramProduct);
    int seq = 0;
    string column = "program_name,first_delay,first_wire,first_slack,second_delay,second_wire,second_slack,delay_ratio,wire_ratio,slack_ratio,isDefect\n";
    for(string recentProgramPath : allProgramProduct){
        string rptPath = recentProgramPath + "/testInfo.csv";
        if(!Tool::isFileExists(rptPath)){
            Tool::logMessage(recentProgramPath + " has no rpt, skip.");
            continue;
        }
        seq++;
        string recentInfo = Tool::readFile(rptPath);
        Tool::logMessage("Program " + to_string(seq) + " extract successfully.");
        column += recentInfo;
    }
    Tool::fileWrite(randomTestPath + "/csv/random_test_performance_" + Tool::getLogFileTime() + ".csv", column);
    return false;
}

bool UserDrive::VTRCaseSizeGet(string randomTestPath, string csv)
{
    Tool::logMessage("Reading csv...");
    string content = Tool::readFile(csv);
    string result = "program_name,io_nums,blocks,total_nets,fanout_ave,fanout_max,isDefect\n";
    vector<string> lines = Tool::split(content, '\n');
    for(int i = 0; i < lines.size(); i++){
        if(i == 0)
            continue;
        string recentLine = lines[i];
        vector<string> items = Tool::split(recentLine, ',');
        string isDefect = items.back();
        if(isDefect.find("TRUE") != string::npos)
            isDefect = "TRUE";
        if(isDefect == "TRUE" || isDefect == "true"){
            string mainProgram = items[0];
            string productPath = randomTestPath + "/product/" + mainProgram + "/first";
            string features = VTRDrive::getFeaturesFromLog(productPath + "/vpr_stdout.log");
            string recentResult = mainProgram + "," + features + "," + isDefect + "\n";
            result += recentResult;
        }
        Tool::logMessage("seq: " + to_string(i) + " isDefect: " + isDefect + ".");
    }
    Tool::fileWrite(randomTestPath + "/csv/case_size.csv", result);
    return true;
}