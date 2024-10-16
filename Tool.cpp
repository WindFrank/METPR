#include "Tool.h"

string Tool::logFileTime = "";

string Tool::findGrammar(string type, string generateType, int expIndex)
{
    Json::Reader reader;
    Json::Value root;
    string grammarJsonName = string(getenv("VGENERATOR_WORK_DIR")) + "/Data/Grammar.json";
    if(generateType == "VTR")
        grammarJsonName = string(getenv("VGENERATOR_WORK_DIR")) + "/Data/Grammar_VTR.json";
    ifstream in(grammarJsonName, ios::binary);

    if(!in.is_open()){
        Tool::error("Error: Failed to open file " + grammarJsonName);
        exit(1);
    }

    if(reader.parse(in, root)){
        string index = to_string(expIndex);
        string recentGrammar = root[type][index].asString();
        if (recentGrammar != "")
            return recentGrammar;
    }
    return "";
}

vector<string> Tool::extractGrammar(string grammar)
{

    vector<string> resultStruct;
    string temp_grammar = grammar;
    int length = grammar.size();

    int stateOr = 0;
    int stateMultiple = 0;
    string temp = "";
    
    for(int i = 0; i <= length; i++){
        if (i == length){
            resultStruct.push_back(temp);
        }
        else if (temp_grammar[i] >= 'A' && temp_grammar[i] <= 'Z'){  //Detect Expression:
            string tempExpr = "";
            tempExpr += temp_grammar[i];
            for(i = i + 1; isWord(temp_grammar[i]); i++)
                tempExpr += temp_grammar[i];
            temp += tempExpr;
            i--;
        }
        else if (temp_grammar[i] == '+' || temp_grammar[i] == '*'){ //Connection
            resultStruct.push_back(temp);
            temp = "";
            if (temp_grammar[i] == '*')
                temp += '*';
        }
        else if (temp_grammar[i] == '|' || temp_grammar[i] == '-'){ //Inter calculation
            temp += temp_grammar[i];
        }
        else if(temp_grammar[i] == '\''){ // String or char
            string tempExpr = "";
            tempExpr += temp_grammar[i];
            for(i = i + 1; temp_grammar[i] != '\''; i++)
                tempExpr += temp_grammar[i];
            tempExpr += '\'';
            temp += tempExpr;
        }
        else{
            Tool::error("Error: extractGrammar find unknown syntax.");
        }
    }
    return resultStruct;

}

int Tool::isWord(char c){
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return 1;
    return 0;
}

void Tool::error(string info){
    ofstream testbench;
    testbench.open("Vgenerator.log", ios::app);
    if(testbench.is_open()){
        testbench << info;
        testbench.close();
    }
    else{
        testbench.close();
    }
    cout << info << endl;
    exit(1);
}

int Tool::getNumIndex(string grammarType, string generateType){
    int index = 1;
    while(true){
        if(findGrammar(grammarType, generateType, index) != ""){
            index++;
        }
        else
            break;
    }
    return index - 1;
}

void Tool::logMessage(string message){
    string targetLogTime = getLogFileTime();
    string text = message + "\n";
    std::ofstream testbench;
    testbench.open("Vgenerator_" + targetLogTime+ ".log", ios::app);
    if(testbench.is_open()){
        testbench << text;
        testbench.close();
    }
    else{
        testbench.close();
    }
    cout << message << endl;
}

vector<Statement*> Tool::getTypeStateFromV(vector<Statement*> initialV, string grammarType, string actionType){
    vector<Statement*> resultV;
    for (auto it = initialV.begin(); it < initialV.end(); it++){
        vector<Statement*> tempV = (*it)->getTargetSubStatement(grammarType, actionType);
        resultV.insert(resultV.end(), tempV.begin(), tempV.end());
    }
    return resultV;
}

int Tool::isContianInV(vector<Statement*> initialV, string value, string grammarType){
    for (auto it = initialV.begin(); it < initialV.end(); it++){
        if((*it)->isContain(grammarType, value))
            return 1;
    }
    return 0;
}

void Tool::setCodeinItem(Statement* item, string value){
    item->setRealCode(value);
    item->setIsterminal(1);
    item->setExpIndex(0);
    item->resetSubStatement();
}

int Tool::getRandomfromClosed(int start, int end){
    return (rand() % (end - start + 1)) + start;
}

string Tool::intToBinary(int num){
    string result = "";
    if(num == 0)
        return "0";
    while(num != 1){
        result = to_string(num % 2) + result;
        num = num / 2;
    }
    result = "1" + result;
    return result;
}

vector<Statement*> Tool::findSfromMultiS(Statement* multiS, string grammarType, string value){
    vector<Statement*> result;
    vector<Statement>* mDV = multiS->getSubStatement();
    for(int i = 0 ; i < mDV->size(); i++){
        vector<Statement*> recentResult = (*mDV)[i].getTargetSubStatement(grammarType, "ALL_LAYER");
        for(auto subIt = recentResult.begin(); subIt < recentResult.end(); subIt++){
            if((*subIt)->getRealCode() == value){
                result.push_back(&(*mDV)[i]);
            }    
        }
    }
    return result;
}

/*
    Fire write. If get 0, the file was written successfully.
    The filename should contain the path to the target location.
*/
bool Tool::fileWrite(string filename, string text, bool ifPrint){
    std::ofstream testbench;
    testbench.open(filename);
    if(testbench.is_open()){
        testbench << text;
        testbench.close();
        if(ifPrint)
            Tool::logMessage(filename + " write successfully");
        return 0;
    }
    else{
        testbench.close();
        Tool::logMessage(filename + " write failed");
        return 1;
    }
}

/*
    getXMLTree: Get the tree data of XML file
    filename: The path of target file.
*/
TiXmlElement* Tool::getXMLTree(string filename, string type){
    // file<> fdoc(filename.c_str());
    // xml_document<> doc;
    // char* data = fdoc.data();
    // doc.parse<0>(data);
    TiXmlDocument* doc = new TiXmlDocument(filename);
    TiXmlElement* result;
    if(!doc->LoadFile()){
        error("Error: getXMLTree cannot load " + type);
        exit(1);
    }
    if(type == "")
        result = doc->FirstChildElement();
    else
        result = doc->FirstChildElement(type.c_str());
    return result;
}

vector<string> Tool::split(string s, char c, string clean){
    vector<string> result;
    int nextIndex = 0;
    if(clean == "clean"){
        bool isPreSpace = false;
        string temp = "";
        for(int i = 0; i < s.size(); i++){
            if(s[i] == ' ' || s[i] == '\t'){
                if(isPreSpace)
                    continue;
                else{
                    isPreSpace = true;
                    temp += " ";
                }
            }
            else{
                isPreSpace = false;
                temp += s[i];
            }
        }
        s = temp;
    }
    while(true){
        nextIndex = s.find(c);
        if(nextIndex == string::npos)
            nextIndex = s.size();
        string recentELem = s.substr(0, nextIndex);
        result.push_back(recentELem);
        if(nextIndex == s.size())
            break;
        else if(nextIndex + 1 == s.size()){
            result.push_back("");
            break;
        }
        s = s.substr(nextIndex + 1, s.size() - nextIndex - 1);
    }
    return result;
}

int Tool::getNumFromClose(string s)
{
    int leftIndex = s.find("[");
    int rightIndex = s.find("]");
    if(leftIndex == string::npos || rightIndex == string::npos)
        error("Error: getNumFromClose cannot find []");
    int result = atoi(s.substr(leftIndex + 1, rightIndex - leftIndex - 1).c_str());
    return result;
}

TiXmlElement *Tool::getFirstElementByName(TiXmlElement *root, string name)
{
    queue<TiXmlElement*> operateNode;
    operateNode.push(root);
    while(operateNode.size() != 0){
        TiXmlElement* recentNode = operateNode.front();
        operateNode.pop();
        if(recentNode->Value() == name)
            return recentNode;
        else{
            for(TiXmlElement* it = recentNode->FirstChildElement();
                it != nullptr;
                it = it->NextSiblingElement()){
                    operateNode.push(it);
            }
        }
    }
    return nullptr;
}

vector<vector<string>> Tool::twoDSplit(string s, char smallC, char largeC)
{
    string cleanStr = "";
    for(int i = 0; i < s.size(); i++)
        if(s[i] != '\t')
            cleanStr += s[i];
    vector<string> temp = Tool::split(cleanStr, largeC);
    vector<vector<string>> matrix;
    for(auto rowString = temp.begin(); rowString < temp.end(); rowString++){
        matrix.push_back(Tool::split(*rowString, smallC));
    }
    return matrix;
}

pair<int, int> Tool::getBracketPair(string s)
{
    int commaIndex = s.find(",");
    int firstNum = atoi(s.substr(1, commaIndex - 1).c_str());
    int secondNum = atoi(s.substr(commaIndex + 1, s.size() - commaIndex - 2).c_str());
    return pair<int, int>(firstNum, secondNum);
}


void Tool::verilogCom(string filepath1, string filepath2, string targetDirectory, string newFileName)
{
    int lastSlash1 = filepath1.rfind('/');
    string filename1 = filepath1;
    if(lastSlash1 != string::npos)
        filename1 = filepath1.substr(lastSlash1 + 1, filepath1.size() - lastSlash1 - 1);
    int lastSlash2 = filepath2.rfind('/');
    string filename2 = filepath2;
    if(lastSlash2 != string::npos)
        filename2 = filepath2.substr(lastSlash2 + 1, filepath2.size() - lastSlash2 - 1);

    //read files
    ifstream file1(filepath1);
    if(!file1.is_open()){
        Tool::error("Error opening file: " + filepath1);
        return;
    }
    stringstream ss1;
    ss1 << file1.rdbuf();
    string content1 = ss1.str();
    file1.close();

    ifstream file2(filepath2);
    if(!file2.is_open()){
        Tool::error("Error opening file: " + filepath2);
        return;
    }
    stringstream ss2;
    ss2 << file2.rdbuf();
    string content2 = ss2.str();
    file2.close();
   
    //deal with same module name
    vector<string> moduleNames1 = getAllModuleNames(content1);
    vector<string> moduleNames2 = getAllModuleNames(content2);
    stringstream module1 = getModule(filepath1, moduleNames1[0]);
    stringstream module2 = getModule(filepath2, moduleNames2[0]);
    vector<string> commonModuleNames;
    unordered_set<string> set(moduleNames1.begin(), moduleNames1.end());
    for(const auto& str : moduleNames2){
        if(set.count(str) > 0){
            commonModuleNames.push_back(str);
        }
    }
    for(const auto& name : commonModuleNames){
        if(name.compare(moduleNames1[0]) == 0 || name.compare(moduleNames2[0]) == 0){
            string result1;
            string result2;
            stringstream m1 = getModule(filepath1, name);
            stringstream m2 = getModule(filepath2, name);
            for(char c : m1.str()){
                if(c != '\t' && c != '\n'){
                    result1 += c;
                }
            }
            for(char c : m2.str()){
                if(c != '\t' && c != '\n'){
                    result2 += c;
                }
            }
            if(result1.compare(result2) != 0){
                error("Error: Same main module with inconsist function.");
            }
        }
        string moduleStart = "module " + name + "(";
        string moduleEnd = "endmodule";
        size_t startPos = content2.find(moduleStart);
        size_t endPos = content2.find(moduleEnd, startPos + moduleStart.length());
        content2 = content2.substr(0, startPos) + content2.substr(endPos + moduleEnd.length());
    }
   
    //deal with same variable name
    string combinedContent = content1 + "\n" + content2;
    // string combinedFilename = "_" + filename1.substr(0, filename1.find_last_of('.')) + "_and_" + filename2.substr(0, filename2.find_last_of('.')) + "_";
    string def1;
    string def2;
    getline(module1, def1);
    getline(module2, def2);
    vector<string> variables1 = getVariables(def1);
    vector<string> variables2 = getVariables(def2);
    vector<string> originalV2 = variables2;
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
        while(find(variables1.begin(), variables1.end(), v) != variables1.end() ||
              find(variables2.begin(), variables2.end(), v) != variables2.end() ||
              hasDuplicate(modifiedVariables) || isSystemKeyword(v)){
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
    string line;
    stringstream newModule2;
    module2.seekg(0, ios::beg);
    while(getline(module2, line)){
        for(size_t i = 0; i < commonVariables.size(); i++){
            size_t pos = line.find(commonVariables[i]);
            while(pos != string::npos){
                char prevChar = (pos > 0) ? line[pos - 1] : ' ';
                char nextChar = (pos + commonVariables[i].length() < line.length()) ? line[pos + commonVariables[i].length()] : ' ';
                if((isspace(prevChar) || prevChar == '(') && (nextChar == ',' || nextChar == ')' || nextChar == ';')){
                    line.replace(pos, commonVariables[i].length(), modifiedVariables[i]);
                }
                pos = line.find(commonVariables[i], pos + modifiedVariables[i].length());
            }
        }
        newModule2 << line << '\n';
    }
    for(const auto& commonVar : commonVariables){
        auto it = std::remove(variables2.begin(), variables2.end(), commonVar);
        variables2.erase(it, variables2.end());
    }
    vector<string> newVariables;
    newVariables.insert(newVariables.end(), variables1.begin(), variables1.end());
    newVariables.insert(newVariables.end(), modifiedVariables.begin(), modifiedVariables.end());
    newVariables.insert(newVariables.end(), variables2.begin(), variables2.end());

    //establish the new main module
    string newModule = "module " + newFileName + "(";
    for(const auto& var : newVariables){
        newModule += var + ", ";
    }
    newModule = newModule.substr(0, newModule.length()-2) + ");" + "\n";
    module1.seekg(0, ios::beg);
    getline(module1, line);
    vector<string> v1 = variables1;
    string previousLine = "";
    while(getline(module1, line) && (variables1.size() != 0 || line.find("reg ") != string::npos)){
        if((line.find("wire ") == string::npos && line.find("reg ") == string::npos) || line.find("input ") != string::npos || line.find("output ") != string::npos) 
            newModule += line + "\n";
        for(auto it = variables1.begin(); it != variables1.end();){
            if (line.find(" " + *it + ";") != string::npos || line.find(" " + *it + ",") != string::npos) {
                it = variables1.erase(it);
            } else {
                ++it;
            }
        }
        previousLine = line;
    }
    getline(newModule2, line);
    vector<string> newVariables2 = getVariables(line);
    vector<string> v2 = newVariables2;
    while(getline(newModule2, line) && (newVariables2.size() != 0 || line.find("reg ") != string::npos)){
        if((line.find("wire ") == string::npos && line.find("reg ") == string::npos) || line.find("input ") != string::npos || line.find("output ") != string::npos)
            newModule += line + "\n";
        for(auto it = newVariables2.begin(); it != newVariables2.end();){
            if (line.find(" " + *it + ";") != string::npos || line.find(" " + *it + ",") != string::npos) {
                it = newVariables2.erase(it);
            } else {
                ++it;
            }
        }
        previousLine = line;
    }

    newModule += "\t" + filename1.substr(0, filename1.find_last_of('.')) + " ";
    string character = filename1.substr(0, 1);
    character[0] = toupper(character[0]);
    int postNum = 1;
    string callName1 = character + to_string(postNum);
    while(find(newVariables.begin(), newVariables.end(), callName1) != newVariables.end()){
        postNum++;
        callName1 = character + to_string(postNum);
    }
    newModule += callName1 + "(";
    for(const auto& var : v1){
        newModule += "." + var + "(" + var + ")" + ", ";
    }
    newModule = newModule.substr(0, newModule.length()-2) + ");" + "\n";
    newModule += "\t" +filename2.substr(0, filename2.find_last_of('.')) + " ";
    string character2 = filename2.substr(0, 1);
    character2[0] = toupper(character2[0]);
    postNum = 1;
    string callName2 = character2 + to_string(postNum);
    while(find(newVariables.begin(), newVariables.end(), callName2) != newVariables.end() || callName2 == callName1){
        postNum++;
        callName2 = character + to_string(postNum);
    }
    newModule += callName2 + "(";

    for(size_t i = 0; i < v2.size(); i++){
        newModule += "." + originalV2[i] + "(" + v2[i] + ")" + ", ";
    }
    newModule = newModule.substr(0, newModule.length()-2) + ");" + "\n";
    newModule += "endmodule";

    //write into file
    if(targetDirectory != "")
        targetDirectory += "/";
    ofstream outputFile(targetDirectory + newFileName + ".v");
    if(!outputFile.is_open()){
        Tool::error("Error creating output file: " + targetDirectory + newFileName);
        return;
    }
    else{
        logMessage("Create file " + targetDirectory + newFileName + ".v" + " successfully.\n");
    }
    outputFile << newModule;
    outputFile << "\n";
    outputFile << "\n";
    outputFile << combinedContent;
    outputFile.close();
    // //write graph file
    // string graphPath = targetDirectory + "graph/";
    // string file1GraphPath = graphPath + filename1.substr(0, filename1.find_last_of('.')) + ".graph";
    // string file2GraphPath = graphPath + filename2.substr(0, filename2.find_last_of('.')) + ".graph";
    // if(!isFileExists(file1GraphPath) || !isFileExists(file2GraphPath)){
    //     Tool::error("Error: VerilogCom cannot found the graph of submodule.");
    // }
    // string formula1 = getFormulafromGraph(file1GraphPath);
    // string formula2 = getFormulafromGraph(file2GraphPath);
    // string newFormula = "( " + formula1 + " and " + formula2 + " )";
    // string graphContent = generateGraph(newFileName + ".v", filename1, filename2, newFormula);
    // fileWrite(graphPath + newFileName + ".graph", graphContent);

    return;
}


stringstream Tool::getModule(string filepath, string moduleName)
{
    string content = readFile(filepath);
    return getStrModule(content, moduleName);
}

stringstream Tool::getStrModule(string content, string moduleName){
    istringstream iss(content);
    string line;
    bool insideModule = false;
    stringstream module;
    while(getline(iss, line)) {
        if (line.find("module " + moduleName + "(") != string::npos || line.find("module " + moduleName + " (") != string::npos) insideModule = true;   
        if (insideModule) {
            module << line << "\n";
            if (line.find("endmodule") != string::npos) {
                insideModule = false;
                break;
            }
        }
    }
    return module;
}

string Tool::getLogFileTime()
{
    if(logFileTime == "" || logFileTime.empty()){
        logFileTime = getRecentTime();
    }
    return logFileTime;
}

vector<string> Tool::getAllModuleNames(string verilog)
{
    vector<string> moduleNames;
    regex moduleRegex(R"(module\s+(\w+)\s*\()");
    auto words_begin = sregex_iterator(verilog.begin(), verilog.end(), moduleRegex);
    auto words_end = sregex_iterator();
    for (sregex_iterator i = words_begin; i != words_end; ++i) {
        smatch match = *i;
        string moduleName = match.str(1);
        moduleNames.push_back(moduleName);
    }
    return moduleNames;
}

vector<string> Tool::getVariables(string def)
{
    vector<string> variables;
    size_t startPos = def.find('(');
    size_t endPos = def.find(')');
    if(startPos != string::npos && endPos != string::npos && startPos < endPos){
        string variableString = def.substr(startPos + 1, endPos - startPos - 1);
        istringstream iss(variableString);
        string variable;
        while (std::getline(iss, variable, ',')) {
            variable.erase(std::remove_if(variable.begin(), variable.end(), ::isspace), variable.end());
            variables.push_back(variable);
        }
    }
    return variables;
}

bool Tool::hasDuplicate(const vector<string>& vec)
{
    unordered_set<string> uniqueStrings;
    for(const auto& str : vec){
        if (!uniqueStrings.insert(str).second) {
            return true;
        }
    }
    return false;
}

bool Tool::isDirectoryExists(string path){
    struct stat fileStat;
    return (stat(path.c_str(), &fileStat) == 0) && S_ISDIR(fileStat.st_mode);
}

bool Tool::isFileExists(string path){
    ifstream fin(path);
    bool isCannotOpen = !fin;
    fin.close();
    return !isCannotOpen;
}

bool Tool::createDirectory(string path){
    if(mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
        return true;
    else
        return false;
}

bool Tool::isDirectoryEmpty(const char* path){
    DIR* dir = opendir(path);
    if (dir == nullptr)
    {
        return false; // error opening directory
    }
    bool isEmpty = true;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_type != DT_DIR || (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0))
        {
            isEmpty = false;
            break;
        }
    }
    closedir(dir);
    return isEmpty;
}

void Tool::verilogSeiresCom(string filepath1, string filepath2, string targetDirectory, string newFileName, string strategy){
    int lastSlash1 = filepath1.rfind('/');
    string filename1 = filepath1;
    if(lastSlash1 != string::npos)
        filename1 = filepath1.substr(lastSlash1 + 1, filepath1.size() - lastSlash1 - 1);
    int lastSlash2 = filepath2.rfind('/');
    string filename2 = filepath2;
    if(lastSlash2 != string::npos)
        filename2 = filepath2.substr(lastSlash2 + 1, filepath2.size() - lastSlash2 - 1);

    //read files
    ifstream file1(filepath1);
    if(!file1.is_open()){
        Tool::error("Error opening file: " + filepath1);
        return;
    }
    stringstream ss1;
    ss1 << file1.rdbuf();
    string content1 = ss1.str();
    file1.close();

    ifstream file2(filepath2);
    if(!file2.is_open()){
        Tool::error("Error opening file: " + filepath2);
        return;
    }
    stringstream ss2;
    ss2 << file2.rdbuf();
    string content2 = ss2.str();
    file2.close();

    //deal with the duplicate module name
    vector<string> moduleNames1 = getAllModuleNames(content1);
    vector<string> moduleNames2 = getAllModuleNames(content2);
    stringstream module1 = getModule(filepath1, moduleNames1[0]);
    stringstream module2 = getModule(filepath2, moduleNames2[0]);
    vector<string> commonModuleNames;
    unordered_set<string> set(moduleNames1.begin(), moduleNames1.end());
    for(const auto& str : moduleNames2){
        if(set.count(str) > 0){
            commonModuleNames.push_back(str);
        }
    }
    for(const auto& name : commonModuleNames){
        if(name.compare(moduleNames1[0]) == 0 || name.compare(moduleNames2[0]) == 0){
            string result1;
            string result2;
            stringstream m1 = getModule(filepath1, name);
            stringstream m2 = getModule(filepath2, name);
            for(char c : m1.str()){
                if(c != '\t' && c != '\n'){
                    result1 += c;
                }
            }
            for(char c : m2.str()){
                if(c != '\t' && c != '\n'){
                    result2 += c;
                }
            }
            if(result1.compare(result2) != 0){
                error("Error: Same main module with inconsist function.");
            }
        }
        string moduleStart = "module " + name + "(";
        string moduleEnd = "endmodule";
        size_t startPos = content2.find(moduleStart);
        size_t endPos = content2.find(moduleEnd, startPos + moduleStart.length());
        content2 = content2.substr(0, startPos) + content2.substr(endPos + moduleEnd.length());
    }

    //Extract output of first module and input of second module.
    string combinedContent = content1 + "\n" + content2;
    // string combinedFilename = "_" + filename1.substr(0, filename1.find_last_of('.')) + "_next_" + filename2.substr(0, filename2.find_last_of('.')) + "_";
    //Find output in first module.
    string def;
    vector<Attribute*> first_outputs, first_inputs, second_inputs, second_outputs;
    bool firstModuleTimes = false;
    extractInoutfromTextV(module1, first_outputs, first_inputs);
    extractInoutfromTextV(module2, second_outputs, second_inputs);
    //Turn second_outputs's type from reg to wire
    for(auto it = second_outputs.begin(); it < second_outputs.end(); it++){
        Attribute* attr = *it;
        attr->setDataType("wire");
    }
    vector<Attribute> init_first_outputs, init_first_inputs, init_second_inputs, init_second_outputs;
    init_first_inputs = deepCopyAttr(first_inputs);
    init_first_outputs = deepCopyAttr(first_outputs);
    init_second_inputs = deepCopyAttr(second_inputs);
    init_second_outputs = deepCopyAttr(second_outputs);

    //Generate new module
    //whole inout generation.
    string newModule = "module " + newFileName + "(";
    vector<Attribute*> tempAV;
    tempAV.insert(tempAV.end(), first_inputs.begin(), first_inputs.end());
    tempAV.insert(tempAV.end(), first_outputs.begin(), first_outputs.end());
    tempAV.insert(tempAV.end(), second_inputs.begin(), second_inputs.end());
    tempAV.insert(tempAV.end(), second_outputs.begin(), second_outputs.end());

    //eliminate repeated var
    string commonVar = "";
    for(auto it = tempAV.begin(); it < tempAV.end(); it++){
        Attribute* attr = *it;
        string varName = attr->getName();
        if(varName == "if01")
            int a = 0;
        int suffix = 1;
        vector<string> variables = Tool::split(commonVar, ';');
        while(find(variables.begin(), variables.end(), varName) != variables.end() || isSystemKeyword(varName)){
            if(isdigit(varName[varName.length()-1])){
                size_t pos = varName.find_last_not_of("0123456789");
                if(pos != string::npos) {
                    size_t numericSuffixStart = pos + 1;
                    size_t numericSuffixLength = varName.size() - numericSuffixStart;
                    string numericSuffix = varName.substr(numericSuffixStart, numericSuffixLength);
                    suffix = stoi(numericSuffix) + 1;
                    varName = varName.substr(0, pos+1);
                    string completeSuffix = to_string(suffix);
                    if(numericSuffix[0] == '0' && numericSuffixLength != 1){
                        size_t count = numericSuffix.find_first_not_of('0');
                        count = (count == std::string::npos) ? numericSuffix.length() : count;
                        string leadingZeros(count, '0');
                        completeSuffix = leadingZeros + completeSuffix;
                    }
                    varName += completeSuffix;
                }
            }
            else{
                varName += to_string(suffix);
            }
        }
        attr->setName(varName);
        commonVar += varName + ";";
    }

    //first input and second output interface write
    vector<Attribute*> interfaceVar;
    interfaceVar.insert(interfaceVar.end(), first_inputs.begin(), first_inputs.end());
    interfaceVar.insert(interfaceVar.end(), second_outputs.begin(), second_outputs.end());
    bool firstInterfacePin = true;
    string interfaceVarStr = "";
    for(auto it = interfaceVar.begin(); it < interfaceVar.end(); it++){
        Attribute* attr = *it;
        if(firstInterfacePin){
            firstInterfacePin = false;
            interfaceVarStr += attr->getName();
        }
        else{
            interfaceVarStr += ", " + attr->getName();
        }
    }
    newModule += interfaceVarStr;
    newModule += ");\n";
    newModule += genDecalaration(second_outputs, "inout");
    newModule += genDecalaration(first_inputs, "inout");
    newModule += genDecalaration(first_outputs, "none");
    // newModule += genDecalaration(second_inputs, "none");
    
    //Call first module
    int signNum = 1;
    string firstModuleName = filename1.substr(0, filename1.find_last_of('.'));
    string callFirstModule = "\t" + firstModuleName;
    
    string callInstance =  " M" + to_string(signNum++);
    while(commonVar.find(callInstance.substr(1, callInstance.size() - 1)) != string::npos)
        callInstance = " M" + to_string(signNum++);
    string callParams = "(";
    bool firstAttr = true;
    for(int i = 0; i < first_inputs.size(); i++){
        string recentParam = ", ";
        string acceptPin = init_first_inputs[i].getName();
        string passParam = first_inputs[i]->getName();
        if(firstAttr){
            recentParam = "";
            firstAttr = false;
        }
        recentParam += "." + acceptPin + "(" + passParam + ")";
        callParams += recentParam;
    }
    for(int i = 0; i < first_outputs.size(); i++){
        string recentParam = ", ";
        string acceptPin = init_first_outputs[i].getName();
        string passParam = first_outputs[i]->getName();
        recentParam += "." + acceptPin + "(" + passParam + ")";
        callParams += recentParam;
    }
    callParams += ");\n";
    //Note: The result of first call will be added after temp var defination.

    //Random connection between first_outputs and second_inputs.
    vector<pair<int, int>> first_output_left;
    vector<vector<vector<pair<int, int>>*>> second_input_choose;
    vector<pair<int, int>> second_input_all;
    //initial
    string occupyFlag = "";
    vector<string> secondOccupyFlagV;
    int lenFirstOut = 0;
    for(int i = 0; i < init_first_outputs.size(); i++){
        Attribute recentAttr = init_first_outputs[i];
        int start = recentAttr.getStart();
        int end = recentAttr.getEnd();
        for(int j = start; j <= end; j++){
            //Pair first: passAttr; Pair second: concrect bit on passAttr
            first_output_left.push_back(pair<int, int>(i, j));
            lenFirstOut++;
            occupyFlag += "0";
        }
    }
    //Execute different Strategy
    if(strategy == "simple"){   //simple connect
        //random choose, second inputs full occupy.
        for(int i = 0; i < init_second_inputs.size(); i++){
            Attribute recentAttr = init_second_inputs[i];
            int start = recentAttr.getStart();
            int end = recentAttr.getEnd();
            vector<vector<pair<int, int>>*> recentInputAttr;
            for(int j = start; j <= end; j++){
                vector<pair<int, int>>* recentInputBit = new vector<pair<int, int>>();
                int randIndex = getRandomfromClosed(0, lenFirstOut - 1);
                occupyFlag[randIndex] = '1';
                pair<int, int> recentPass = first_output_left[randIndex];
                recentInputBit->push_back(recentPass);
                second_input_all.push_back(pair<int, int>(i, j));
                recentInputAttr.push_back(recentInputBit);
            }
            second_input_choose.push_back(recentInputAttr);
        }
        //check if some pin is hang. If so, choose hang pin in first output and connect it to the random one in second input.
        if(occupyFlag.find("0") != string::npos){
            for(int i = 0; i < occupyFlag.size(); i++){
                char recentC = occupyFlag[i];
                if(recentC == '0'){
                    int randSecondInIndex = getRandomfromClosed(0, second_input_all.size() - 1);
                    pair<int, int> recentPass = first_output_left[i];
                    pair<int, int> recentInputPair = second_input_all[randSecondInIndex];
                    int recentAcceptVar = recentInputPair.first;
                    int recentAcceptVarBit = recentInputPair.second;
                    vector<pair<int, int>>* recentInputBit = second_input_choose[recentAcceptVar][recentAcceptVarBit];
                    recentInputBit->push_back(recentPass);
                    occupyFlag[i] = '1';
                }
            }
        }
    }
    else if(strategy == "full"){    //full connect
        //random choose, second inputs full occupy.
        for(int i = 0; i < init_second_inputs.size(); i++){
            Attribute recentAttr = init_second_inputs[i];
            int start = recentAttr.getStart();
            int end = recentAttr.getEnd();
            vector<vector<pair<int, int>>*> recentInputAttr;
            string recentAttrOccupy = "";
            for(int j = start; j <= end; j++){
                vector<pair<int, int>>* recentInputBit = new vector<pair<int, int>>();
                // int randIndex = getRandomfromClosed(0, lenFirstOut - 1);
                // occupyFlag[randIndex] = '1';
                // pair<int, int> recentPass = first_output_left[randIndex];
                // recentInputBit->push_back(recentPass);
                second_input_all.push_back(pair<int, int>(i, j));
                recentInputAttr.push_back(recentInputBit);
                recentAttrOccupy += "0";
            }
            secondOccupyFlagV.push_back(recentAttrOccupy);
            second_input_choose.push_back(recentInputAttr);
        }
        //check if some pin is hang. If so, random choose first and random choose second
        //keep checking until all the pins are not hang.
        while(occupyFlag.find("0") != string::npos || !oneCheckonV(secondOccupyFlagV)){
            int randFirstOutIndex = getRandomfromClosed(0, lenFirstOut - 1);
            int randSecondInIndex = getRandomfromClosed(0, second_input_all.size() - 1);
            occupyFlag[randFirstOutIndex] = '1';
            pair<int, int> recentPass = first_output_left[randFirstOutIndex];
            pair<int, int> recentInputPair = second_input_all[randSecondInIndex];
            secondOccupyFlagV[recentInputPair.first][recentInputPair.second] = '1';
            int recentAcceptVar = recentInputPair.first;
            int recentAcceptVarBit = recentInputPair.second;
            vector<pair<int, int>>* recentInputBit = second_input_choose[recentAcceptVar][recentAcceptVarBit];
            recentInputBit->push_back(recentPass);
        }
    }
    else{   //invalid command: error
        error("Error: Tool verilogSeriesCom cannot recognize command " + strategy + ".");
        return;
    }
    
    //Connect
    pair<int, int> lastPair;
    string callSecond = "\t" + filename2.substr(0, filename2.find_last_of('.'));
    string callSecondInstance = " M" + to_string(signNum++);
    while(commonVar.find(callSecondInstance.substr(1, callSecondInstance.size() - 1)) != string::npos)
        callSecondInstance = " M" + to_string(signNum++);
    string callSecondParams = "(";
    firstAttr = true;
    vector<string> tempVarDeclare;
    for(int i = 0; i < second_input_choose.size(); i++){
        string paramItemStr = firstAttr ? "." : ", .";
        firstAttr = false;
        string calculateStr = "";
        string tempVarDeclaration = "";
        vector<vector<pair<int, int>>*> recentAcceptAttrPass = second_input_choose[i];
        
        //temp declaration
        Attribute recentAcceptAttr = init_second_inputs[i];
        string assignorReg = "\t" + recentAcceptAttr.getDataType() + " ";
        int endRecentAccept = recentAcceptAttr.getEnd();
        int startRecentAccept = recentAcceptAttr.getStart();
        string initalScope = endRecentAccept == startRecentAccept ? 
                            "" : "[" + to_string(endRecentAccept) + ":" + to_string(startRecentAccept) + "] ";
        string tempVar = "var" + to_string(signNum++);
        tempVarDeclaration = assignorReg + initalScope + tempVar + ";\n";

        //Assign resual of operation
        string recentDataType = recentAcceptAttr.getDataType();
        string assignDeclare = recentDataType == "reg" ? "\t" : "\tassign ";
        for(int bitIndex = 0; bitIndex < recentAcceptAttrPass.size(); bitIndex++){
            vector<pair<int, int>> recenInputBit = *recentAcceptAttrPass[bitIndex];
            string targetPinUse = "";
            bool innerFirstAttr = true;
            string operators = "+-*&|";
            string assignBit = initalScope == "" ? "" : "[" + to_string(bitIndex) + "]";
            for(auto it = recenInputBit.begin(); it < recenInputBit.end(); it++){
                pair<int, int> recentKV = *it;
                Attribute* useFirstOutAttr = first_outputs[recentKV.first];
                int bit = recentKV.second;
                int operateRandIndex = getRandomfromClosed(0, operators.size() - 1);
                char chooseOperator = operators[operateRandIndex];
                string tempOperator = "";
                tempOperator.push_back(chooseOperator);
                string plus = innerFirstAttr ? "" : " " + tempOperator + " ";
                string useBit = useFirstOutAttr->getStart() == useFirstOutAttr->getEnd() ? "" : "[" + to_string(bit) +"]";
                string recentUseItem = useFirstOutAttr->getName() + useBit;
                targetPinUse += plus + recentUseItem;
                innerFirstAttr = false;
            }
            calculateStr += assignDeclare + tempVar + assignBit + " = " + targetPinUse + ";\n";
        }
        tempVarDeclare.push_back(calculateStr);
        

        //Call second module
        string acceptPin = recentAcceptAttr.getName() + "(";
        string passAttr = tempVar + ")";
        paramItemStr += acceptPin + passAttr;
        callSecondParams += paramItemStr;

        newModule += tempVarDeclaration;
    }
    for(int i = 0; i < second_outputs.size(); i++){
        string recentParam = ", ";
        string acceptPin = init_second_outputs[i].getName();
        string passParam = second_outputs[i]->getName();
        recentParam += "." + acceptPin + "(" + passParam + ")";
        callSecondParams += recentParam;
    }
    callSecondParams += ");\n";

    newModule += callFirstModule + callInstance + callParams;
    for(auto it = tempVarDeclare.begin(); it < tempVarDeclare.end(); it++){
        newModule += *it;
    }
    newModule += callSecond + callSecondInstance + callSecondParams;
    newModule += "endmodule\n\n";
    newModule += combinedContent;

    //write into file
    if(targetDirectory != "")
        targetDirectory += "/";
    //write .v file
    ofstream outputFile(targetDirectory + newFileName + ".v");
    if(!outputFile.is_open()){
        Tool::error("Error creating output file: " + newFileName + ".v");
        return;
    }
    else{
        logMessage("Create file " + targetDirectory + newFileName + ".v" + " successfully.\n");
    }
    outputFile << newModule;
    outputFile.close();

    return;
}

bool Tool::extractInoutfromTextV(stringstream& module, vector<Attribute*>& outputs, vector<Attribute*>& inputs){
    string def;
    bool firstModuleTimes = false;
    string inputs_of_first_module;
    vector<Attribute*> allAttr;
    allAttr.insert(allAttr.end(), outputs.begin(), outputs.end());
    allAttr.insert(allAttr.end(), inputs.begin(), inputs.end());
    while(getline(module, def)){
        string temp = "";
        bool isLastSpace = false;
        for(int i = 0; i < def.size(); i++){
            if(def[i] != '\t' && def[i] != '\n' && def[i] != ';' && def[i] != ','){
                if(def[i] == ' ' && !isLastSpace){
                    isLastSpace = true;
                }
                else if(def[i] == ' ' && isLastSpace){
                    continue;
                }
                else{
                    isLastSpace = false;
                }
                temp += def[i];
            }
        }
        if(temp[0] == ' ' && temp.size() > 1)
            temp = temp.substr(1, temp.size() - 1);
        else if(temp.size() <= 1)
            continue;
        while(temp.size() > 0 && temp.back() == ' ')
            temp = temp.substr(0, temp.size() - 1);
        vector<string> elems = Tool::split(temp, ' ');
        if(elems[0] == "output" || elems[0] == "input"){
            int arr = elems[1] == "wire" || elems[1] == "reg" ? 1 : 0;
            string attrName = "";
            string dataType = arr == 1 ? elems[1] : "wire";
            int colon = elems[1 + arr].find(":");
            int maxEnd = 0;
            int minEnd = 0;
            int attrNameIndex = -1;
            bool isBus = false;
            if(colon != string::npos){
                isBus = true;
                attrNameIndex = 2 + arr;
                string colonItem = elems[1 + arr];
                maxEnd = atoi(colonItem.substr(colonItem.find("[") + 1, colon - colonItem.find("[") - 1).c_str());
                minEnd = atoi(colonItem.substr(colon + 1, colonItem.size() - colon - 1).c_str());
            }
            else{
                attrNameIndex = 1 + arr;
                attrName = elems[1 + arr];
            }
            Attribute* recenInout;
            while(attrNameIndex < elems.size()){
                attrName = elems[attrNameIndex];
                recenInout = new Attribute(attrName, elems[0], maxEnd, minEnd);
                recenInout->setDataType(dataType);
                recenInout->isBus = isBus;
                if(elems[0] == "input")
                    inputs.push_back(recenInout);
                else
                    outputs.push_back(recenInout);
                attrNameIndex++;
            }
        }
        else if(elems[0] == "module"){
            if(!firstModuleTimes){
                firstModuleTimes = true;
            }
            else
                break;
        }
        else if(elems[0] == "wire" || elems[0] == "reg"){
            int colon = elems[1].find(":");
            for(auto it = allAttr.begin(); it < allAttr.end(); it++){
                Attribute* attr = *it;
                string attrName = attr->getName();
                string outputName;
                if(colon != string::npos)
                    outputName = elems[2];
                else
                    outputName = elems[1];
                if(attrName == outputName){
                    attr->setDataType(elems[0]);
                }
            }
        }
    }
    return true;
}

string Tool::genDecalaration(vector<Attribute*> av, string type){
    string result = "";
    for(auto it = av.begin(); it < av.end(); it++){
        Attribute* attr = *it;
        string attrName = attr->getName();
        int end = attr->getEnd();
        int start = attr->getStart();
        string inout = "\t";
        if(type == "inout"){
            string inoutType = attr->getType();
            if(inoutType == "input" || inoutType == "output")
                inout += inoutType + " ";
        }
        string dataType = attr->getDataType() + " ";
        string initialScope = "";
        if(end != start)
            initialScope = "[" + to_string(end) + ":" + to_string(start) + "] ";
        string newDeclaration = inout + dataType + initialScope + attrName + ";\n"; //input turn to out
        result += newDeclaration;
    }
    return result;
}

vector<Attribute> Tool::deepCopyAttr(vector<Attribute*> avstr){
    vector<Attribute> av;
    for(auto it = avstr.begin(); it < avstr.end(); it++){
        Attribute* attr = *it;
        Attribute newAttr(*attr);
        av.push_back(newAttr);
    }
    return av;
}

bool Tool::oneCheckonV(vector<string> sv){
    for(int i = 0; i < sv.size(); i++)
        if(sv[i].find('0') != string::npos)
            return false;
    return true;
}

bool Tool::copyFile(string sourcePath, string destinationPath) {
    std::ifstream sourceFile(sourcePath, std::ios::binary);
    if (!sourceFile) {
        std::cout << "Error opening source file: " << sourcePath << std::endl;
        return false;
    }
 
    std::ofstream destinationFile(destinationPath, std::ios::binary);
    if (!destinationFile) {
        std::cout << "Error opening destination file: " << destinationPath << std::endl;
        return false;
    }
 
    destinationFile << sourceFile.rdbuf();
 
    sourceFile.close();
    destinationFile.close();
    return true;
}

void Tool::getAllFilefromDir(string imgDirPath, vector<string> &vimgPath)
{

	DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(imgDirPath.c_str())))
    {
        cout<<"Folder doesn't Exist!"<<endl;
        return;
    }

    while((ptr = readdir(pDir))!=0) 
    {
        //exclude the . and .. directory
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
        {
            vimgPath.push_back(imgDirPath + "/" + ptr->d_name);
    	}
    }
    sort(vimgPath.begin(),vimgPath.end());

    closedir(pDir);
}

bool Tool::same_vec(const vector<int>& a,const vector<int>& b){
    set<int> m( a.begin(),a.end() );
    auto it = b.begin();
    for(; it!=b.end(); it++)
        if( m.find( *it )!=m.end() )
            return true;
    return false;
}

bool Tool::same_vec(const vector<string>& a, const vector<string>& b){
    set<string> m( a.begin(),a.end() );
    auto it = b.begin();
    for(; it!=b.end(); it++)
        if( m.find( *it )!=m.end() )
            return true;
    return false;
}

bool Tool::same_vec(vector<pair<int, int>> a, vector<pair<int, int>> b){
    set<pair<int, int>> m( a.begin(),a.end() );
    auto it = b.begin();
    for(; it!=b.end(); it++)
        if( m.find( *it )!=m.end() )
            return true;
    return false;
}

string Tool::findFilefromPath(string path){
    int lastSlash = path.rfind('/');
    string filename = path;
    if(lastSlash != string::npos)
        filename = path.substr(lastSlash + 1, path.size() - lastSlash - 1);
    return filename;
}

string Tool::getRecentTime(){
    time_t nowtime;
    time(&nowtime);
    tm* p = localtime(&nowtime);
    string year = to_string(p->tm_year + 1900);
    string month = to_string(p->tm_mon + 1);
    string day = to_string(p->tm_mday);
    string hour = to_string(p->tm_hour);
    string min = to_string(p->tm_min);
    string sec = to_string(p->tm_sec);
    string result = year + "_" + month + "_" + day + "_" + hour + "_" + min + "_" + sec;
    return result;
}

string Tool::readFile(string path){
    //read files
    ifstream file1(path);
    if(!file1.is_open()){
        Tool::error("Error opening file: " + path);
        return "";
    }
    stringstream ss1;
    ss1 << file1.rdbuf();
    string content1 = ss1.str();
    file1.close();
    return content1;
}

string Tool::washString(string path){
    string result = "";
    bool lastNonsense = false;
    for(int i = 0; i < path.size(); i++){
        char c = path[i];
        if((c == ' ' || c == '\t' || c == '\n') && !lastNonsense){
            lastNonsense = true;
            result += ' ';
        }
        else if(c != '\t' && c != ' ' && c != '\n'){
            lastNonsense = false;
            result += c;
        }
    }
    return result;
}

bool Tool::isNum(string str){
    if(str.empty())
        return false;
    
    bool hasDecimal = false;
    for(int i = 0; i < str.length(); i++){
        if(str[i] >= '0' && str[i] <= '9')
            continue;
        else if(str[i] == '.' && !hasDecimal)
            hasDecimal = true;
        else if(str[i] == '-' && i == 0)
            continue;
        else
            return false;
    }
    return true;
}

// string Tool::scientificToNor(string scientific){
//     std::stringstream ss;
//     double number;

//     ss << scientific;
//     ss >> number;

//     ss.str("");
//     ss.clear();
//     ss << std::fixed << number;

//     return ss.str();
// }

string Tool::getFormulafromGraph(string filePath){
    vector<string> graphItems = split(readFile(filePath), '\n');
    string formula;
    for(auto it = graphItems.begin(); it < graphItems.end(); it++){
        string recentLine = *it;
        if(recentLine == "Graph:"){
            formula = *(it + 1);
            break;
        }
    }
    return formula;
}

void Tool::copyDirectory(const string& strSourceDir, const string& strDestDir)
{
    boost::filesystem::recursive_directory_iterator end;
    boost::system::error_code ec;  
    for (boost::filesystem::recursive_directory_iterator pos(strSourceDir); pos != end; ++pos)  
    {  
        if(boost::filesystem::is_directory(*pos))  
            continue;  
        std::string strAppPath = boost::filesystem::path(*pos).string();  
        std::string strRestorePath;  
    
        size_t strSourceDirLength = strSourceDir.length();
        string strSourceDirNew = strSourceDir;
        if (strSourceDir[strSourceDirLength-1]=='/')
        {
            strSourceDirNew = strSourceDirNew.erase(strSourceDirLength-1,1);
        }
        size_t strDestDirLength = strDestDir.length();
        string strDestDirNew = strDestDir;
        if (strDestDir[strDestDirLength-1]=='/')
        {
            strDestDirNew = strDestDirNew.erase(strDestDirLength-1,1);
        }
        boost::replace_first_copy(std::back_inserter(strRestorePath), strAppPath, strSourceDirNew, strDestDirNew);  
        if(!boost::filesystem::exists(boost::filesystem::path(strRestorePath).parent_path()))  
        {  
            boost::filesystem::create_directories(boost::filesystem::path(strRestorePath).parent_path(), ec);  
        }  
        boost::filesystem::copy_file(strAppPath, strRestorePath, bfs::copy_option::overwrite_if_exists, ec);  
    } 
/*
    if(ec)  
    {  
        return false;  
    }  
    return true; 
*/
    
}

bool Tool::isSystemKeyword(string variable){
    set<string> keywords = {
        "always", "and", "assign", "begin", "buf", "bit", "bufifo0", "bufif1", "case", 
        "casex", "casez", "cmos", "deassign", "default", "defparam", "disable", 
        "edge", "else", "end", "endcase", "endmodule", "endfunction", 
        "endprimitive", "endspecify", "endtable", "endtask", "event", "for", 
        "force", "forever", "fork", "function", "highz0", "highzl", "if", 
        "initial", "inout", "input", "integer", "join", "large", "macromodule", 
        "mediumn", "module", "nand", "negedge", "nmos", "nor", "not", "notif0", 
        "notifl", "or", "output", "parameter", "pmos", "posedge", "primitive", 
        "pull0", "pull1", "pullup", "pulldown", "remos", "reg", "releses", 
        "repeat", "mmos", "rpmos", "rtran", "rtranif0", "rtranif1", "scalared", 
        "small", "specify", "specparam", "strength", "strong0", "strongl", 
        "supply0", "supply1", "table", "task", "time", "tran", "tranif0", 
        "tranif1", "tri", "tri0", "tri1", "triand", "trior", "trireg", 
        "vectored", "wait", "wand", "weak0", "weak1", "while", "wire", "wor", 
        "xnor", "xor", "dut", "do"
    };
    return keywords.find(variable) != keywords.end();
}

string Tool::replacePartStr(string str, string oldStr, string newStr){
    size_t pos = 0;
    while ((pos = str.find(oldStr, pos)) != string::npos){
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
    return str;
}

string Tool::arrayToStr(vector<int> arr){
    string result = "[";
    for(auto it = arr.begin(); it < arr.end(); it++)
        result += *it + ", ";
    result = result.substr(0, result.size() - 2);
    result += "]";
    return result;
}

void Tool::serializeBuilder(Builder builder, string targetDirectory) {
    if(targetDirectory != "")
        targetDirectory += "/";
    ofstream ofs(targetDirectory + builder.getProgramName() + ".txt", ios::binary);
    if(!ofs.is_open()){
        Tool::error("Error: Failed to open file " + builder.getProgramName() + ".txt" + " for writing.");
    }
    try{
        boost::archive::text_oarchive archive(ofs);
        archive << builder;
        cout << "Builder has been serialized to file " << builder.getProgramName() + ".txt" << endl;
    } catch(const exception& e) {
        Tool::error("Error: Failed to serialize Builder object");
    }

    ofs.close();
}

Builder Tool::deserializeBuilder(const string& filename) {
    Builder builder("", 10, 15);

    ifstream ifs(filename, ios::binary);
    if(!ifs.is_open()){
        Tool::error("Error: Failed to open file " + filename + " reading.");
    }
    try{
        boost::archive::text_iarchive archive(ifs);
        archive >> builder;
        cout << "from " << filename << " read builder object" << endl;
    } catch(const exception& e) {
        Tool::error("Error: Failed to deserialize Builder object");
    }
    ifs.close();
    return builder;
}

double Tool::twoPointsDistance(pair<int, int> point1, pair<int, int> point2){
    double result = pow(point1.first - point2.first, 2) + pow(point1.second - point2.second, 2);
    return sqrt(result);
}

int Tool::isConditionAbsolute(string verilog, int index)
{
    if (!Py_IsInitialized()) {
        cout << "python initialize failed!" << endl;
        return 0;
    }
    //run without cmake please replace this line
    string pythonCode = string("import sys\nsys.path.append('") + getenv("VGENERATOR_WORK_DIR") + "/lib')";
    // string pythonCode = string("import sys\nsys.path.append('./lib')");
    PyRun_SimpleString(pythonCode.c_str());
    PyObject* pModule = PyImport_ImportModule("isConditionAbsolute");
    if (pModule == nullptr) {
        Tool::logMessage("Module not found! Please confirm the location of isConditionAbsolute.py");
        Py_XDECREF(pModule);
        return 0;
    }
    else {
        PyObject* pClass = PyObject_GetAttrString(pModule, "isConditionAbsolute");
        PyObject* pInstance;
        PyObject* pyValue;
        if (pClass && PyCallable_Check(pClass)) {
            pInstance = PyObject_CallObject(pClass, nullptr);
            if (pInstance != nullptr) {
                pyValue = PyObject_CallMethod(pInstance, "analyze_verilog", "si", verilog.c_str(), index);
                if (pyValue == nullptr) {
                    PyErr_Print();
                    return 0;
                }
            }
        }
        int res;
        if (!PyArg_Parse(pyValue, "i", &res)) {
            PyErr_Print();
        }
        Py_XDECREF(pModule);
        Py_XDECREF(pClass);
        Py_XDECREF(pInstance);
        Py_XDECREF(pyValue);
        return res;
    }
}

int Tool::getLastNumberInBrackt(string text){
    if(text.find("[") == string::npos || text.find("]") == string::npos)
        Tool::error("Error: Tool getLastNumberInBrackt find no [] in " + text);
    int leftBrackt = text.rfind("[");
    int rightBrackt = text.rfind("]");
    string num = text.substr(leftBrackt + 1, rightBrackt - leftBrackt - 1);
    return stoi(num);
}

pair<string, string> Tool::getMaxSubGraph(string graph1, string graph2){
    vector<string> graphLines1 = Tool::split(graph1, '\n');
    vector<string> graphLines2 = Tool::split(graph2, '\n');
    GraphMaxSub g1 = parseGraph(graphLines1);
    GraphMaxSub g2 = parseGraph(graphLines2);

    unordered_map<string, string> best_match;
    unordered_map<string, string> current_match;
    unordered_set<string> visited1, visited2;

    for (auto& node1 : g1.adj) {
        for (auto& node2 : g2.adj) {
            findMaxCommonSubgraph(node1.first, node2.first, g1, g2, current_match, best_match, visited1, visited2);
        }
    }

    pair<string, string> twoResult = printGraphFromMapping(best_match, g1, g2);

    // string result = "Common 1:\n";
    // result += twoResult.first + "\n";
    // result += "\nCommon 2:\n";
    // result += twoResult.second;

    return twoResult;
}

void Tool::parseGraphLine(const string& line, GraphMaxSub& graph) {
    stringstream ss(line);
    string node, nodeType;
    vector<string> nodes;

    ss >> node;
    if (ss >> nodeType) { 
        graph.addNode(node, nodeType); 
        string adjNode;
        while (ss >> adjNode) {
            graph.addNode(adjNode, ""); 
            graph.addEdge(node, adjNode);
        }
    }
}

bool Tool::containsRequiredTypes(const unordered_map<string, string>& match, const GraphMaxSub& g1, const GraphMaxSub& g2) {
    bool g1HasInput = false, g1HasOutput = false, g2HasInput = false, g2HasOutput = false;
    for (const auto& p : match) {
        const string& type1 = g1.type.at(p.first);
        const string& type2 = g2.type.at(p.second);
        if (type1 == "input") g1HasInput = true;
        else if(type1 == "output") g1HasOutput = true;

        if (type2 == "input") g2HasInput = true;
        else if(type2 == "output") g2HasOutput = true;
        if (g1HasInput && g1HasOutput && g2HasInput && g2HasOutput) 
            return true;
    }
    return false;
}

void Tool::findMaxCommonSubgraph(
    const string& node1, const string& node2,
    const GraphMaxSub& g1, const GraphMaxSub& g2,
    unordered_map<string, string>& current_match,
    unordered_map<string, string>& best_match,
    unordered_set<string>& visited1, unordered_set<string>& visited2
) {
    visited1.insert(node1);
    visited2.insert(node2);
    current_match[node1] = node2;

    if (containsRequiredTypes(current_match, g1, g2) && current_match.size() > best_match.size()) {
        best_match = current_match;
    }

    for (const auto& next1 : g1.adj.at(node1)) {
        if (!visited1.count(next1)) {
            for (const auto& next2 : g2.adj.at(node2)) {
                if (!visited2.count(next2) && g1.canMatch(next1, next2, g2)) {
                    findMaxCommonSubgraph(next1, next2, g1, g2, current_match, best_match, visited1, visited2);
                }
            }
        }
    }

    visited1.erase(node1);
    visited2.erase(node2);
    current_match.erase(node1);
}

pair<string, string> Tool::printGraphFromMapping(const unordered_map<string, string>& mapping, const GraphMaxSub& g1, const GraphMaxSub& g2) {
    unordered_map<string, set<string>> subgraph1;
    unordered_map<string, set<string>> subgraph2;
   
    unordered_map<string, string> reverse_mapping;
    for (const auto& p : mapping) {
        reverse_mapping[p.second] = p.first;
    }

    for (const auto& pair : mapping) {
        const string& g1_node = pair.first;
        const string& g2_node = pair.second;

        if (g1.adj.find(g1_node) != g1.adj.end()) {
            subgraph1[g1_node];
            for (const string& adj : g1.adj.at(g1_node)) {
                if (mapping.find(adj) != mapping.end() && find(g2.adj.at(g2_node).begin(), g2.adj.at(g2_node).end(), mapping.at(adj)) != g2.adj.at(g2_node).end()) {
                    subgraph1[g1_node].insert(adj);
                }
            }
        }

        if (g2.adj.find(g2_node) != g2.adj.end()) {
            subgraph2[g2_node];
            for (const string& adj : g2.adj.at(g2_node)) {
                if (reverse_mapping.find(adj) != reverse_mapping.end() && subgraph1[g1_node].find(reverse_mapping[adj]) != subgraph1[g1_node].end()) {
                    subgraph2[g2_node].insert(adj);
                }
            }
        }
    }

    string graphString1 = generateSubgraphString(subgraph1, g1);
    string graphString2 = generateSubgraphString(subgraph2, g2);
    return {graphString1, graphString2};
}

GraphMaxSub Tool::parseGraph(const vector<string>& lines) {
    GraphMaxSub graph;
    for (const string& line : lines) {
        parseGraphLine(line, graph);
    }
    return graph;
}

double Tool::getManhattan(pair<int, int> point1, pair<int, int> point2)
{
    double result = abs(point1.first - point2.first) + abs(point1.second - point2.second);
    return result;
}

string Tool::repeatModify(string &line, int startIndex, int endIndex, vector<string> existItems)
{
    string modifyName = line.substr(startIndex, endIndex - startIndex);
    do{
        string tieNum = "";
        int numStartIndex = -1;
        for(int cIndex = modifyName.size() - 1; cIndex >= 0; cIndex--){
            char recentB = modifyName[cIndex];
            if(recentB >= '0' && recentB <= '9')
                tieNum = recentB + tieNum;
            else{
                numStartIndex = cIndex + 1;
                break;
            }
        }
        if(numStartIndex != modifyName.size())
            tieNum = to_string(stoi(modifyName.substr(numStartIndex, modifyName.size() - numStartIndex)) + 1);
        if(tieNum == "")
            tieNum = "1";
        modifyName = modifyName.substr(0, numStartIndex) + tieNum;
    }while(find(existItems.begin(), existItems.end(), modifyName) != existItems.end());
    
    line.replace(startIndex, endIndex - startIndex, modifyName);
    return modifyName;
}

bool Tool::ifEscapeName(string name)
{
    for(char c : name){
        if(c >= '0' && c <= '9')
            continue;
        if(c >= 'A' && c <= 'Z')
            continue;
        if(c >= 'a' && c <= 'z')
            continue;
        if(c == '_')
            continue;
        return true;
    }
    return false;
}

string Tool::generateSubgraphString(const unordered_map<string, set<string>>& subgraph, const GraphMaxSub& g) {
    stringstream ss;
    for (const auto& node : subgraph) {
        const string& nodeName = node.first;
        const string& nodeType = g.type.at(nodeName);
        ss << nodeName << " " << nodeType;
        for (const string& adj : node.second) {
            const string& adjType = g.type.at(adj);
            ss << " " << adj; 
        }
        ss << "\n";
    }
    return ss.str();
}
