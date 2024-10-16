#include "Statement.h"
#include "../Tool.h"

Statement::Statement(){
    realCode = "";
    grammarType = "";
    isTerminal = 1;
}

Statement::Statement(string grammarType, string generateType, string realCode){
    //Temporary not set Checker for Terminal for they are generated by Tool.
    this->realCode = realCode;
    this->grammarType = grammarType;
    this->generateType = generateType;
    isTerminal = 1;
}

Statement::Statement(string grammarType, string generateType, int expIndex, vector<Statement> subStatement){
    this->grammarType = grammarType;
    this->subStatement = subStatement;
    this->generateType = generateType;
    this->expIndex = expIndex;
    this->isTerminal = 0;
    //Read relevant grammar from json.
    getRealCode();
}

Statement::~Statement(){

}

void Statement::setRealCode(string value){
    realCode = value;
}

void Statement::setGrammar(string grammarType){
    this->grammarType = grammarType;
}

/*Generate real code while check the vadility of new statement.*/
string Statement::getRealCode(){
    if(realCode.compare("") || isTerminal)
        return realCode;
    else if(grammarType[0] == '*'){
        string realGrammarType = grammarType.substr(1, grammarType.size() - 1);
        for(int i = 0; i < subStatement.size(); i++){
            if(!subStatement[i].getGrammarType().compare(realGrammarType))
                realCode += subStatement[i].getRealCode();
            else{
                cout << "Error: Maybe there are some grammar error in the statement." << endl;
                cout << "*Required type: " << realGrammarType << endl;
                cout << "You get: " << subStatement[i].getGrammarType() << endl;
                exit(1);
            }
        }
        return realCode;
    }
    else{
        try{
            if(grammar == "")
                grammar = Tool::findGrammar(grammarType, generateType, expIndex);
            vector<string> grammarStruct = Tool::extractGrammar(grammar);
            if(grammarStruct.size() != subStatement.size()){
                cout << "Error: Maybe there are some grammar error in the statement." << endl;
                cout << "The values of size of grammarStruct and subStatement are not same." << endl;
                exit(1);
            } 
            for(int i = 0; i < grammarStruct.size(); i++){
                string grammarItem = grammarStruct[i];
                Statement statement = subStatement[i];
                bool coupleTerminal = statement.getIsTerminal() && !Tool::isWord(grammarItem[0]);
                bool coupleVariable = !grammarItem.compare(statement.getGrammarType());
                if(coupleTerminal || coupleVariable || grammarItem[0] == '*'){
                    realCode += statement.getRealCode();
                }
                else{
                    cout << "Error: Maybe there are some grammar error in the statement." << endl;
                    cout << "The grammar item " << grammarItem[i] << " is not valid." << endl;
                    exit(1);
                }
            }
            return realCode;
        }catch(int){
            cout << "Error: Maybe there are some grammar error in the statement." << endl;
            cout << "Unknwon error." << endl;
            exit(1);
        }
    }
    
}

string Statement::getGrammarType(){
    return grammarType;
}

string Statement::getGrammar(){
    return grammar;
}

void Statement::setIsterminal(int value){
    if(value == 0 || value == 1)
        isTerminal = value;
    else
        Tool::error("Error: isTermial should be 0 or 1.");
}

void Statement::setExpIndex(int value){
    expIndex = value;
}

int Statement::getExpIndex(){
    return expIndex;
}

int Statement::getIsTerminal(){
    return isTerminal;
}

vector<Statement>* Statement::getSubStatement(){
    return &subStatement;
}

void Statement::resetSubStatement(){
    vector<Statement> sb = vector<Statement>();
    subStatement = sb;
}

vector<Attribute*>* Statement::getParameters(){
    return &parameters;
}

void Statement::codeReset(){
    if(isTerminal)
        return;
    else{
        realCode = "";
        for(auto it = subStatement.begin(); subStatement.end() > it; it++){
            it->codeReset();
            realCode += it->getRealCode();
        }
    }
}

vector<Statement*> Statement::getTargetSubStatement(string grammarType, string action){
    vector<Statement*> result;
    if(!action.compare("FIRST_LAYER")){
        for(int i = 0; i < subStatement.size(); i++){
            string recentType = subStatement[i].getGrammarType();
            if(!recentType.compare(grammarType) || (grammarType == "*" && recentType.size() >= 2 && recentType[0] == '*' && Tool::isWord(recentType[1])))
                result.push_back(&subStatement[i]);
        }
    }
    else if(!action.compare("ALL_LAYER")){
        queue<Statement*> tempStates;
        tempStates.push(this);
        while(tempStates.size() != 0){
            Statement* recentState = tempStates.front();
            tempStates.pop();
            vector<Statement>* recentSubStates = recentState->getSubStatement();
            for(int i = 0; i < recentSubStates->size(); i++){
                Statement* recentSubState = &(*recentSubStates)[i];
                string recentType = recentSubState->getGrammarType();
                bool isSearchforM = grammarType == "*";
                if(!recentType.compare(grammarType) || (recentType.size() >= 2 && recentType[0] == '*' && Tool::isWord(recentType[1]) && isSearchforM))
                    result.push_back(recentSubState);
                if(recentSubState->getSubStatement()->size() != 0)
                    tempStates.push(recentSubState);
            }
        }
    }
    else{
        string message = "Error: Invalid action type in getTargetSubStatement";
        Tool::error(message);
    }
    
    return result;
}

vector<Statement*> Statement::getTargetSubStatement(vector<string> grammarTypes, string action){
    vector<Statement*> result;
    if(!action.compare("FIRST_LAYER")){
        for(int i = 0; i < subStatement.size(); i++){
            string recentType = subStatement[i].getGrammarType();
            for(string gt : grammarTypes){
                if(!recentType.compare(gt) || (gt == "*" && recentType.size() >= 2 && recentType[0] == '*' && Tool::isWord(recentType[1])))
                    result.push_back(&subStatement[i]);
            }
        }
    }
    else if(!action.compare("ALL_LAYER")){
        queue<Statement*> tempStates;
        tempStates.push(this);
        while(tempStates.size() != 0){
            Statement* recentState = tempStates.front();
            tempStates.pop();
            vector<Statement>* recentSubStates = recentState->getSubStatement();
            for(int i = 0; i < recentSubStates->size(); i++){
                Statement* recentSubState = &(*recentSubStates)[i];
                string recentType = recentSubState->getGrammarType();
                for(string gt : grammarTypes){
                    bool isSearchforM = gt == "*";
                    if(!recentType.compare(gt) || (recentType.size() >= 2 && recentType[0] == '*' && Tool::isWord(recentType[1]) && isSearchforM))
                        result.push_back(recentSubState);
                }
                if(recentSubState->getSubStatement()->size() != 0)
                    tempStates.push(recentSubState);
            }
        }
    }
    else{
        string message = "Error: Invalid action type in getTargetSubStatement";
        Tool::error(message);
    }
    
    return result;
}

vector<Statement*> Statement::getTargetSubStatement_DFS(string grammarType){
    vector<Statement*> result, temp;
    if(isTerminal)
        Tool::error("There's no substatements.");
    for(int i = 0; i < subStatement.size(); i++){
        if(subStatement[i].getGrammarType() == grammarType){
            result.push_back(&subStatement[i]);
        }
        if(!subStatement[i].getIsTerminal()){
            temp = subStatement[i].getTargetSubStatement_DFS(grammarType);
            result.insert(result.end(), temp.begin(), temp.end());
        }
    }
    return result;
}

vector<Statement*> Statement::getTargetSubStatement_DFS_beforeS(string grammarType, bool* isfinish, Statement* currentS){
    vector<Statement*> result, temp;
    if(isTerminal)
        Tool::error("There's no substatements.");
    for(int i = 0; i < subStatement.size() && !(*isfinish); i++){
        if(subStatement[i].getGrammarType() == grammarType){
            result.push_back(&subStatement[i]);
        }
        if(!subStatement[i].getIsTerminal()){
            temp = subStatement[i].getTargetSubStatement_DFS_beforeS(grammarType, isfinish, currentS);
            result.insert(result.end(), temp.begin(), temp.end());
        }
        if(currentS == &subStatement[i]){
            *isfinish = 1;
            break;
        }
    }
    return result;
}

/*
save: turn the recent Statement into xml file
*/
string Statement::save(int tabNum, string filename){
    string recentText = "";
    vector<string> resultItem;
    string tab = "";
    while(tabNum-- > 0)
        tab += "\t";
    resultItem.push_back(tab + "<statement ");
    resultItem.push_back("grammarType=\"" + grammarType + "\" ");
    resultItem.push_back("grammar=\"" + grammar + "\" ");
    resultItem.push_back("expIndex=\"" + to_string(expIndex) + "\" ");
    resultItem.push_back("isTerminal=\"" + to_string(isTerminal) + "\"");
    resultItem.push_back(">\n");

    //Attribute save
    resultItem.push_back(tab + "\t<attributes>");
    for(auto it = parameters.begin(); it < parameters.end(); it++){
        resultItem.push_back(tab + "\t\t<param ");
        resultItem.push_back("name=\"" + (*it)->getName() + "\" ");
        resultItem.push_back("type=\"" + (*it)->getType() + "\" ");
        resultItem.push_back("dataType=\"" + (*it)->getDataType() + "\" ");
        resultItem.push_back("end=\"" + to_string((*it)->getEnd()) + "\" ");
        resultItem.push_back("isPublic=\"" + to_string((*it)->getIsPublic()) + "\"");
        resultItem.push_back("</param>\n");
    }
    resultItem.push_back(tab + "\t</attributes>\n");

    //Realcode. Notice: the code in realcode will not use tab.
    resultItem.push_back(tab + "\t<realcode>\n");
    resultItem.push_back(realCode);
    resultItem.push_back(tab + "\t</realcode>\n");

    //SubStatement save
    for(auto it = subStatement.begin(); it < subStatement.end(); it++)
        resultItem.push_back(it->save(tabNum + 1));

    resultItem.push_back(tab + "</statement>\n");

    for(auto it = resultItem.begin(); it < resultItem.end(); it++)
        recentText += *it;

    if(tabNum == 0 && filename != "")
        Tool::fileWrite(filename, recentText);
    return recentText;
}

int Statement::isContain(string grammarType, string value, string findType){
    vector<Statement*> allStates = getTargetSubStatement(grammarType, findType);
    if (allStates.size() == 0)
        return 0;
    else{
        if(value == "")
            return 1;
        for (auto it = allStates.begin(); it < allStates.end(); it++){
            string recentValue = (*it)->getRealCode();
            if(value.compare(recentValue) == 0)
                return 1;
        }
    }
    return 0;
}
