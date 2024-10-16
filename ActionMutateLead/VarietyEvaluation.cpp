#include "VarietyEvaluation.h"
#include "../Tool.h"

string VarietyEvaluation::fromVtoFormularG(string filePath, string mainModule){
    string content = formatVerilog(filePath);
    vector<string> lines = split(content, '\n');
    string moduleName = "";

    // first scan: confirm inputs
    vector<string> inputs; 
    for(int i = 0; i < lines.size(); i++){
        lines[i] = Tool::washString(lines[i]);
        string recentLine = lines[i];
        vector<string> items = split(recentLine, ' ');
        int moduleFlag = moduleProcess(items, moduleName);
        if (moduleName != mainModule || moduleFlag == 1) {
            continue;
        }
        if (moduleFlag == -1)
            break;
        else if (items[0] == "input"){   //collect input
            for(int j = 0; j < items.size(); j++){
                string item = items[j];
                item = washVarName(item);
                if(item.find("[") == string::npos && item != "reg" && item != "wire" && item != "")
                    inputs.push_back(item);
            }
        } 
    }

    // second scan: replace called modules
    string newContent = "";
    for(int i = 0; i < lines.size(); i++){
        lines[i] = Tool::washString(lines[i]);
        string recentLine = lines[i];
        vector<string> items = split(recentLine, ' ');
        int moduleFlag = moduleProcess(items, moduleName);
        if (moduleName != mainModule || moduleFlag == 1) {
            continue;
        }
        if(moduleFlag == -1)
            break;
        else if(items.size() > 1 && items[1].find(".") != string::npos){   //module execute
            newContent += moduleReplace(recentLine, content);
            continue;
        }
        //exclude pre definition.
        if(items[0] == "input" || items[0] == "output" || items[0] == "wire" || items[0] == "reg")
            continue;
        
        newContent += recentLine;
        if(i + 1 != lines.size())
            newContent += "\n";
    }
    if(newContent.back() == '\n')
        newContent = newContent.substr(0, newContent.size() - 1);
    lines = split(newContent, '\n');

    // third scan: dependency build
    map<string, set<string>> var2Nodes;
    map<string, map<string, vector<string>>> always2Nodes;
    map<string, set<string>> nodes2Dependency;
    // process inputs
    for (auto input : inputs) {
        nodes2Dependency[input] = set<string>();
    }
    // process always
    for (int i = 0; i < lines.size(); i++){
        string recentLine = lines[i];
        vector<string> items = split(recentLine, ' ');
        if (items[0] == "always") {
            string alwaysNode = "always_" + to_string(i);
            nodes2Dependency[alwaysNode] = set<string>();
            vector<string> dependNode = {alwaysNode};
            int levels = 1; // layers inside 'always'
            vector<vector<string>> lastDep = {vector<string>{alwaysNode}};

            if ((recentLine.find("posedge") == string::npos) && (recentLine.find("negedge") == string::npos)) {
                // Combined circuit
                set<string> multiDepend;
                bool afterEnd = false;
                while (levels > 0) {
                    i++;
                    string processLine = lines[i];
                    vector<string> processItems = split(processLine, ' ');
                    if (processItems[0] == "if" || processItems[0] == "else") {
                        levels++;
                        string ifNode = processItems[0] + "_" + to_string(i);
                        if (processItems[0] == "if" && afterEnd) {
                            lastDep[lastDep.size() - 1].assign(multiDepend.begin(), multiDepend.end());
                            nodes2Dependency[ifNode] = multiDepend;
                            multiDepend.clear();
                            afterEnd = false;
                        }
                        for (auto tempDep : lastDep.back()) {
                            nodes2Dependency[ifNode].insert(tempDep);
                        }
                        dependNode = {ifNode};
                        lastDep.push_back({ifNode});
                        afterEnd = false;
                    }
                    else if (processItems[0] == "end") {
                        if (!afterEnd) {
                            for (auto tempDep : dependNode) {
                                multiDepend.insert(tempDep);
                            }
                            afterEnd = true;
                        }
                        if (levels > 1) {
                            lastDep.pop_back();
                            dependNode = lastDep.back();
                        }
                        levels--;
                    }
                    else {
                        string varName = washVarName(extractFirstVar(processLine)[0]);
                        if (varName == "") {
                            continue;
                        }
                        string varNode = varName + "_" + to_string(i);
                        var2Nodes[varName].insert(varNode);
                        always2Nodes[alwaysNode][varName].push_back(varNode);
                        lastDep[lastDep.size() - 1] = {varNode};
                        if (afterEnd) {
                            nodes2Dependency[varNode] = multiDepend;
                            multiDepend.clear();
                            afterEnd = false;
                        }
                        else {
                            for (auto tempDep : dependNode) {
                                nodes2Dependency[varNode].insert(tempDep);
                            }
                        }
                        dependNode = {varNode};
                    }
                }
                nodes2Dependency[alwaysNode] = multiDepend;
            }
            else {
                // Sequential circuit
                while (levels > 0) {
                    i++;
                    string processLine = lines[i];
                    vector<string> processItems = split(processLine, ' ');
                    if (processItems[0] == "if" || processItems[0] == "else") {
                        levels++;
                        string ifNode = processItems[0] + "_" + to_string(i);
                        for (auto tempDep : lastDep.back()) {
                            nodes2Dependency[ifNode].insert(tempDep);
                        }
                        lastDep.push_back({ifNode});
                    }
                    else if (processItems[0] == "end") {
                        if (levels > 1) {
                            lastDep.pop_back();
                        }
                        levels--;
                    }
                    else {
                        string varName = washVarName(extractFirstVar(processLine)[0]);
                        if (varName == "") {
                            continue;
                        }
                        string varNode = varName + "_" + to_string(i);
                        var2Nodes[varName].insert(varNode);
                        always2Nodes[alwaysNode][varName].push_back(varNode);
                        for (auto tempDep : lastDep.back()) {
                            nodes2Dependency[varNode].insert(tempDep);
                        }
                        nodes2Dependency[alwaysNode].insert(varNode);
                    }
                }
            }
        }
    }
    // process assign and always again
    for (int i = 0; i < lines.size(); i++) {
        string recentLine = lines[i];
        vector<string> items = split(recentLine, ' ');
        if (items[0] == "assign") {
            string leftName = washVarName(extractFirstVar(items[1])[0]);
            if (leftName == "") {
                continue;
            }
            set<string> rightNames;
            // get rightNames
            for (int j = 3; j < items.size(); j++) {
                string tempVar = washVarName(items[j]);
                if (tempVar != "" && !Tool::isNum(tempVar) && !Tool::isSystemKeyword(tempVar)) {
                    rightNames.insert(tempVar);
                }
            }
            // get dependencies
            if (rightNames.empty()) {
                nodes2Dependency[leftName] = set<string>();
            }
            for (auto rightName : rightNames) {
                if (var2Nodes[rightName].empty()) {
                    nodes2Dependency[leftName].insert(rightName);
                }
                else {
                    for (auto nodeName : var2Nodes[rightName]) {
                        nodes2Dependency[leftName].insert(nodeName);
                    }
                }
            }
            var2Nodes[leftName].insert(leftName);
        }
        else if (items[0] == "always") {
            string alwaysNode = "always_" + to_string(i);
            map<string, set<string>> innerNodes;
            set<string> allVars;

            if ((recentLine.find("posedge") != string::npos) || (recentLine.find("negedge") != string::npos)) {
                for (int j = 1; j < items.size(); j++) {
                    string tempVar = washVarName(items[j]);
                    if (tempVar != "" && !Tool::isNum(tempVar) && !Tool::isSystemKeyword(tempVar)) {
                        innerNodes[tempVar] = set<string>();
                        allVars.insert(tempVar);
                    }
                }
            }

            // process always statements in lines
            vector<string> alwaysLines{recentLine};
            int levels = 1;
            while (levels > 0) {
                i++;
                vector<string> recentItems = split(lines[i], ' ');
                if (recentItems[0] == "end") {
                    levels--;
                }
                else if (recentItems[0] == "if" || recentItems[0] == "else") {
                    levels++;
                    for (int j = 1; j < recentItems.size(); j++) {
                        string tempVar = washVarName(recentItems[j]);
                        if (tempVar != "" && !Tool::isNum(tempVar) && !Tool::isSystemKeyword(tempVar)) {
                            innerNodes[tempVar] = set<string>();
                            allVars.insert(tempVar);
                        }
                    }
                }
                else {
                    for (int j = 2; j < recentItems.size(); j++) {
                        string tempVar = washVarName(recentItems[j]);
                        if (tempVar != "" && !Tool::isNum(tempVar) && !Tool::isSystemKeyword(tempVar)) {
                            innerNodes[tempVar] = set<string>();
                            allVars.insert(tempVar);
                        }
                    }
                }
            }
            // get varNodes
            for (auto alwaysPair : always2Nodes) {
                if (alwaysPair.first == alwaysNode)
                    continue;
                map<string, vector<string>> v2n = alwaysPair.second;
                for (auto vn : v2n) {
                    if (allVars.find(vn.first) != allVars.end()) {
                        for (auto node : vn.second) {
                            innerNodes[vn.first].insert(node);
                        }
                    }
                }
            }
            // update nodes2Dependency
            for (auto nodePair : innerNodes) {
                string key = nodePair.first;
                if (innerNodes[key].empty() && always2Nodes[alwaysNode].find(key) == always2Nodes[alwaysNode].end()) {
                    innerNodes[key].insert(key);
                }
                set<string> nodes = innerNodes[key];
                for (auto node : nodes) {
                    nodes2Dependency[alwaysNode].insert(node);
                }
            }
        }
    }
    
    // formular output
    string result = "";
    for (auto nodeAnddep : nodes2Dependency) {
        string tempDep = nodeAnddep.first;
        for (auto dep : nodeAnddep.second) {
            tempDep += " " + dep;
        }
        // result += tempDep + ";";
        result += tempDep + ";";
    }
    result = result.substr(0, result.size() - 1);
    return result;
}

bool VarietyEvaluation::isOperateSyntax(string note){
    set<string> keywords = {"=", "+", "-", "*", "/", "<", ">", "@", "!", "\\", "\n", "\t",
        "(", ")", "[", "]", "{", "}", ",", "&", "^", "?", ";", "|", " ", "$", "~", "#", "%"};
    return keywords.find(note) != keywords.end();
}

string VarietyEvaluation::washVarName(string rawVar){
    string tempVar = rawVar.find("[") == string::npos ? rawVar : rawVar.substr(0, rawVar.find("["));
    string newVar = "";
    if(!Tool::isSystemKeyword(tempVar)){
        for(int k = 0; k < tempVar.size(); k++){
            char c = tempVar[k];
            string singleChar(1, c);
            if(!isOperateSyntax(singleChar))
                newVar += singleChar;
        }
    }
    if(newVar.size() > 0 && newVar[0] >= '0' && newVar[0] <= '9')
        newVar = "";
    return newVar;
}

int VarietyEvaluation::moduleProcess(vector<string> items, string& moduleName){
    if(items[0] == "module"){
        moduleName = items[1].substr(0, items[1].find('('));
        return 1;
    }
    else if(items[0] == "endmodule"){
        // moduleName = "";
        return -1;
    }
    return 0;
}

string VarietyEvaluation::moduleReplace(string callLine, string content){
    string result = "";
    vector<string> lines = split(content, '\n');

    //call line info extract
    vector<string> callLineItems = split(callLine, ' ');
    map<string, string> interfaceandPass;
    string callModule = callLineItems[0];
    for(int i = 1; i < callLineItems.size(); i++){
        string recentItem = callLineItems[i];
        while(count(recentItem.begin(), recentItem.end(), '(') > 1)
            recentItem = recentItem.substr(recentItem.find('(') + 1, recentItem.size() - recentItem.find('(') - 1);
        while(count(recentItem.begin(), recentItem.end(), ')') > 1)
            recentItem = recentItem.substr(0, recentItem.find(')'));
        if(recentItem[0] == '.')
            recentItem = recentItem.substr(1, recentItem.size() - 1);
        int leftBracket = recentItem.find('(');
        string varInterface = recentItem.substr(0, leftBracket);
        string varPass = recentItem.substr(leftBracket + 1, recentItem.find(')') - leftBracket - 1);
        interfaceandPass[varInterface] = varPass;
    }   

    //module replace
    bool findModule = false;
    for(int i = 0; i < lines.size(); i++){
        string recentLine = lines[i];
        vector<string> items = split(recentLine, ' ');
        if(items[0] == "module"){
            string moduleName = items[1].substr(0, items[1].find('('));
            if(moduleName == callModule){
                findModule = true;
                continue;
            }
        }
        if (!findModule) {
            continue;
        }
        
        //exclude pre definition.
        if(items[0] == "input" || items[0] == "output" || items[0] == "wire" || items[0] == "reg")
            continue;
        else if(items[0] == "endmodule" && findModule)
            break;
        else {
            string recentNewLine = "";
            vector<string> vars = splitVar(lines[i]);
            if (items.size() > 1 && items[1].find(".") != string::npos){   //module execute
                for(auto it = vars.begin(); it < vars.end(); it++){
                    string recentVar = *it;
                    if(interfaceandPass.find(recentVar) != interfaceandPass.end()) {
                        recentLine = replaceVar(items, interfaceandPass);
                        break;
                    }
                }
                recentNewLine = moduleReplace(recentLine, content);
            }
            else {
                recentNewLine = recentLine;
                for(auto it = vars.begin(); it < vars.end(); it++){
                    string recentVar = *it;
                    if(interfaceandPass.find(recentVar) != interfaceandPass.end()) {
                        recentNewLine = replaceVar(items, interfaceandPass);
                        break;
                    }
                }
            }
            if(i + 1 != lines.size())
                recentNewLine += "\n";
            result += recentNewLine;
        }
    }
    return result;
}

string VarietyEvaluation::replaceVar(vector<string> items, map<string, string> replaceRelation) {
    vector<string> keys;
    string res = "";
    
    for (auto it = replaceRelation.begin(); it != replaceRelation.end(); ++it) {
        keys.push_back(it->first);
    }

    for (auto it = items.begin(); it < items.end(); it++) {
        for (auto key = keys.begin(); key < keys.end(); key++) {
            size_t pos = it->find("(" + *key + ")");
            if (pos != string::npos) {
                it->replace(pos + 1, key->length(), replaceRelation[*key]);
                break;
            }
            else {
                vector<string> onlyVar = splitVar(*it);
                bool isequal = false;
                for (auto var : onlyVar) {
                    if (var == *key) {
                        isequal = true;
                        break;
                    }
                }
                if (isequal) {
                    it->replace(it->find(*key), key->length(), replaceRelation[*key]);
                    break;
                }
            }
        }
        res += *it + " ";
    }
    return res.substr(0, res.length() - 1);
}

vector<string> VarietyEvaluation::splitVar(string line){
    string result = "";

    int lastVar = 0;
    for(int index = 0; index < line.length(); index++){    
        string recentChar = line.substr(index, 1);
        if(!isOperateSyntax(recentChar) || recentChar == "."){
            result += recentChar;
            lastVar = 1;
        }
        else if (lastVar == 1){
            result += " ";
            lastVar = 0;
        }
    }
    
    if (result[result.length()] == '\n')
        result = result.substr(0, line.size() - 1);
    return split(result, ' ');
}

vector<string> VarietyEvaluation::extractFirstVar(string statement){
    vector<string> items = split(statement, ' ');
    vector<string> result;
    if(items[0] == "assign")
        result.push_back(washVarName(items[1]));
    else if(items[0] == "always" && statement.find('\n') != string::npos){
        vector<string> alwaysLines = split(statement, '\n');
        for(int h = 1; h < alwaysLines.size() - 1; h++){
            string thisLine = alwaysLines[h];
            string firstVar = extractFirstVar(thisLine)[0];
            result.push_back(firstVar);
        }
    }
    else{
        string tempVar = Tool::washString(items[0]);
        if(tempVar != "" && !Tool::isNum(tempVar) && !Tool::isSystemKeyword(tempVar)){
            result.push_back(tempVar);
        }
    }
    if(result.size() == 0)
        result.push_back("");
    return result;
}

vector<string> VarietyEvaluation::split(string line, char ch) {
    vector<string> res = Tool::split(line, ch);
    for (auto it = res.end() - 1; it >= res.begin(); it--) {
        if (*it == "") {
            res.erase(it);
        }
    }
    return res;
}

string VarietyEvaluation::formatVerilog(string filePath){
    ifstream file(filePath);
    if(!file.is_open()){
        Tool::error("Error opening file: " + filePath);
    }
    stringstream ss;
    ss << file.rdbuf();
    string content = ss.str();
    vector<string> moduleNames = Tool::getAllModuleNames(content);
    vector<string> otherVariables;
    stringstream result;
    for(auto& n : moduleNames){
        stringstream module = Tool::getModule(filePath, n);
        stringstream newModule;
        vector<string> outputVariables;
        vector<string> toModify;
        vector<string> modified;
        vector<string> seg;
        string line;
        while(getline(module, line)){
            if(line.find("output") != string::npos){
                seg = Tool::split(line, ' ');
                outputVariables.push_back(seg.back().substr(0, seg.back().length()-1));
            }
            else if(line.find("reg") != string::npos){
                seg = Tool::split(line, ' ');
                string regName = seg.back().substr(0, seg.back().length()-1);
                if(find(outputVariables.begin(), outputVariables.end(), regName) != outputVariables.end()){
                    toModify.push_back(regName);
                }
            }
            else continue;
        }
        module = Tool::getModule(filePath, n);
        while(getline(module, line)){
            size_t start = line.find_first_not_of(" \t\n\r\f\v");
            line = line.substr(start);
            if(size_t pos = line.find("output") != string::npos){
                seg = Tool::split(line, ' ');
                string outputVar = seg.back().substr(0, seg.back().length()-1);
                if(find(toModify.begin(), toModify.end(), outputVar) != toModify.end()){
                    if(pos != string::npos){
                        line.insert(pos + strlen("output"), "reg ");
                    }
                }
                else{
                    if(line.find("wire") == string::npos){
                        line.insert(pos + strlen("output"), "wire ");
                    }
                }
                newModule << line + "\n";
            }
            else if(line.find("reg") != string::npos){
                seg = Tool::split(line, ' ');
                string regName = seg.back().substr(0, seg.back().length()-1);
                if(find(toModify.begin(), toModify.end(), regName) == toModify.end()){
                    newModule << line + "\n";
                } 
            }
            else if(size_t pos = line.find("input") != string::npos){
                if(line.find("wire") == string::npos){
                    line.insert(pos + strlen("input"), "wire ");
                }
                newModule << line + "\n";
            }
            else{
                newModule << line + "\n";
            }
        }
        result << newModule.str() + "\n";
    }
    string line;
    stringstream resCopy(result.str());
    while(getline(result, line)){
        if((line.find("wire") != string::npos || line.find("reg") != string::npos) && line.find("input") == string::npos && line.find("output") == string::npos){
            vector<string> seg = Tool::split(line, ' ');
            otherVariables.push_back(seg.back().substr(0, seg.back().length()-1));
        }
    }
    unordered_map<string, int> countMap;
    vector<string> duplicates;
    for(string num : otherVariables){
        countMap[num]++;
    }
    for(auto& pair : countMap){
        if (pair.second > 1) {
            duplicates.insert(duplicates.end(), pair.second - 1, pair.first);
        }
    }

    stringstream result2;
    vector<string> allModifiedVariables;
    for(auto& n : moduleNames){
        stringstream module;
        string line;
        bool insideModule = false;
        while(getline(resCopy, line)){
            if (line.find("module " + n + "(") != string::npos) insideModule = true;   
            if (insideModule) {
                module << line << "\n";
                if (line.find("endmodule") != string::npos) {
                    insideModule = false;
                    break;
                }
            }
        }
        stringstream copy(module.str());
        vector<string> seg;
        vector<string> ioVariables;
        vector<string> modifyVariables;
        while(getline(module, line)){
            if(line.find("input") != string::npos || line.find("output") != string::npos){
                seg = Tool::split(line, ' ');
                ioVariables.push_back(seg.back().substr(0, seg.back().length()-1));
            }
            else{
                for (auto it = duplicates.begin(); it != duplicates.end();) {
                    auto& v = *it;
                    if (!v.empty() && line.find(v + ";") != string::npos && find(modifyVariables.begin(), modifyVariables.end(), v) == modifyVariables.end()) {
                        modifyVariables.push_back(v);
                        it = duplicates.erase(it);
                        continue;
                    } else {
                        ++it;
                    }
                }
            }
        }
        int suffix = 1;
        vector<string> oriModifyVariables = modifyVariables;
        for(auto& v : modifyVariables){
            while(find(ioVariables.begin(), ioVariables.end(), v) != ioVariables.end() ||
            find(duplicates.begin(), duplicates.end(), v) != duplicates.end() ||
            find(allModifiedVariables.begin(), allModifiedVariables.end(), v) != allModifiedVariables.end() ||
            Tool::isSystemKeyword(v) || find(otherVariables.begin(), otherVariables.end(), v) != otherVariables.end()){
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
            allModifiedVariables.push_back(v);
        }
        stringstream newModule;
        while(getline(copy, line)){
            for(size_t i = 0; i < oriModifyVariables.size(); i++){
                size_t pos = line.find(oriModifyVariables[i]);
                while(pos != string::npos){
                    char prevChar = (pos > 0) ? line[pos - 1] : ' ';
                    char nextChar = (pos + oriModifyVariables[i].length() < line.length()) ? line[pos + oriModifyVariables[i].length()] : ' ';
                    if((isspace(prevChar) || prevChar == '(') && (isspace(nextChar) || nextChar == ',' || nextChar == ')' || nextChar == ';' || nextChar == '[')){
                        line.replace(pos, oriModifyVariables[i].length(), modifyVariables[i]);
                    }
                    pos = line.find(oriModifyVariables[i], pos + modifyVariables[i].length());
                }
            }
            newModule << line << '\n';
        }
        result2 << newModule.str() << '\n';
    }
    return result2.str();
}

string VarietyEvaluation::turnGrammar(string content){
    vector<string> lines = Tool::split(content, '\n');
    vector<string> moduleNames = Tool::getAllModuleNames(content);
    return "";
}


/*
string VarietyEvaluation::formatStrVerilog(string content){
    vector<string> moduleNames = Tool::getAllModuleNames(content);
    vector<string> otherVariables;
    stringstream result;
    for(auto& n : moduleNames){
        stringstream module = Tool::getModule(filePath, n);
        stringstream newModule;
        vector<string> outputVariables;
        vector<string> toModify;
        vector<string> modified;
        vector<string> seg;
        string line;
        while(getline(module, line)){
            if(line.find("output") != string::npos){
                seg = Tool::split(line, ' ');
                outputVariables.push_back(seg.back().substr(0, seg.back().length()-1));
            }
            else if(line.find("reg") != string::npos){
                seg = Tool::split(line, ' ');
                string regName = seg.back().substr(0, seg.back().length()-1);
                if(find(outputVariables.begin(), outputVariables.end(), regName) != outputVariables.end()){
                    toModify.push_back(regName);
                }
            }
            else continue;
        }
        module = Tool::getModule(filePath, n);
        while(getline(module, line)){
            size_t start = line.find_first_not_of(" \t\n\r\f\v");
            line = line.substr(start);
            if(size_t pos = line.find("output") != string::npos){
                seg = Tool::split(line, ' ');
                string outputVar = seg.back().substr(0, seg.back().length()-1);
                if(find(toModify.begin(), toModify.end(), outputVar) != toModify.end()){
                    if(pos != string::npos){
                        line.insert(pos + strlen("output"), "reg ");
                    }
                }
                else{
                    if(line.find("wire") == string::npos){
                        line.insert(pos + strlen("output"), "wire ");
                    }
                }
                newModule << line + "\n";
            }
            else if(line.find("reg") != string::npos){
                seg = Tool::split(line, ' ');
                string regName = seg.back().substr(0, seg.back().length()-1);
                if(find(toModify.begin(), toModify.end(), regName) == toModify.end()){
                    newModule << line + "\n";
                } 
            }
            else if(size_t pos = line.find("input") != string::npos){
                if(line.find("wire") == string::npos){
                    line.insert(pos + strlen("input"), "wire ");
                }
                newModule << line + "\n";
            }
            else{
                newModule << line + "\n";
            }
        }
        result << newModule.str() + "\n";
    }
    string line;
    stringstream resCopy(result.str());
    while(getline(result, line)){
        if((line.find("wire") != string::npos || line.find("reg") != string::npos) && line.find("input") == string::npos && line.find("output") == string::npos){
            vector<string> seg = Tool::split(line, ' ');
            otherVariables.push_back(seg.back().substr(0, seg.back().length()-1));
        }
    }
    unordered_map<string, int> countMap;
    vector<string> duplicates;
    for(string num : otherVariables){
        countMap[num]++;
    }
    for(auto& pair : countMap){
        if (pair.second > 1) {
            duplicates.insert(duplicates.end(), pair.second - 1, pair.first);
        }
    }

    stringstream result2;
    vector<string> allModifiedVariables;
    for(auto& n : moduleNames){
        stringstream module;
        string line;
        bool insideModule = false;
        while(getline(resCopy, line)){
            if (line.find("module " + n + "(") != string::npos) insideModule = true;   
            if (insideModule) {
                module << line << "\n";
                if (line.find("endmodule") != string::npos) {
                    insideModule = false;
                    break;
                }
            }
        }
        stringstream copy(module.str());
        vector<string> seg;
        vector<string> ioVariables;
        vector<string> modifyVariables;
        while(getline(module, line)){
            if(line.find("input") != string::npos || line.find("output") != string::npos){
                seg = Tool::split(line, ' ');
                ioVariables.push_back(seg.back().substr(0, seg.back().length()-1));
            }
            else{
                for (auto it = duplicates.begin(); it != duplicates.end();) {
                    auto& v = *it;
                    if (!v.empty() && line.find(v + ";") != string::npos && find(modifyVariables.begin(), modifyVariables.end(), v) == modifyVariables.end()) {
                        modifyVariables.push_back(v);
                        it = duplicates.erase(it);
                        continue;
                    } else {
                        ++it;
                    }
                }
            }
        }
        int suffix = 1;
        vector<string> oriModifyVariables = modifyVariables;
        for(auto& v : modifyVariables){
            while(find(ioVariables.begin(), ioVariables.end(), v) != ioVariables.end() ||
            find(duplicates.begin(), duplicates.end(), v) != duplicates.end() ||
            find(allModifiedVariables.begin(), allModifiedVariables.end(), v) != allModifiedVariables.end() ||
            Tool::isSystemKeyword(v) || find(otherVariables.begin(), otherVariables.end(), v) != otherVariables.end()){
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
            allModifiedVariables.push_back(v);
        }
        stringstream newModule;
        while(getline(copy, line)){
            for(size_t i = 0; i < oriModifyVariables.size(); i++){
                size_t pos = line.find(oriModifyVariables[i]);
                while(pos != string::npos){
                    char prevChar = (pos > 0) ? line[pos - 1] : ' ';
                    char nextChar = (pos + oriModifyVariables[i].length() < line.length()) ? line[pos + oriModifyVariables[i].length()] : ' ';
                    if((isspace(prevChar) || prevChar == '(') && (isspace(nextChar) || nextChar == ',' || nextChar == ')' || nextChar == ';' || nextChar == '[')){
                        line.replace(pos, oriModifyVariables[i].length(), modifyVariables[i]);
                    }
                    pos = line.find(oriModifyVariables[i], pos + modifyVariables[i].length());
                }
            }
            newModule << line << '\n';
        }
        result2 << newModule.str() << '\n';
    }
    return result2.str();
}*/