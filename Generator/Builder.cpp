
#include "Builder.h"
#include "../Tool.h"
#include "Executer.h"

using namespace std;

/*
addCommand: the number of extra add IOs
seq: The No. of the program which used for ID.
varMaxW: The max width of the var defined in the module.
*/
Builder::Builder(string programName, int addCommand, int varMaxW, int out_begin) : 
programName(programName), varMaxw(varMaxW), out_begin(out_begin)
{
    this->programName = programName;
    string grammar = Tool::findGrammar("Module", generateType);
    vector<Statement> contentStatement;
    vector<string> recentGrammarItems = Tool::extractGrammar(grammar);
    int grammarLength = recentGrammarItems.size();
    vector<int> isParamUsed;
    this->varMaxw = varMaxW;
    this->out_begin = out_begin;
    this->randaddtimes = 0;
    for(int i = 0; i < grammarLength; i++){
        if(recentGrammarItems[i] == "ModuleName"){
            Statement name("ModuleName", generateType, programName);
            contentStatement.push_back(name);
        }
        else if(recentGrammarItems[i] == "*Define"){
            vector<Statement> mDV;
            //Define output Statement
            int mVNum = mParameters.size();
            int randwr = 0;
            if(mParameters.size() == 0)
                Tool::error("Error: No parameter in module.");
            else if(mVNum == 2){
                //Only generate one output define parameter.
                Statement define;
                vector<string> attriNames;
                attriNames.push_back(mParameters[0].getName());
                define = generateDefine("output", attriNames);
                vector<Statement*> initScope = define.getTargetSubStatement("InitialScope");
                int recentEnd;
                if(initScope.size() != 0){
                    recentEnd = atoi((*(initScope[0]->getSubStatement()))[1].getRealCode().c_str());
                    Statement* endItem = &(*(initScope[0]->getSubStatement()))[1];
                    if(recentEnd > varMaxW - 1){
                        recentEnd = Tool::getRandomfromClosed(0, varMaxW);
                        Tool::setCodeinItem(endItem, to_string(recentEnd));
                    }
                    mParameters[0].setEnd(recentEnd);
                    initScope[0]->codeReset();
                }
                else
                    mParameters[0].setEnd(0);
                mDV.push_back(define);
                mParameters[0].setType("output");
                randwr = rand() % 2;
                if(randwr){
                    mParameters[0].setDataType("reg");
                    if(initScope.size() != 0){
                        define = generateDefine("reg", attriNames, 0);
                        Statement* endItem = &((*(define.getTargetSubStatement("InitialScope")[0]->getSubStatement()))[1]);
                        Tool::setCodeinItem(endItem, to_string(recentEnd));
                    }
                    else{
                        define = generateDefine("reg", attriNames, 1);
                    }
                    mDV.push_back(define);
                }
                //output used state update
                isOutpuInitial.push_back(0);
            }else{
                int randOutputNum = rand() % ((mParameters.size() - 2) - 1 + 1) + 1;
                for (int i = 0; i < randOutputNum; i++){
                    Statement define;
                    vector<string> attriNames;
                    attriNames.push_back(mParameters[i].getName());
                    mParameters[i].setType("output");
                    inputStartIndex = i + 1;
                    define = generateDefine("output", attriNames);
                    vector<Statement*> initScope = define.getTargetSubStatement("InitialScope");
                    int recentEnd;
                    if(initScope.size() != 0){
                        recentEnd = atoi((*(initScope[0]->getSubStatement()))[1].getRealCode().c_str());
                        Statement* endItem = &(*(initScope[0]->getSubStatement()))[1];
                        if(recentEnd > varMaxW){
                            recentEnd = Tool::getRandomfromClosed(1, varMaxW);
                            Tool::setCodeinItem(endItem, to_string(recentEnd));
                        }
                        initScope[0]->codeReset();
                        mParameters[i].setEnd(recentEnd);
                    }
                    else{
                        for (int i = 0; i < inputStartIndex; i++){
                            mParameters[i].setEnd(0);
                        }
                    }
                    mDV.push_back(define);
                    randwr = rand() % 2;
                    if(randwr){
                        mParameters[i].setDataType("reg");
                        if(initScope.size() != 0){
                            define = generateDefine("reg", attriNames, 0);
                            Statement* endItem = &((*(define.getTargetSubStatement("InitialScope")[0]->getSubStatement()))[1]);
                            Tool::setCodeinItem(endItem, to_string(recentEnd));
                        }
                        else{
                            define = generateDefine("reg", attriNames, 1);
                        }
                        mDV.push_back(define);
                    }
                    //output used state update
                    isOutpuInitial.push_back(0);
                }
            }
            //Define input
            Statement define;
            for (int i = inputStartIndex; i < mParameters.size(); i++){
                vector<string> names;
                names.push_back(mParameters[i].getName());
                mParameters[i].setType("input");
                mParameters[i].setIsconst(0);
                define = generateDefine("input", names);
                vector<Statement*> initScope = define.getTargetSubStatement("InitialScope");
                if(initScope.size() != 0){
                    int recentEnd = atoi((*(initScope[0]->getSubStatement()))[1].getRealCode().c_str());
                    Statement* endItem = &(*(initScope[0]->getSubStatement()))[1];
                    if(recentEnd > varMaxW){
                        recentEnd = Tool::getRandomfromClosed(1, varMaxW);
                        Tool::setCodeinItem(endItem, to_string(recentEnd));
                    }
                    mParameters[i].setEnd(recentEnd);
                    initScope[0]->codeReset();
                }
                else{
                    mParameters[i].setEnd(0);
                }
                mDV.push_back(define);
            }
            Statement mDefine("*Define", generateType, 0, mDV);
            contentStatement.push_back(mDefine);
        }
        else if(recentGrammarItems[i] == "*Action"){
            vector<Statement> mDV;
            contentStatement.push_back(Statement(recentGrammarItems[i], generateType, 0, mDV));
        }
        else{
            Statement s = generateFromItem(recentGrammarItems[i], generateType);
            if(s.getGrammarType() == "Name"){
                string parameterName = s.getRealCode();
                while(Tool::isSystemKeyword(parameterName)){
                    parameterName = generateFromItem("Name", generateType).getRealCode();
                    Tool::setCodeinItem(&s, parameterName);
                }
                Attribute param(parameterName, "undefine");
                mParameters.push_back(param);
                isParamUsed.push_back(0);
            }
            else if(s.getGrammarType() == "ConsistName"){
                Statement* parameterV = s.getTargetSubStatement("Name", "ALL_LAYER")[0];
                string recentParameter = parameterV->getRealCode();
                while(Tool::isSystemKeyword(recentParameter)){
                    recentParameter = generateFromItem("Name", generateType).getRealCode();
                    Tool::setCodeinItem(parameterV, recentParameter);
                }                
                while (isNameInParamVector(recentParameter, mParameters)) {
                    s = generateFromItem(recentGrammarItems[i], generateType);
                    recentParameter = s.getTargetSubStatement("Name", "ALL_LAYER")[0]->getRealCode();
                    while(Tool::isSystemKeyword(recentParameter)){
                        recentParameter = generateFromItem("Name", generateType).getRealCode();
                        Tool::setCodeinItem(s.getTargetSubStatement("Name", "ALL_LAYER")[0], recentParameter);
                    }
                }
                Attribute param(recentParameter, "undefine");
                mParameters.push_back(param);
                isParamUsed.push_back(0);
            }
            else if(s.getGrammarType() == "*ConsistName"){
                vector<Statement>* parameterV = s.getSubStatement();
                if(addCommand > 0){
                    for(int i = 0; i < addCommand; i++){
                        Statement newS = generateFromItem("*ConsistName", generateType, 0);
                        vector<Statement>* addParameterV = newS.getSubStatement();
                        parameterV->insert(parameterV->end(), addParameterV->begin(), addParameterV->end());
                    }
                }
                if(parameterV->size() != 0)
                    s.setIsterminal(0);
                for(auto it = parameterV->begin(); it < parameterV->end(); it++){
                    string recentParameter = it->getTargetSubStatement("Name", "ALL_LAYER")[0]->getRealCode();
                    while(Tool::isSystemKeyword(recentParameter)){
                        recentParameter = generateFromItem("Name", generateType).getRealCode();
                        Tool::setCodeinItem(it->getTargetSubStatement("Name", "ALL_LAYER")[0], recentParameter);
                    }
                    while (isNameInParamVector(recentParameter, mParameters)) {
                        *it = generateFromItem("ConsistName", generateType);
                        recentParameter = it->getTargetSubStatement("Name", "ALL_LAYER")[0]->getRealCode();
                        while(Tool::isSystemKeyword(recentParameter)){
                            recentParameter = generateFromItem("Name", generateType).getRealCode();
                            Tool::setCodeinItem(it->getTargetSubStatement("Name", "ALL_LAYER")[0], recentParameter);
                        }
                        s.codeReset();
                    }
                    Attribute param(recentParameter, "undefine");
                    mParameters.push_back(param);
                    isParamUsed.push_back(0);
                }
            }
            contentStatement.push_back(s);
        }
    }
    Statement moduleStatement("Module", generateType, 0, contentStatement);
    mainStatement = moduleStatement;
}

Builder::~Builder(){
    
}

bool Builder::isNameInParamVector(string name, vector<Attribute> parameterVector){
    for (auto it = parameterVector.begin(); it < parameterVector.end(); it++) {
        if (name == it->getName()) {
            return true;
        }
    }
    return false;
}

void Builder::info(){
    mainStatement.codeReset();
    summaryCode = mainStatement.getRealCode();
    cout << summaryCode << endl;
}

void Builder::testinfo(){
    mainStatement.codeReset();
    summaryCode = mainStatement.getRealCode();
    // cout << summaryCode << endl;
}

string Builder::getSummaryCode(){
    return summaryCode;
}

string Builder::getProgramName(){
    return programName;
}

Statement* Builder::randomAddStatement(){
    vector<Statement>* mDV = mainStatement.getTargetSubStatement("*Define", "FIRST_LAYER")[0]->getSubStatement();
    vector<Statement*> vs = mainStatement.getTargetSubStatement("*Action", "ALL_LAYER");
    vector<Statement*> vs1 = mainStatement.getTargetSubStatement("*Alwaysaction", "ALL_LAYER");
    vector<Statement*> vs2 = mainStatement.getTargetSubStatement("*EAlwaysaction", "ALL_LAYER");
    vector<Statement*> vs3 = mainStatement.getTargetSubStatement("*Ifaction", "ALL_LAYER");
    vector<Statement*> vs4 = mainStatement.getTargetSubStatement("*EIfaction", "ALL_LAYER");
    vs.insert(vs.end(), vs1.begin(), vs1.end());
    vs.insert(vs.end(), vs2.begin(), vs2.end());
    vs.insert(vs.end(), vs3.begin(), vs3.end());
    vs.insert(vs.end(), vs4.begin(), vs4.end());
    int vsLength = vs.size();
    int randInsert = vsLength == 1 ? 0 : Tool::getRandomfromClosed(0, vsLength - 1);
    Statement* ops = vs[randInsert];
    string addType = ops->getGrammarType();
    //Find avalible variables.
    vector<Statement>* town = ops->getSubStatement();
    Statement* targetS = ops;
    if(town->size() != 0){
        targetS = &(*town)[town->size() - 1];
    }

    //Generate origin Statement.
    string recentGrammarType = addType.substr(1, addType.size() - 1);
    Statement s;
    Statement* realsPtr;
    Statement defineS;
    bool isNewlyDefined = 0;
    vector<Statement*> subVar;
    int newVarEnd = 0;
    if (addType == "*Action") {
    //choose LeftName, and operate according to different Type of Statement
        int randomIndex = Tool::getRandomfromClosed(0, 1);
        vector<Attribute> avaliVar = getAvaliVar(ops);
        // 0: generate assign
        // 1: generate always

        if (randomIndex == 0) {//Assign: use exist or create new define
            s = generateFromItem(recentGrammarType, generateType, 0);
            Statement* itemLeftName = s.getTargetSubStatement("LeftName", "ALL_LAYER")[0];
            string leftName;
            string NewName = getLeftName();

            int out_use = ops->getSubStatement()->size() > out_begin;
            defineS = SureLeftName(&leftName, NewName, "wire", out_use);
            
            Tool::setCodeinItem(itemLeftName, leftName);
            isNewlyDefined = leftName == NewName;

            newVarEnd = getTargetVar(leftName).getEnd();
            subVar = s.getTargetSubStatement("RightName", "ALL_LAYER");

            //Delete the avaliable var less than assign var.
            for(auto it = avaliVar.end() - 1; it >= avaliVar.begin(); it--)
                if(it->getEnd() < newVarEnd)
                    avaliVar.erase(it);
            if(HandlerightExpr(avaliVar, subVar, &s, newVarEnd, "Assign")){
                Tool::error("Error: randomAddStatement HandlerightExpr(avaliVar, subVar, &s, newVarEnd, 'Assign') == true");
                return NULL;
            }
            
        }
        // the statement is the type of Always
        else if (randomIndex == 1) {
            s = generateFromItem(recentGrammarType, generateType, 1, true); // always
            int grammarIndex = s.getTargetSubStatement("Always", "FIRST_LAYER")[0]->getExpIndex();
            Statement* sensitiveStatement;
            Statement* alwaysStatement;
            if (grammarIndex == 0) {
                // block assign
                sensitiveStatement = s.getTargetSubStatement("SensitivityList", "ALL_LAYER")[0];
                alwaysStatement = s.getTargetSubStatement("*Alwaysaction", "ALL_LAYER")[0];
            }
            else {
                // noBlock assign
                sensitiveStatement = s.getTargetSubStatement("ESensitivityList", "ALL_LAYER")[0];
                alwaysStatement = s.getTargetSubStatement("*EAlwaysaction", "ALL_LAYER")[0];
            }
            vector<Statement*> alwaysName = sensitiveStatement->getTargetSubStatement("RightName", "ALL_LAYER");

            for(auto it = avaliVar.end() - 1; it >= avaliVar.begin(); it--)
                if(it->getDataType() == "reg")
                    avaliVar.erase(it);
            if(alwaysName.size() > avaliVar.size()){
                *sensitiveStatement = grammarIndex == 0 ? generateFromItem("SensitivityList", generateType, 0) 
                                                        : generateFromItem("ESensitivityList", generateType, 0);
            }
            else{
                for(int i = 0; i <alwaysName.size(); i++){
                    int randindex = Tool::getRandomfromClosed(0, avaliVar.size() - 1);
                    Tool::setCodeinItem(alwaysName[i], avaliVar[randindex].getName());
                    avaliVar.erase(avaliVar.begin() + randindex);
                }
            }
        }
        checkNum(&s);
        ops->getSubStatement()->push_back(s);
        realsPtr = &ops->getSubStatement()->back();
    }
    else if (addType == "*Alwaysaction") {
        int randomIndex = Tool::getRandomfromClosed(0, 5);
        // 0, 1: generate block
        // 2: generate if

        if (randomIndex <= 2) {//block
            s = generateFromItem(recentGrammarType, generateType, 0); // block
            checkNum(&s);
            ops->getSubStatement()->push_back(s);
            realsPtr = &(*ops->getSubStatement())[ops->getSubStatement()->size() - 1];
            Statement* itemLeftName = realsPtr->getTargetSubStatement("LeftName", "ALL_LAYER")[0];
            string leftName;
            string NewName = getLeftName();
            string type = "Block";

            int out_use = ops->getSubStatement()->size() > out_begin;
            defineS = SureLeftName(&leftName, NewName, "reg", out_use);
            Tool::setCodeinItem(itemLeftName, leftName);
            isNewlyDefined = leftName == NewName;

            vector<Attribute> avaliVar = getStructfrontAttribute(realsPtr, leftName);


            newVarEnd = getTargetVar(leftName).getEnd();
            subVar = realsPtr->getTargetSubStatement("RightName", "ALL_LAYER");

            //Delete the avaliable var less than assign var.
            for(auto it = avaliVar.end() - 1; it >= avaliVar.begin(); it--)
                if(it->getEnd() < newVarEnd)
                    avaliVar.erase(it);

            if(HandlerightExpr(avaliVar, subVar, realsPtr, newVarEnd, type)){
                Tool::error("Error: randomAddStatement HandlerightExpr(avaliVar, subVar, S, newVarEnd, type) == true");
                return NULL;
            }
        }
        else if (randomIndex >= 3) { // if
            s = generateFromItem(recentGrammarType, generateType, 1, true); // if
            checkNum(&s);
            ops->getSubStatement()->push_back(s);
            realsPtr = &(*ops->getSubStatement())[ops->getSubStatement()->size() - 1];
            vector<Attribute> avaliVar = getStructfrontAttribute(realsPtr);

            handle_conditions(realsPtr, avaliVar);
            
            realsPtr->codeReset();
        }
    }
    else if (addType == "*EAlwaysaction") {
        int randomIndex = Tool::getRandomfromClosed(0, 5);
        // 0, 1: generate noblock
        // 2: generate if

        if (randomIndex <= 2) { // noblock
            s = generateFromItem(recentGrammarType, generateType, 0); // noblock
            checkNum(&s);
            ops->getSubStatement()->push_back(s);
            realsPtr = &(*ops->getSubStatement())[ops->getSubStatement()->size() - 1];
            Statement* itemLeftName = realsPtr->getTargetSubStatement("LeftName", "ALL_LAYER")[0];
            string leftName;
            string NewName = getLeftName();
            string type = "NoBlock";

            int out_use = ops->getSubStatement()->size() > out_begin;
            defineS = SureLeftName(&leftName, NewName, "reg", out_use);
            Tool::setCodeinItem(itemLeftName, leftName);
            isNewlyDefined = leftName == NewName;

            vector<Attribute> avaliVar = getStructfrontAttribute(realsPtr, leftName, "seq");
            

            newVarEnd = getTargetVar(leftName).getEnd();
            subVar = realsPtr->getTargetSubStatement("RightName", "ALL_LAYER");

            //Delete the avaliable var less than assign var.
            for(auto it = avaliVar.end() - 1; it >= avaliVar.begin(); it--)
                if(it->getEnd() < newVarEnd)
                    avaliVar.erase(it);

            if(HandlerightExpr(avaliVar, subVar, realsPtr, newVarEnd, type)){
                Tool::error("Error: randomAddStatement HandlerightExpr(avaliVar, subVar, S, newVarEnd, type) == true");
                return NULL;
            }
        }
        else if (randomIndex >= 3) { // if
            s = generateFromItem(recentGrammarType, generateType, 1, true); // if
            checkNum(&s);
            ops->getSubStatement()->push_back(s);
            realsPtr = &(*ops->getSubStatement())[ops->getSubStatement()->size() - 1];
            vector<Attribute> avaliVar = getStructfrontAttribute(realsPtr, "", "seq");
            handle_conditions(realsPtr, avaliVar, "seq");
            realsPtr->codeReset();
        }
        // 2023/10/19 zjk Modify End
    }
    else if (addType == "*Ifaction") {
        int randomIndex = Tool::getRandomfromClosed(0, 2);
        // 0-1: generate block
        // 2: generate if

        if (randomIndex <= 1) { // block
            s = generateFromItem(recentGrammarType, generateType, 0); // block
            checkNum(&s);
            ops->getSubStatement()->push_back(s);
            realsPtr = &(*ops->getSubStatement())[ops->getSubStatement()->size() - 1];

            Statement* itemLeftName = realsPtr->getTargetSubStatement("LeftName", "ALL_LAYER")[0];
            string leftName;
            string NewName = getLeftName();
            string type = "Block";

            int out_use = ops->getSubStatement()->size() > out_begin;
            defineS = SureLeftName(&leftName, NewName, "reg", out_use);
            Tool::setCodeinItem(itemLeftName, leftName);
            isNewlyDefined = leftName == NewName;

            vector<Attribute> avaliVar = getStructfrontAttribute(realsPtr, leftName);
            

            newVarEnd = getTargetVar(leftName).getEnd();
            subVar = realsPtr->getTargetSubStatement("RightName", "ALL_LAYER");

            //Delete the avaliable var less than assign var.
            for(auto it = avaliVar.end() - 1; it >= avaliVar.begin(); it--)
                if(it->getEnd() < newVarEnd)
                    avaliVar.erase(it);
            if(HandlerightExpr(avaliVar, subVar, realsPtr, newVarEnd, type)){
                Tool::error("Error: randomAddStatement HandlerightExpr(avaliVar, subVar, S, newVarEnd, type) == true");
                return NULL;
            }
        }
        else if (randomIndex >= 2) { // if
            s = generateFromItem(recentGrammarType, generateType, 1, true); // if
            checkNum(&s);
            ops->getSubStatement()->push_back(s);
            realsPtr = &(*ops->getSubStatement())[ops->getSubStatement()->size() - 1];
            vector<Attribute> avaliVar = getStructfrontAttribute(realsPtr);
            handle_conditions(realsPtr, avaliVar);

            realsPtr->codeReset();
        }
    }
    else if (addType == "*EIfaction") {
        int randomIndex = Tool::getRandomfromClosed(0, 2);
        // 0-1: generate block
        // 2: generate if

        if (randomIndex <= 1) { // noblock
            s = generateFromItem(recentGrammarType, generateType, 0); // noblock
            checkNum(&s);
            ops->getSubStatement()->push_back(s);
            realsPtr = &(*ops->getSubStatement())[ops->getSubStatement()->size() - 1];

            Statement* itemLeftName = realsPtr->getTargetSubStatement("LeftName", "ALL_LAYER")[0];
            string leftName;
            string NewName = getLeftName();
            string type = "NoBlock";

            int out_use = ops->getSubStatement()->size() > out_begin;
            defineS = SureLeftName(&leftName, NewName, "reg", out_use);
            Tool::setCodeinItem(itemLeftName, leftName);
            isNewlyDefined = leftName == NewName;

            vector<Attribute> avaliVar = getStructfrontAttribute(realsPtr, leftName, "seq");
            
            newVarEnd = getTargetVar(leftName).getEnd();
            subVar = realsPtr->getTargetSubStatement("RightName", "ALL_LAYER");

            //Delete the avaliable var less than assign var.
            for(auto it = avaliVar.end() - 1; it >= avaliVar.begin(); it--)
                if(it->getEnd() < newVarEnd)
                    avaliVar.erase(it);
            if(HandlerightExpr(avaliVar, subVar, realsPtr, newVarEnd, type)){
                Tool::error("Error: randomAddStatement HandlerightExpr(avaliVar, subVar, S, newVarEnd, type) == true");
                return NULL;
            }
        }
        else if (randomIndex >= 2) { // if
            s = generateFromItem(recentGrammarType, generateType, 1, true); // if
            checkNum(&s);
            ops->getSubStatement()->push_back(s);
            realsPtr = &(*ops->getSubStatement())[ops->getSubStatement()->size() - 1];
            vector<Attribute> avaliVar = getStructfrontAttribute(realsPtr, "", "seq");
            handle_conditions(realsPtr, avaliVar, "seq");
            
            realsPtr->codeReset();
        }
    }
    
    if(isNewlyDefined)
        mDV->push_back(defineS);

    mainStatement.codeReset();
    return realsPtr;
}

string Builder::getRandomVarName(vector<Attribute> avaliVar){
    int index = Tool::getRandomfromClosed(0, avaliVar.size() - 1);
    return avaliVar[index].getName();
}

string Builder::getLeftName(){
    string NewName = generateFromItem("LeftName", generateType).getRealCode();
    while(Tool::isSystemKeyword(NewName)){
        NewName = generateFromItem("LeftName", generateType).getRealCode();
    }
    vector<Attribute> allParameters = innerParameters;
    allParameters.insert(allParameters.end(), mParameters.begin(), mParameters.end());
    bool reCheck = 0;
    do{
        reCheck = 0;
        for(int i = 0; i < allParameters.size(); i++){
            if(allParameters[i].getName() == NewName){
                NewName = generateFromItem("LeftName", generateType).getRealCode();
                while(Tool::isSystemKeyword(NewName)){
                    NewName = generateFromItem("LeftName", generateType).getRealCode();
                }
                reCheck = 1;
                break;
            }
        }
    }while(reCheck);
    return NewName;
}

Statement Builder::SureLeftName(string* leftName, string NewName, string datatype, int out_use){
    //get output names
    vector<string> outputNames;
    Statement defineS;
    int newVarEnd;
    outputNames.push_back(NewName);
    int iswire = datatype == "wire";
    if(out_use){
        for(int i = 0; i < inputStartIndex && mParameters[i].getDataType() == datatype; i++)
            if(isOutpuInitial[i] == 0 || !iswire)
                outputNames.push_back(mParameters[i].getName());
    }
    int chooseIndex = Tool::getRandomfromClosed(0, outputNames.size() - 1);
    string chooseName = outputNames[chooseIndex];
    *leftName = chooseName;

    if(chooseIndex == 0){//If use new Name, create new define statement with type wire
        defineS = getNewDefine(chooseName, datatype);
    }
    else{//else use exist output name
        Attribute chooseA = mParameters[chooseIndex - 1];
        newVarEnd = chooseA.getEnd();
        isOutpuInitial[chooseIndex - 1] = 1; //update output use
    }
    return defineS;
}

Statement Builder::getNewDefine(string chooseName, string datatype){
    Statement defineS;
    vector<string> names;
    int newVarEnd = 0;
    names.push_back(chooseName);
    defineS = generateDefine(datatype, names);
    if(!defineS.getExpIndex()){//initialScope, update new var
        Statement* initialScope = defineS.getTargetSubStatement("InitialScope", "FIRST_LAYER")[0];
        string numTypes[] = {"PositiveDoubleNumber", "PositiveNumber"};
        int isDigit = initialScope->getExpIndex();
        Statement* numItem = initialScope->getTargetSubStatement(numTypes[isDigit], "FIRST_LAYER")[0];
        newVarEnd = atoi(numItem->getRealCode().c_str());
        if(newVarEnd > varMaxw - 1){
            int temp = newVarEnd;
            newVarEnd = Tool::getRandomfromClosed(0, varMaxw - 1);
            Tool::setCodeinItem(numItem, to_string(newVarEnd));
            if(newVarEnd == 0)
                defineS = generateDefine(datatype, names, 1);
            else if(temp >= 10 && newVarEnd < 10){
                numItem->setGrammar("PositiveNumber");
                initialScope->setExpIndex(1);
            }   
        }
    }
    //else one bit, no additional execution.
    Attribute newVar(chooseName, "", newVarEnd, 0, datatype);
    innerParameters.push_back(newVar);   
    return defineS;
}

Attribute Builder::getTargetVar(string name){
    vector<Attribute> allParameters = innerParameters;
    Attribute var("", "");
    allParameters.insert(allParameters.end(), mParameters.begin(), mParameters.end());
    for(int i = 0; i < allParameters.size(); i++){
        if(name == allParameters[i].getName()){
            var =  allParameters[i];
        }
    }
    if(var.getName() == "")
        Tool::error("Error: Didn't find target var.");
    return var;
}

bool Builder::HandlerightExpr(vector<Attribute> avaliVar, vector<Statement*> subVar, Statement* s, int newVarEnd, string type){
    if(subVar.size() == 0){} //Num assign
    else if(avaliVar.size() == 0){// no avaliable parameter, use num to reset the Statement
        Statement expr = generateFromItem("Expr", generateType, 0);
        vector<Statement>* subS = s->getTargetSubStatement(type)[0]->getSubStatement();
        for(int z = 0; z < subS->size(); z++)
            if((*subS)[z].getGrammarType() == "Expr")
                (*subS)[z] = expr;
        s->codeReset();
    }
    else{
        for(int i = 0; i < subVar.size(); i++){ //choose RightName
            Statement* it = subVar[i];
            int randA = Tool::getRandomfromClosed(0, avaliVar.size() - 1);
            Attribute* recentA = &avaliVar[randA];
            string chooseName = recentA->getName();
            int recentEnd = recentA->getEnd();
            string additionScope = "";
            //Scope adjustment
            if(newVarEnd != recentEnd){
                if(newVarEnd < recentEnd){
                    int randStart = Tool::getRandomfromClosed(0, recentEnd - newVarEnd);
                    if(newVarEnd == 0)
                        additionScope = "[" + to_string(randStart) + "]";
                    else
                        additionScope = "[" + to_string(randStart + newVarEnd) + ":" + to_string(randStart) + "]";
                }
                else{
                    Tool::error("Error: The input parameter size less than the var assigend.");
                    return 1;
                }
                Tool::setCodeinItem(it, chooseName + additionScope);
            }
            else{
                Tool::setCodeinItem(it, chooseName);
            }
        }
    }
    return 0;        
}

vector<Attribute> Builder::getRegAndNotInitialOutputs() {
    vector<Attribute> result;
    for (int index = 0; index < inputStartIndex; index++) {
        if (isOutpuInitial[index] == 0) {
            // not been initialled
            result.push_back(mParameters[index]);
        }
        else {
            // the output had been initialled, the judge if its type is reg
            if (mParameters[index].getDataType() == "reg") {
                result.push_back(mParameters[index]);
            }
        }
    }
    return result;
}

vector<Attribute> Builder::getInnerRegVariables() {
    vector<Attribute> result;
    for (auto it = innerParameters.begin(); it < innerParameters.end(); it++) {
        if (it->getDataType() == "reg") {
            result.push_back(*it);
        }
    }
    return result;
}

int Builder::checker(int maxRandaddTimes, int stateIter){
    int isPinFull = 2;
    int turns = 0;
    
    int isConditionNeedCheck = 1;
    while(isPinFull == 2){
        turns++;
        if(turns ==  52)
            int a = 0;

        vector<int> states = checkPin(maxRandaddTimes, isConditionNeedCheck, stateIter);
        isPinFull = states[0];
        isConditionNeedCheck = states[1];
    }

    return isPinFull;
}

vector<int> Builder::checkPin(int maxRandaddTimes, int ifCheck, int stateIter){
    map<string, string> varPinOccupy;
    vector<Statement>* mDefine = mainStatement.getTargetSubStatement("*Define")[0]->getSubStatement(); // all the define statements
    Statement* mActionState = mainStatement.getTargetSubStatement("*Action")[0];
    vector<Statement>* mAction = mActionState->getSubStatement(); // all the action statements
    vector<Attribute*> allParameter;
    int isPinSafe = 1; //Judge if need check again.

    vector<string> allStarAlwaysactionTypes = {"*Alwaysaction", "*EAlwaysaction"};
    vector<string> allIfActionTypes = {"IfAction", "EIfAction"};
    vector<string> allElseIfActionTypes = {"ElseIfAction", "EElseIfAction"};
    vector<string> allStarIfactionTypes = {"*Ifaction", "*EIfaction"};
    vector<string> allIfTypes = {"If", "EIf"};

    for(int i = 0; i < mParameters.size(); i++){
        allParameter.push_back(&mParameters[i]);
    }
    for(int i = 0; i < innerParameters.size(); i++){
        allParameter.push_back(&innerParameters[i]);
    }

    resetIsconst();

    //Var Extract
    for(int i = 0; i < allParameter.size(); i++) {
        string endPin = "";
        for (int j = 0; j <= allParameter[i]->getEnd(); j++) {
            endPin += "0";
        }
        varPinOccupy[allParameter[i]->getName()] = endPin;
    }

    //Relation Extract
    for(int i = 0; i < mAction->size(); i++){
        Statement action = (*mAction)[i];
        if (action.isContain("Assign", "", "FIRST_LAYER")) {
            string leftName = action.getTargetSubStatement("LeftName", "ALL_LAYER")[0]->getRealCode();
            Attribute* leftA = findAttribute(leftName, 2);
            int isOutput = leftA->getType() == "output";
            vector<Statement*> rightName = action.getTargetSubStatement("RightName", "ALL_LAYER");
            if(rightName.size() != 0){
                for(int j = 0; j < rightName.size(); j++){
                    string recentName = rightName[j]->getRealCode();  
                    int closeTax = recentName.find("[");
                    if(recentName == ""){
                        Tool::error("Error: Find null RightName");
                        vector<int> resultStates = {1, 1};
                        return resultStates;
                    }
                    else if(closeTax == string::npos){
                        string fullPin = fullPinSet(varPinOccupy[recentName].size());
                        varPinOccupy[recentName] = fullPin;
                        if(isOutput){//Part Assign Execution
                            for(int k = 0; k < fullPin.size(); k++)
                                varPinOccupy[leftName][k] = '1';
                        }
                    }
                    else{
                        int colon = recentName.find(":");
                        int recentStart = 0;
                        int recentEnd = 0;
                        if(colon != string::npos){
                            recentEnd = atoi(recentName.substr(closeTax + 1, colon - closeTax - 1).c_str());
                            recentStart = atoi(recentName.substr(colon + 1, recentName.size() - 1 - colon - 1).c_str());
                        }
                        else{
                            recentStart = atoi(recentName.substr(closeTax + 1, recentName.find("]") - closeTax - 1).c_str());
                            recentEnd = recentStart;
                        }
                        recentName = recentName.substr(0, closeTax);
                       
                        for(int k = recentStart; k <= recentEnd; k++){
                            varPinOccupy[recentName][k] = '1';
                            if(isOutput){//Part Assign Execution
                                varPinOccupy[leftName][k - recentStart] = '1';
                            }
                        }
                    }
                }
            }
            else if(isOutput){
                string fullPin = fullPinSet(varPinOccupy[leftName].size());
                varPinOccupy[leftName] = fullPin;
            }
        }
        else if (action.isContain("Always", "", "FIRST_LAYER")) {
            vector<Statement*> alwaysAssign = action.getTargetSubStatement("Block", "ALL_LAYER");
            vector<Statement*> noBlockAssign = action.getTargetSubStatement("NoBlock", "ALL_LAYER");
            alwaysAssign.insert(alwaysAssign.end(), noBlockAssign.begin(), noBlockAssign.end());
            for (auto it = alwaysAssign.begin(); it < alwaysAssign.end(); it++) {
                string leftName = (*it)->getTargetSubStatement("LeftName", "ALL_LAYER")[0]->getRealCode();
                int isOutput = findAttribute(leftName, 2)->getType() == "output";
                vector<Statement*> rightName = (*it)->getTargetSubStatement("RightName", "ALL_LAYER");
                if(rightName.size() != 0){
                    for(int j = 0; j < rightName.size(); j++){
                        string recentName = rightName[j]->getRealCode();  
                        int closeTax = recentName.find("[");
                        if(recentName == ""){
                            Tool::error("Error: Find null RightName");
                            return vector<int>();
                        }
                        else if(closeTax == string::npos){
                            string fullPin = fullPinSet(varPinOccupy[recentName].size());
                            varPinOccupy[recentName] = fullPin;
                            if(isOutput){//Part Assign Execution
                                for(int k = 0; k < fullPin.size(); k++)
                                    varPinOccupy[leftName][k] = '1';
                            }
                        }
                        else{
                            int colon = recentName.find(":");
                            int recentStart = 0;
                            int recentEnd = 0;
                            if(colon != string::npos){
                                recentEnd = atoi(recentName.substr(closeTax + 1, colon - closeTax - 1).c_str());
                                recentStart = atoi(recentName.substr(colon + 1, recentName.size() - 1 - colon - 1).c_str());
                            }
                            else{
                                recentStart = atoi(recentName.substr(closeTax + 1, recentName.find("]") - closeTax - 1).c_str());
                                recentEnd = recentStart;
                            }
                            recentName = recentName.substr(0, closeTax);
                        
                            for(int k = recentStart; k <= recentEnd; k++){
                                varPinOccupy[recentName][k] = '1';
                                if(isOutput){//Part Assign Execution
                                    varPinOccupy[leftName][k - recentStart] = '1';
                                }
                            }
                        }
                    }
                }
                else if(isOutput){
                    string fullPin = fullPinSet(varPinOccupy[leftName].size());
                    varPinOccupy[leftName] = fullPin;
                }
            }

            vector<Statement*> ifCondition = action.getTargetSubStatement(allIfActionTypes, "ALL_LAYER");
            vector<Statement*> elseifCondition = action.getTargetSubStatement(allElseIfActionTypes, "ALL_LAYER");
            ifCondition.insert(ifCondition.end(), elseifCondition.begin(), elseifCondition.end());
            for(auto it = ifCondition.begin(); it < ifCondition.end(); it++){
                Statement* condition = (*it)->getTargetSubStatement("Condition", "FIRST_LAYER")[0];
                vector<Statement*> rNameS = condition->getTargetSubStatement("RightName", "ALL_LAYER");
                for(auto itname = rNameS.begin(); itname < rNameS.end(); itname++){
                    string name = (*itname)->getRealCode();
                    name = getTrueName(name);
                    string fullPin = fullPinSet(varPinOccupy[name].size());
                    varPinOccupy[name] = fullPin;
                }
            }
        }
    }

    // dead code check
    do{
        findInitialisconst();
    }while(checkisconst());
    for(auto it = mAction->begin(); it < mAction->end(); it++){
        if(it->isContain("Always", "", "FIRST_LAYER")){
            bool allIsconst = 1;
            Statement rs = *it;
            vector<Statement*> gotAlwaysaction = it->getTargetSubStatement(allStarAlwaysactionTypes, "ALL_LAYER");
            Statement* allaction = gotAlwaysaction[0];
            vector<Statement*> allRightNameS = allaction->getTargetSubStatement("RightName", "ALL_LAYER");
            
            if(allaction->getSubStatement()->size() == 0)
                continue;

            //check if this always is deadcode
            for(int j = 0; j < allRightNameS.size(); j++){
                string RName = allRightNameS[j]->getRealCode();
                RName = getTrueName(RName);
                if(!findAttribute(RName, 2)->getIsconst()){
                    allIsconst = 0;
                    break;
                }
            }

            if(allIsconst){
                isPinSafe = 0;
                int originalMinWidth = INT32_MAX;
                int candiditeMaxWidth;
                vector<int> originWidth;
                int flag = 0; 
                // flag = 0: random add statement
                // flag = 1: replace RN or Num
                // flag = 2: replace if condition

                string block = "Block";
                string ifAction = "IfAction";
                string elseIfAction = "ElseIfAction";
                string alwaysAction = "Alwaysaction";
                if(allaction->getGrammarType() == "*EAlwaysaction"){
                    block = "NoBlock";
                    ifAction = "EIfAction";
                    elseIfAction = "EElseIfAction";
                    alwaysAction = "EAlwaysaction";
                }
                vector<Statement*> allLeftNameS = allaction->getTargetSubStatement("LeftName", "ALL_LAYER");
                vector<Statement*> allassignment = allaction->getTargetSubStatement(block, "ALL_LAYER");
                vector<Statement*> allifcondition = allaction->getTargetSubStatement(ifAction, "ALL_LAYER");
                vector<Statement*> allelseifcondition = allaction->getTargetSubStatement(elseIfAction, "ALL_LAYER");
                allifcondition.insert(allifcondition.end(), allelseifcondition.begin(), allelseifcondition.end());

                //get all assignment in BFS way
                //calculate width of every leftname
                for(auto it = allLeftNameS.begin(); it < allLeftNameS.end(); it++){
                    string leftname = (*it)->getRealCode();
                    Attribute* leftnameA = findAttribute(leftname, 2);
                    int w = leftnameA->getEnd() - leftnameA->getStart() + 1;
                    originWidth.push_back(w);
                }


                //calculate the min width of RightName and Num
                if(originWidth.size() != 0)
                    originalMinWidth = *min_element(originWidth.begin(), originWidth.end());

                //calculate the max width of Mutable var
                vector<Attribute> mutVar = getAllMutableVar(allaction);
                if(mutVar.size() == 0){
                    Tool::error("Error: checkerPin() occur mutVar.size() == 0");
                    return vector<int>();
                }

                vector<int> candiditeWidth = MutVarMaxWidth(mutVar);
                candiditeMaxWidth = *max_element(candiditeWidth.begin(), candiditeWidth.end());

                if(candiditeMaxWidth >= originalMinWidth)
                    flag = 1;
                

                if(!flag){ //if can't replace RN or Num, check if there is "if"
                    if(allifcondition.size() != 0)
                        flag = 2;
                }
                else{ // if can, random replace block or if
                    int replaceif = allifcondition.size();
                    int replaceassign = allassignment.size();
                    int result = Tool::getRandomfromClosed(0, replaceif + replaceassign - 1);
                    if(result < replaceif)
                        flag = 2;
                }

                if(!flag){ //flag = 0, add a suitable statement into the always statement
                    Statement s = generateFromItem(alwaysAction, generateType, 0);
                    Statement* LeftS = s.getTargetSubStatement("LeftName", "ALL_LAYER")[0];
                    vector<Statement*> allRightS = s.getTargetSubStatement("RightName", "ALL_LAYER");
                    
                    //find a suitable leftname
                    vector<Attribute*> allreg = getinitializedreg(allaction);
                    int chosen;
                    int leftvarWidth;
                    //when all reg width is greater than max mutvar width
                    if(allreg.size() == 0){ // there's no reg
                        mAction->erase(it);
                        break;
                    }                    
                    int regMinWidth = allreg[0]->getEnd() - allreg[0]->getStart() + 1;
                    for(int i = 0; i < allreg.size(); i++){
                        int w = allreg[i]->getEnd() - allreg[i]->getStart() + 1;
                        if(w < regMinWidth)
                            regMinWidth = w;
                    }
                    if(regMinWidth > candiditeMaxWidth){// no mutvar wider than min reg
                        if(randaddtimes > maxRandaddTimes){
                            addsuitableinput(regMinWidth);
                            vector<int> resultStates = {2, 1};
                            return resultStates;
                        }
                        else{
                            int times = 5;
                            randaddtimes++;
                            bool isAssigned = false;
                            while(times-- > 0){
                                Statement* recentSPtr = randomAddStatement();
                            }
                        }
                    }
                    else{// there is mutvar wider than min reg
                        do{
                            chosen = Tool::getRandomfromClosed(0, allreg.size() - 1);
                            leftvarWidth = allreg[chosen]->getEnd() - allreg[chosen]->getStart() + 1;
                        }
                        while(leftvarWidth > candiditeMaxWidth);
                        Tool::setCodeinItem(LeftS, allreg[chosen]->getName());

                        vector<Attribute> mutvar;
                        for(int i = 0; i < mutVar.size(); i++){
                            int width = mutVar[i].getEnd() - mutVar[i].getStart() + 1;
                            if(width >= leftvarWidth)
                                mutvar.push_back(mutVar[i]);
                        }
                        if(HandlerightExpr(mutvar, allRightS, &s, leftvarWidth - 1, block)){
                            Tool::error("Error: checkPin find HandlerightExpr(mutvar, allRightS, &s, leftvarWidth - 1, block) == true");
                            return vector<int>();
                        }


                        checkNum(&s);

                        allaction->getSubStatement()->push_back(s);
                    }
                }
                else if(flag == 1){ //replace a RightName or Num with mutable variable
                    //random choose suitable origin and candidite
                    int originindex = Tool::getRandomfromClosed(0, originWidth.size() - 1);
                    while(originWidth[originindex] > candiditeMaxWidth)
                        originindex = (originindex + 1) % originWidth.size();
                    int candiditeindex = Tool::getRandomfromClosed(0, candiditeWidth.size() - 1);
                    while(candiditeWidth[candiditeindex] < originWidth[originindex])
                        candiditeindex = (candiditeindex + 1) % candiditeWidth.size();
                    
                    //replace
                    Statement* originAssignmentS = allassignment[originindex];
                    Attribute* candiditeA = &mutVar[candiditeindex];
                    replaceSwithA(originAssignmentS, candiditeA, originWidth[originindex]);
                }
                else{ //flag = 2, replace if condition
                    int ifindex = Tool::getRandomfromClosed(0, allifcondition.size() - 1);
                    Statement* replacedifCS = allifcondition[ifindex]->getTargetSubStatement("Condition", "FIRST_LAYER")[0];
                    int candiditeindex = Tool::getRandomfromClosed(0, candiditeWidth.size() - 1);
                    Attribute* candiditeA = &mutVar[candiditeindex];
                    replaceSwithA(replacedifCS, candiditeA, 0);
                }
                break; //after replacement, Isconst may change, so we should start again.
            }
        }
    }
    if(!isPinSafe){
        mainStatement.codeReset();
        vector<int> resultStates = {2, 1};
        return resultStates;
    }

    //If some output isn't assigned, randomly add an statement so that the Pin is not all "0"
    for(int i = 0; i < inputStartIndex; i++){
        string leftName = mParameters[i].getName();
        string pinOccupy = varPinOccupy[leftName];

        if(pinOccupy.find("1") == string::npos){
            // insert an assign / block / noblock statement
            isPinSafe = 0;
            string leftNameType = mParameters[i].getDataType();
            vector<Statement*> action;
            string type;
            Statement s;

            if (leftNameType == "wire") {
                // add an assign
                action = mainStatement.getTargetSubStatement("*Action", "ALL_LAYER");
                type = "Assign";
                s = generateFromItem("Action", generateType, 0);
            }
            else if (leftNameType == "reg") {
                // add into an always
                // combine begin
                action = mainStatement.getTargetSubStatement(allStarAlwaysactionTypes, "ALL_LAYER");
                if(action.size() == 0){
                    Statement* Def = mainStatement.getTargetSubStatement("*Define", "FIRST_LAYER")[0];
                    vector<Statement>* def = Def->getSubStatement();
                    for(auto it = def->begin(); it < def->end(); it++){
                        if(it->getRealCode() == "\nreg " + leftName + ";"){
                            def->erase(it);
                            break;
                        }
                    }
                    mParameters[i].setDataType("wire");
                    continue;
                }
                type = "Block";
                // type = "NoBlock";
                s = generateFromItem("Alwaysaction", generateType, 0);
                // combine end
            }
            
            // get the insert opposition
            int actionLength = action.size();
            int randInsert = actionLength == 1 ? 0 : Tool::getRandomfromClosed(0, actionLength - 1);
            Statement* ops = action[randInsert];
            vector<Statement>* town = ops->getSubStatement();
            Statement* targetS = ops;
            if (town->size() != 0) {
                targetS = &(*town)[town->size() - 1];
            }

            if (leftNameType == "reg" && ops->getGrammarType() == "*EAlwaysaction") {
                type = "NoBlock";
                s = generateFromItem("EAlwaysaction", generateType, 0);
            }
            checkNum(&s);
            ops->getSubStatement()->push_back(s);
            Statement* S = &(*ops->getSubStatement())[ops->getSubStatement()->size() - 1];
            // adjust leftName
            Statement* itemLeftName = S->getTargetSubStatement("LeftName", "ALL_LAYER")[0];
            Tool::setCodeinItem(itemLeftName, leftName);
            // adjust rightName
            vector<Statement*> rightVar = S->getTargetSubStatement("RightName", "ALL_LAYER");
            // delete the avaliable var less than leftName
            int leftNameEnd = mParameters[i].getEnd();
            // find avalible variables
            vector<Attribute> avaliVar;

            if (type == "Block") {
                avaliVar = getStructfrontAttribute(S);
            }
            else if(type == "NoBlock"){
                avaliVar = getStructfrontAttribute(S, "", "seq");
            }
            else{
                avaliVar = getAvaliVar(ops);
            }

            for (auto it = avaliVar.end() - 1; it >= avaliVar.begin(); it--)
                if (it->getEnd() < leftNameEnd || it->getName() == leftName)
                    avaliVar.erase(it);
            if (HandlerightExpr(avaliVar, rightVar, S, leftNameEnd, type))
                exit(1);

            //Push the new Statement
            S->codeReset();
        }
    }
    if (isPinSafe == 0) {
        mainStatement.codeReset();
        vector<int> resultStates = {2, 1};
        return resultStates;
    }

    // check the condition in if
    for(auto it = mAction->begin(); it < mAction->end(); it++){
        if(it->isContain("Always", "", "FIRST_LAYER")){
            Statement* allaction = it->getTargetSubStatement(allStarAlwaysactionTypes, "ALL_LAYER")[0];
            int cursafe = handleifcondtion_issafe(allaction);
            if(isPinSafe)
                isPinSafe = cursafe;
            else
                break;
        }
    }
    if(!isPinSafe){
        mainStatement.codeReset();
        vector<int> resultStates = {2, 1};
        return resultStates;
    }

    //Pin Revise
    //Assigned / Always Revise
    for(auto it = mAction->end() - 1; it >= mAction->begin(); it--){
        if (it->isContain("Assign", "", "FIRST_LAYER")){
            string leftName = it->getTargetSubStatement("LeftName", "ALL_LAYER")[0]->getRealCode();
            string pinOcuppy = varPinOccupy[leftName];
            Attribute* recentA = findAttribute(leftName, 2);

            if(pinOcuppy.find("0") == string::npos)
                continue;
            else{
                isPinSafe = 0;
                vector<pair<int, int>> scopeSet = getScopeSet(pinOcuppy);
                // all the var pins is null: complete Remove
                if(scopeSet.size() == 1 && scopeSet[0].first == 0 && scopeSet[0].second == pinOcuppy.size() - 1){
                    deleteDefineStatements(leftName);
                    deleteParameter(leftName, false);
                    for (auto p = allParameter.end() - 1; p >= allParameter.begin(); p--) {
                        if ((*p)->getName() == leftName) {
                            allParameter.erase(p);
                            break;
                        }
                    }
                    mAction->erase(it);
                }
                else{// Part of the null pin: remove the pins and revise realCode related
                    vector<pair<int, int>> usedPinSet = getComplementSet(scopeSet, recentA->getEnd());
                    int newLen = getPairVectorValueLen(usedPinSet);
                    //define adjust
                    checkPinDefineAdj(*recentA, mDefine, newLen);
                    //assign adjust
                    vector<Statement*> RNs = it->getTargetSubStatement("RightName", "ALL_LAYER");
                    for(auto rn = RNs.begin(); rn < RNs.end(); rn++){
                        Statement* assignRN = *rn;
                        string recentRName = getTrueName(assignRN->getRealCode());

                        Attribute* rnA = findAttribute(recentRName, 2);

                        int rnEnd = getUsedPinNumber(varPinOccupy[recentRName]) - 1;
                        int randStartMax = rnEnd - newLen + 1;
                        int reviseStart = Tool::getRandomfromClosed(0, randStartMax);
                        int reviseEnd = reviseStart + newLen - 1;
                        string resultName = "";
                        if(newLen == 1){
                            resultName = recentRName + "[" + to_string(reviseStart) + "]";
                            Attribute* recentRightA = findAttribute(recentRName, 2);
                            if(recentRightA->getEnd() == 0)
                                resultName = recentRName;
                        }
                        else
                            resultName = recentRName + "[" + to_string(reviseEnd) + ":" + to_string(reviseStart) + "]";
                        Tool::setCodeinItem(assignRN, resultName);
                    }
                    //used adjust
                    checkPinUsedAdj(leftName, usedPinSet, mActionState);
                    findAttribute(leftName, 2)->setEnd(newLen - 1);
                    break;
                }
            }
        }
        else if (it->isContain("Always", "", "FIRST_LAYER")) {
            Statement* always = it->getTargetSubStatement("Always", "FIRST_LAYER")[0];
            vector<Statement*> alwaysStatement;
            if (always->getExpIndex() == 0) {
                alwaysStatement = always->getTargetSubStatement("Block", "ALL_LAYER");
            }
            else if (always->getExpIndex() == 1) {
                alwaysStatement = always->getTargetSubStatement("NoBlock", "ALL_LAYER");
            }
            if (alwaysStatement.size() != 0) {
                for (auto ss = alwaysStatement.end() - 1; ss >= alwaysStatement.begin(); ss--) {
                    string leftName = (*ss)->getTargetSubStatement("LeftName", "ALL_LAYER")[0]->getRealCode();
                    string pinOcuppy = varPinOccupy[leftName];
                    Attribute* recentA = findAttribute(leftName, 2);
                    if(pinOcuppy.find("0") == string::npos)
                        continue;
                    else{
                        isPinSafe = 0;
                        vector<pair<int, int>> scopeSet = getScopeSet(pinOcuppy);
                        // all the var pins is null: complete Remove
                        if(scopeSet.size() == 1 && scopeSet[0].first == 0 && scopeSet[0].second == pinOcuppy.size() - 1){
                            int LeftTimes = LeftNametimes(leftName);
                            int deletetimes = deleteStatement_ofthisLeftName(leftName);
                            if(LeftTimes != deletetimes){
                                Tool::error("Did't delete thoroughly.");
                                return vector<int>();
                            }
                            else{
                                deleteDefineStatements(leftName);
                                deleteParameter(leftName, false);
                            }
                            for (auto p = allParameter.end() - 1; p >= allParameter.begin(); p--) {
                                if ((*p)->getName() == leftName) {
                                    allParameter.erase(p);
                                    break;
                                }
                            }
                        }
                        else { // Part of the null pin: remove the pins and revise realCode related
                            vector<pair<int, int>> usedPinSet = getComplementSet(scopeSet, recentA->getEnd());
                            int newLen = getPairVectorValueLen(usedPinSet);
                            //define adjust
                            checkPinDefineAdj(*recentA, mDefine, newLen);
                            //assign adjust
                            vector<Statement*> RNs = (*ss)->getTargetSubStatement("RightName", "ALL_LAYER");
                            for(auto rn = RNs.begin(); rn < RNs.end(); rn++){
                                Statement* assignRN = *rn;
                                string recentRName = getTrueName(assignRN->getRealCode());

                                Attribute* rnA = findAttribute(recentRName, 2);
                                // int rnEnd = rnA->getEnd(); 
                                int rnEnd = getUsedPinNumber(varPinOccupy[recentRName]) - 1;
                                int randStartMax = rnEnd - newLen + 1;
                                int reviseStart = Tool::getRandomfromClosed(0, randStartMax);
                                int reviseEnd = reviseStart + newLen - 1;
                                string resultName = "";
                                if(newLen == 1){
                                    resultName = recentRName + "[" + to_string(reviseStart) + "]";
                                    Attribute* recentRightA = findAttribute(recentRName, 2);
                                    if(recentRightA->getEnd() == 0)
                                        resultName = recentRName;
                                }  
                                else
                                    resultName = recentRName + "[" + to_string(reviseEnd) + ":" + to_string(reviseStart) + "]";
                                Tool::setCodeinItem(assignRN, resultName);
                            }
                            //used adjust
                            checkPinUsedAdj(leftName, usedPinSet, mActionState);
                            findAttribute(leftName, 2)->setEnd(newLen - 1);
                        }
                    }
                    if(!isPinSafe){
                        mainStatement.codeReset();
                        vector<int> resultStates = {2, 1};
                        return resultStates;
                    }
                }
            }
            // delete empty statements
            Statement* alwaysAction;
            if (always->getExpIndex() == 0) {
                alwaysAction = always->getTargetSubStatement("*Alwaysaction", "FIRST_LAYER")[0];
            }
            else if (always->getExpIndex() == 1) {
                alwaysAction = always->getTargetSubStatement("*EAlwaysaction", "FIRST_LAYER")[0];
            }
            vector<Statement>* recentSubStates = alwaysAction->getSubStatement();
            // combine begin
            if (recentSubStates->size() == 0) {
                isPinSafe = 0;
                mAction->erase(it);
            }
            else {
                // maybe there are empty statements(If Statement) in recent Always Statement('always')
                if(deleteEmptySubStatementsInAlways(alwaysAction)){
                    isPinSafe = 0;
                }
                // the Always Statement('always') maybe empty after delete empty statements in it
                if (alwaysAction->getSubStatement()->size() == 0) {
                    isPinSafe = 0;
                    mAction->erase(it);
                }
            }
            // combine end
        }
    }
    
    if (isPinSafe == 0) {
        mainStatement.codeReset();
        vector<int> resultStates = {2, 1};
        return resultStates;
    }


    //input delete
    vector<Statement>* mainSub = mainStatement.getSubStatement();
    vector<Statement*> allConsistName;
    Statement* firstConsitName = &(*mainSub)[4];
    Statement* mConsistName = &(*mainSub)[5];
    Statement* firstName = firstConsitName->getIsTerminal() == 0 ? firstConsitName->getTargetSubStatement("Name")[0] : nullptr;
    vector<Statement>* mcv = mConsistName->getSubStatement();
    // replace input
    bool replaceFlag = true;
    for (int i = inputStartIndex; i < mParameters.size(); i++) {
        if (varPinOccupy[mParameters[i].getName()].find("1") != string::npos) {
            replaceFlag = false;
            break;
        }
    }

    // if there're always, some input must be used. So, here only take "assign" into accont.
    if (replaceFlag) {
        isPinSafe = 0;
        vector<Statement*> allAssign = mainStatement.getTargetSubStatement("Assign", "ALL_LAYER");
        vector<Statement*> candiditRightName;
        string maxInputName;
        int inputmaxW = findmaxWidthofinput(&maxInputName);

        int Assign_randindex = Tool::getRandomfromClosed(0, allAssign.size() - 1);
        Statement* LeftNameS = allAssign[Assign_randindex]->getTargetSubStatement("LeftName", "ALL_LAYER")[0];
        string LeftNames = getTrueName(LeftNameS->getRealCode());
        Attribute* LeftNameA = findAttribute(LeftNames, 2);
        int leftW = LeftNameA->getEnd() - LeftNameA->getStart() + 1;

        if(inputmaxW < leftW){
            addsuitableinput(leftW);
            inputmaxW = findmaxWidthofinput(&maxInputName);
        }

        int inputNum = mParameters.size() - inputStartIndex;
        int inputrandIndex = Tool::getRandomfromClosed(0, inputNum - 1);
        Attribute* choseninput = &mParameters[inputrandIndex + inputStartIndex];
        int chosenW = choseninput->getEnd() - choseninput->getStart() + 1;
        while(chosenW < leftW){
            inputrandIndex = (inputrandIndex + 1) % inputNum;
            choseninput = &mParameters[inputrandIndex + inputStartIndex];
            chosenW = choseninput->getEnd() - choseninput->getStart() + 1;
        }
        
        vector<Statement*> RightNameS = allAssign[Assign_randindex]->getTargetSubStatement("RightName", "ALL_LAYER");
        vector<Statement*> NumS = allAssign[Assign_randindex]->getTargetSubStatement("Num", "ALL_LAYER");

        int replaceIndex = Tool::getRandomfromClosed(0, RightNameS.size() + NumS.size() - 1);
        
        if(replaceIndex < RightNameS.size()){
            ReplaceRightname(choseninput->getName(), RightNameS[replaceIndex]);
        }
        else{
            ReplaceNumber(choseninput->getName(), NumS[replaceIndex - RightNameS.size()]);
        }


    }
    
    if(!isPinSafe){
        mainStatement.codeReset();
        vector<int> resultStates = {2, 1};
        return resultStates;
    }

    // delete all the input of which all pin is 0
    for (int i = mcv->size() - 1; i >= 0; i--){
        auto it = mcv->begin() + i;
        string name = it->getTargetSubStatement("Name")[0]->getRealCode();
        int isInput = findAttribute(name, 2)->getType() == "input";
        if (!isInput) {
            break;
        }
        if(varPinOccupy[name].find("1") == string::npos){
            isPinSafe = 0;
            mcv->erase(it);
            deleteDefineStatements(name);
            deleteParameter(name, true);
            for (auto p = allParameter.end() - 1; p >= allParameter.begin(); p--) {
                if ((*p)->getName() == name) {
                    allParameter.erase(p);
                }
            }
        }
    }
    
    if (firstName != nullptr) {
        string name = firstName->getRealCode();
        int isInput = findAttribute(name, 2)->getType() == "input";
        if(isInput && varPinOccupy[name].find("1") == string::npos){
            isPinSafe = 0;
            Tool::setCodeinItem(firstConsitName, "");
            deleteDefineStatements(name);
            deleteParameter(name, true);
            for (auto p = allParameter.end() - 1; p >= allParameter.begin(); p--) {
                if ((*p)->getName() == name) {
                    allParameter.erase(p);
                }
            }
        }
    }
    
    if (isPinSafe == 0) {
        mainStatement.codeReset();
        vector<int> resultStates = {2, 1};
        return resultStates;
    }

    //Inout Revise
    for (auto it = mParameters.begin(); it < mParameters.end(); it++) {
        string name = it->getName();
        string pinOccupy = varPinOccupy[name];
        if (pinOccupy.find("0") == string::npos) {
            continue;
        }
        else {
            isPinSafe = 0;
            vector<pair<int, int>> scopeSet = getScopeSet(pinOccupy);
            vector<pair<int, int>> usedPinSet = getComplementSet(scopeSet, it->getEnd());
            int newLen = getPairVectorValueLen(usedPinSet);
            checkPinDefineAdj(*it, mDefine, newLen);
            checkPinUsedAdj(name, usedPinSet, mActionState);
            it->setEnd(newLen - 1);
        }
    }
    if (isPinSafe == 0) {
        mainStatement.codeReset();
        vector<int> resultStates = {2, 1};
        return resultStates;
    }

    // modify all the posedge and negedge name in ESensitivity to be input parameters.
    vector<Statement*> allESensitivity = mainStatement.getTargetSubStatement("ESensitivityList", "ALL_LAYER");
    vector<Statement*> allEConsistSensitive = mainStatement.getTargetSubStatement("*EConsistSensitiveName", "ALL_LAYER");
    for(auto it = allEConsistSensitive.begin(); it < allEConsistSensitive.end(); it++){
        (*it)->getSubStatement()->clear();
        (*it)->codeReset();
    }

    vector<string> sensitiveInput;
    vector<int> commonIndexes;
    int count = 0;
    for(Attribute attr : mParameters){
        string recentAttrName = attr.getName();
        if(attr.getType() == "input"){
            sensitiveInput.push_back(recentAttrName);
            commonIndexes.push_back(count);
            count++;
        }
    }

    for(int i = 0; i < allESensitivity.size(); i++){
        vector<Statement*> recentRightNames = allESensitivity[i]->getTargetSubStatement("RightName", "ALL_LAYER");
        vector<string> recentCLK;
        for(Statement* rn : recentRightNames){
            recentCLK.push_back(rn->getRealCode());
        }
        for(int j = recentRightNames.size() - 1; j >= 0; j--){
            string recentRightName = recentRightNames[j]->getRealCode();
            string recentFullRName = recentRightNames[j]->getRealCode();
            if(recentRightName.find("[") != string::npos){
                recentRightName = recentRightName.substr(0, recentRightName.find("["));
            }
            if(find(sensitiveInput.begin(), sensitiveInput.end(), recentRightName) == sensitiveInput.end()
                || find(recentCLK.begin(), recentCLK.begin() + j, recentFullRName) != recentCLK.begin() + j){
                vector<int> usefulIndexes(commonIndexes);
                int inIndex = -1;
                bool ifNeedReset = true;
                while(true){
                    inIndex = Tool::getRandomfromClosed(0, usefulIndexes.size() - 1);
                    if(find(recentCLK.begin(), recentCLK.end(), sensitiveInput[usefulIndexes[inIndex]]) != recentCLK.end())
                        usefulIndexes.erase(usefulIndexes.begin() + inIndex);
                    else
                        break;
                    if(usefulIndexes.size() == 0){  // all the inputs are used: delete recent Statement
                        vector<Statement>* eConsists = allESensitivity[i]->getTargetSubStatement("*EConsistSensitiveName")[0]->getSubStatement();
                        if(j == 0){ //put the second condition to first
                            Tool::setCodeinItem(recentRightNames[0], (*(*eConsists)[0].getSubStatement())[3].getRealCode());
                            eConsists->erase(eConsists->begin());
                            recentCLK.erase(recentCLK.begin());
                        }
                        else{   // take *consist out and find it to erase
                            for(int k = eConsists->size() - 1; k >= 0; k--){
                                if((*eConsists)[k].getTargetSubStatement("RightName")[0]->getRealCode() == recentRightNames[j]->getRealCode()){
                                    eConsists->erase(eConsists->begin() + k);
                                    recentCLK.erase(recentCLK.begin() + k);
                                    break;
                                }
                            }
                        }
                        ifNeedReset = false;
                        break;
                    }
                }
                if(ifNeedReset){
                    string setName = sensitiveInput[usefulIndexes[inIndex]];
                    Tool::setCodeinItem(recentRightNames[j], setName);
                    recentFullRName = setName;
                    recentRightName = setName;
                    recentCLK[j] = setName;
                }
            }
            //multiple bit vector random choose one bit as active bit
            for(Attribute attr : mParameters){
                int end = attr.getEnd();
                int start = attr.getStart();
                int recentIndex = recentFullRName.find("[") == string::npos ? start : Tool::getNumFromClose(recentFullRName);
                bool needAdjust = false;
                bool nameMatch = attr.getName() == recentRightName;
                if(nameMatch && (recentIndex < start || recentIndex > end)){
                    needAdjust = true;
                }
                else if(nameMatch && end > start && recentFullRName.find("[") == string::npos){
                    needAdjust = true;
                }
                else if(nameMatch)
                    break;
                if(needAdjust){
                    int clkIndex = Tool::getRandomfromClosed(start, end);
                    string bracketIndex = "[" + to_string(clkIndex) + "]";
                    Tool::setCodeinItem(recentRightNames[j], recentRightName + bracketIndex);
                    break;
                }
            }
        }
    }

    // combine begin
    // adjust 'if' statement structure
    for(auto it = mAction->end() - 1; it >= mAction->begin(); it--){
        if (it->isContain("Always", "", "FIRST_LAYER")) {
            Statement* always = it->getTargetSubStatement("Always", "FIRST_LAYER")[0];
            Statement* alwaysAction;
            if (always->getExpIndex() == 0) {
                alwaysAction = always->getTargetSubStatement("*Alwaysaction", "FIRST_LAYER")[0];
            }
            else if (always->getExpIndex() == 1) {
                alwaysAction = always->getTargetSubStatement("*EAlwaysaction", "FIRST_LAYER")[0];
            }
            // adjust 'if' statement structure
            if (adjustIFStatementInAlways(alwaysAction)) {
                mainStatement.codeReset();
                vector<int> resultStates = {2, 1};
                return resultStates;
            }
        }
    }

    mainStatement.codeReset();
    if (isPinSafe == 0) {
        vector<int> resultStates = {2, 1};
        return resultStates;
    }

    vector<Statement*> allalways = mainStatement.getTargetSubStatement(allStarAlwaysactionTypes, "ALL_LAYER");
    for(int i = 0; i < allalways.size(); i++){
        vector<string> outvar;
        add_branch(allalways[i], outvar);
    }

    mainStatement.codeReset();
    int recentAllLines = Tool::split(mainStatement.getRealCode(), '\n').size();
    if(recentAllLines < stateIter){
        vector<int> resultStates = {0, 0};
        return resultStates;
    }

    //combine end
    if(ifCheck){
        int ifCount = 0;
        for (auto it = mAction->begin(); it < mAction->end(); it++) {
            if (it->isContain("Always", "", "FIRST_LAYER")) { 
                vector<Statement>* alwaysActions = nullptr;
                Statement* alwaysState = it->getTargetSubStatement("Always", "FIRST_LAYER")[0];
                if (alwaysState->isContain("*Alwaysaction", "", "FIRST_LAYER")) {
                    alwaysActions = alwaysState->getTargetSubStatement("*Alwaysaction", "FIRST_LAYER")[0]->getSubStatement();
                }
                else if(alwaysState->isContain("*EAlwaysaction", "", "FIRST_LAYER")){
                    alwaysActions = alwaysState->getTargetSubStatement("*EAlwaysaction", "FIRST_LAYER")[0]->getSubStatement();
                }

                for (auto eachAlways = alwaysActions->begin(); eachAlways < alwaysActions->end(); eachAlways++) {
                    if (eachAlways->getGrammar() != "If" && eachAlways->getGrammar() != "EIf")
                        continue;
                    mainStatement.codeReset();
                    summaryCode = mainStatement.getRealCode();
                    
                    Statement* outerIfState = &(*eachAlways);
                    Statement* ifState = outerIfState->getTargetSubStatement(allIfTypes, "FIRST_LAYER")[0];
                    vector<Statement>* ifactions = ifState->getSubStatement(); // if/else if/else

                    stack<pair<Statement*, int>> allIfStates; //<pointer, if level
                    map<int, int> stateMap; //Used to note the condition states of recent if level. '0' means need to check; '1' means this statement needn't to check; '2' means the level needn't to check
                    for(int i = ifactions->size() - 1; i >= 0; i--){
                        allIfStates.push(pair<Statement*, int>(&(*ifactions)[i], 0));
                    }
                    stateMap[0] = 0;

                    while (!allIfStates.empty()) {
                        Statement* ifaction = allIfStates.top().first;
                        int level = allIfStates.top().second;
                        allIfStates.pop();
                        if (ifaction->getGrammarType() == "IfAction" || ifaction->getGrammarType() == "ElseIfAction" 
                            || ifaction->getGrammarType() == "EIfAction" || ifaction->getGrammarType() == "EElseIfAction") {
                            Statement* cond = ifaction->getTargetSubStatement("Condition", "FIRST_LAYER")[0];
                            bool stateNeedCheck = true;
                            // state need check set
                            if(ifaction->getGrammarType() == "IfAction" || ifaction->getGrammarType() == "EIfAction"){
                                if(stateMap.count(level) == 0 || stateMap[level] != 2){ //situation: new if
                                    stateMap[level] = 0;
                                    stateMap[level + 1] = 0;
                                }
                                else{
                                    stateNeedCheck = false; //situation: pre-level if is absolute
                                    stateMap[level + 1] = 2;
                                }

                            }
                            else if(stateMap[level] == 1){  //situation: else if branch, and previous if is absolute
                                stateNeedCheck = false;
                                stateMap[level + 1] = 2;
                            }
                            
                            if(stateNeedCheck){
                                // get vars in condition
                                vector<Statement*> tempvarName = cond->getTargetSubStatement("RightName", "ALL_LAYER");
                                set<string> allvarName;
                                for (auto eachvarName = tempvarName.begin(); eachvarName < tempvarName.end(); eachvarName++) {
                                    allvarName.insert(getTrueName((*eachvarName)->getRealCode()));
                                }
                                vector<Attribute> allvar;
                                for (auto var = allvarName.begin(); var != allvarName.end(); var++) {
                                    for (auto attr = allParameter.begin(); attr < allParameter.end(); attr++) {
                                        if ((*attr)->getName() == *var) {
                                            allvar.push_back(**attr);
                                        }
                                    }
                                }

                                int loopTimes = 0;
                                while(true){
                                    int isAbs = Tool::isConditionAbsolute(summaryCode, ifCount);
                                    int acceptAbsTruePercent = 1;   //10%
                                    if(loopTimes >= 10)
                                        acceptAbsTruePercent = 10;  //100%
                                    
                                    if (isAbs == 1) {
                                        int rani = Tool::getRandomfromClosed(1, 10);
                                        if (rani < 10 - acceptAbsTruePercent) {
                                            // rewrite
                                            *cond = getnewCondition(allvar, generateType);
                                            isPinSafe = 0;
                                        }
                                        else{
                                            stateMap[level] = 1;
                                            stateMap[level + 1] = 2;
                                            break;
                                        }
                                    }
                                    else if (isAbs == -1) {
                                        // rewrite
                                        *cond = getnewCondition(allvar, generateType);
                                        isPinSafe = 0;
                                    }
                                    else{   //isAbs == 0: break
                                        break;
                                    }
                                    loopTimes++;
                                    mainStatement.codeReset();
                                    summaryCode = mainStatement.getRealCode();
                                }
                            }
                            ifCount++;
                            // get inner ifactions
                            vector<Statement>* innerifactions = ifaction->getTargetSubStatement(allStarIfactionTypes, "FIRST_LAYER")[0]->getSubStatement();
                            for (int j = innerifactions->size() - 1; j >= 0; j--) {
                                Statement* eachaction = &(*innerifactions)[j];
                                if (eachaction->getGrammar() == "If" || eachaction->getGrammar() == "EIf") {
                                    Statement* recentIfState = eachaction->getTargetSubStatement(allIfTypes, "FIRST_LAYER")[0];
                                    for(int k = recentIfState->getSubStatement()->size() - 1; k >= 0; k--){
                                        allIfStates.push(pair<Statement*, int>(&(*recentIfState->getSubStatement())[k], level + 1));
                                    }
                                }
                            }
                        }
                        else if (ifaction->getGrammarType() == "*ElseIfAction" || ifaction->getGrammarType() == "*EElseIfAction") {
                            vector<Statement>* elifactions = ifaction->getSubStatement();
                            for (int i = elifactions->size() - 1; i >= 0; i--) {
                                allIfStates.push(pair<Statement*, int>(&(*elifactions)[i], level));
                            }
                        }
                        else if(ifaction->getGrammarType() == "ElseAction" || ifaction->getGrammarType() == "EElseAction"){    // get inner ifactions
                            vector<Statement>* innerifactions = ifaction->getTargetSubStatement(allStarIfactionTypes, "FIRST_LAYER")[0]->getSubStatement();
                            for (int j = innerifactions->size() - 1; j >= 0; j--) {
                                Statement* eachaction = &(*innerifactions)[j];
                                if (eachaction->getGrammar() == "If" || eachaction->getGrammar() == "EIf") {
                                    Statement* recentIfState = eachaction->getTargetSubStatement(allIfTypes, "FIRST_LAYER")[0];
                                    for(int k = recentIfState->getSubStatement()->size() - 1; k >= 0; k--){
                                        allIfStates.push(pair<Statement*, int>(&(*recentIfState->getSubStatement())[k], level + 1));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (isPinSafe == 0) {
        mainStatement.codeReset();
        vector<int> resultStates = {2, 0};
        return resultStates;
    }

    //get all the clk
    string clkStr = "^";
    for(Statement* es : allESensitivity){
        vector<Statement*> recentRClks = es->getTargetSubStatement("RightName", "ALL_LAYER");
        for(Statement* rs : recentRClks){
            string recentRightName = rs->getRealCode();
            if(clkStr.find("^" + recentRightName + "^") == string::npos)
                clkStr += recentRightName + "^";
        }
    }
    if(clkStr != "^"){
        clkStr = clkStr.substr(1, clkStr.size() - 2);
        clks = Tool::split(clkStr, '^');
    }

    mainStatement.codeReset();
    summaryCode = mainStatement.getRealCode();
    vector<int> resultStates = {0, 0};
    return resultStates;
}

void Builder::checkNum(Statement* NowStatement){
    vector<Statement*> NumStatements = NowStatement->getTargetSubStatement("Num", "ALL_LAYER");

    for(auto it = NumStatements.begin(); it < NumStatements.end(); it++){
        bool resetflag = false;
        Statement* NumStt = (*it);
        string Numstr = NumStt->getRealCode();
        int max_num = pow(2, 10);
        if(Numstr.size() > 4){ // make sure 10 bit wide
            resetflag = true;
            Numstr = Numstr.substr(0, 4);
        }
        long int Numint = stol(Numstr);
        if(Numint >= max_num){ // make sure not bigger than INT_MAX
            resetflag = true;
            Numint = Numint % max_num;
            Numstr = to_string(Numint);
        }
        if(resetflag){
            Tool::setCodeinItem(NumStt, Numstr);
        }
    }
}

void Builder::resetIsconst(){
    for(int i = 0; i < inputStartIndex; i++)
        mParameters[i].setIsconst(-1);    
    for(int i = inputStartIndex; i < mParameters.size(); i++)
        mParameters[i].setIsconst(0);
    for(int i = 0; i < innerParameters.size(); i++){
        innerParameters[i].setIsconst(-1);
    }
}

int Builder::checkSimulate(){
    if(Tool::fileWrite(programName + ".v", summaryCode)){
        Tool::error("Error: File write wrong.");
        return 1;
    }
    // return 0;
    return Executer::generateTbandVerify(programName, mParameters);
}

// To be perfect 1
string Builder::GetNewInput(){
    string leftName;
    string NewName = getLeftName();
    return NewName;
}

string Builder::getTrueName(string name) {
    int closeIndex = name.find("[");
    if(closeIndex != string::npos)
        name = name.substr(0, closeIndex);
    return name;
}

void Builder::ReplaceRightname(string name, Statement* replaced) {
    string oldname = getTrueName(replaced->getRealCode());
    Attribute* oldrightname = findAttribute(oldname, 2);
    if(oldrightname == nullptr){
        oldrightname = findAttribute(oldname, 2);
    }
    int oldStart = oldrightname->getStart();
    int oldEnd = oldrightname->getEnd();
    if(oldname.find("[") != string::npos){
        int index = oldname.find("[");
        int colon = oldname.find(":");
        if(colon != string::npos){
            oldEnd = atoi(oldname.substr(index + 1, colon - index - 1).c_str());
            oldStart = atoi(oldname.substr(colon + 1, oldname.size() - 1 - colon - 1).c_str());
        }
        else{
            oldStart = atoi(oldname.substr(index + 1, oldname.find("]") - index - 1).c_str());
            oldEnd = oldStart;
        }
    }
    replaceInput(name, replaced, oldEnd - oldStart + 1);
}

void Builder::ReplaceNumber(string name, Statement* replaced) {
    long int replacedNum = stol(replaced->getRealCode());
    int Width = Numbitwidth(replacedNum);
    replaced->setGrammar("RightName");
    replaceInput(name, replaced, Width);
}

void Builder::replaceInput(string name, Statement* replaced, int Width){
    Attribute* newInput = findAttribute(name, 0);
    int newEnd = newInput->getEnd();
    int newStart = newInput->getStart();

    if(Width == 1){
        if(newEnd == 0){
            Tool::setCodeinItem(replaced, name);
        }
        else{
            string additionScope = "[" + to_string(newStart) + "]";
            Tool::setCodeinItem(replaced, name + additionScope);
        }

    }
    else{
        if((newEnd - newStart + 1) > Width){
            string additionScope = "[" + to_string(newInput->getStart() + (Width - 1)) + ":" + to_string(newInput->getStart()) + "]";
            Tool::setCodeinItem(replaced, name + additionScope);
        }
        else if((newEnd - newStart + 1) == Width){
            Tool::setCodeinItem(replaced, name);
        }
        else{
            Statement* inputDefine = getTargetDefine(name);
            newInput->setEnd(Width - 1);
            newInput->setStart(0);
            string additionScope = "[" + to_string(Width - 1) + ":0]";
            Tool::setCodeinItem(replaced, name);

            if (!inputDefine->isContain("InitialScope")) {
                vector<string> tempvec;
                tempvec.push_back(name);
                *inputDefine = generateDefine("input", tempvec, 0);
            }
            Statement* DefineScope = inputDefine->getTargetSubStatement("InitialScope")[0];
            if(DefineScope->isContain("PositiveDoubleNumber")){
                if(Width > 10){
                    Statement* doublestate = DefineScope->getTargetSubStatement("PositiveDoubleNumber")[0];
                    Tool::setCodeinItem(doublestate, to_string(Width - 1));
                }
                else{
                    *DefineScope = generateFromItem("InitialScope", generateType, 1);
                    Statement* doublestate = DefineScope->getTargetSubStatement("PositiveNumber")[0];
                    Tool::setCodeinItem(doublestate, to_string(Width - 1));
                }
            }
            else{
                if(Width > 10){
                    *DefineScope = generateFromItem("InitialScope", generateType, 0);
                    Statement* digitstate = DefineScope->getTargetSubStatement("PositiveDoubleNumber")[0];
                    Tool::setCodeinItem(digitstate, to_string(Width - 1));
                }
                else{
                    Statement* digitstate = DefineScope->getTargetSubStatement("PositiveNumber")[0];
                    Tool::setCodeinItem(digitstate, to_string(Width - 1));
                }
            }
        }
    }
}

int Builder::Numbitwidth(int x){
    int width = 0;
    do {
        x /= 2;
        width++;
    } while(x > 0 && width < varMaxw);
    return width;
}

Statement* Builder::getTargetDefine(string name) {
    vector<Statement*> mDV = mainStatement.getTargetSubStatement("*Define", "FIRST_LAYER")[0]->getTargetSubStatement("Define");
    int i = 0;
    for(i = 0; i < mDV.size(); i++){
        Statement* recentDefine = mDV[i];
        string recentName = recentDefine->getTargetSubStatement("Name")[0]->getRealCode();
        if(recentName == name){
            return recentDefine;
        }
    }
    Tool::error("Can't find target Define Statement");
    exit(1);
}

void Builder::deleteParameter(string name, bool isIO) {
    findanddeleteParameterinsensitivitylist(name);
    if (isIO) {
        for (auto it = mParameters.end() - 1; it >= mParameters.begin(); it--) {
            if (it->getName() == name) {
                mParameters.erase(it);
                break;
            }
        }
    }
    else {
        for (auto it = innerParameters.end() - 1; it >= innerParameters.begin(); it--) {
            if (it->getName() == name) {
                innerParameters.erase(it);
                break;
            }
        }
    }
}

void Builder::findanddeleteParameterinsensitivitylist(string name){
    vector<string> allSensitivityListTypes = {"SensitivityList", "ESensitivityList"};
    vector<Statement*> Sensitivitylists = mainStatement.getTargetSubStatement(allSensitivityListTypes, "ALL_LAYER");
    vector<Attribute> chooseInputs;

    for(Attribute attr : mParameters)
        if(attr.getType() == "input")
            chooseInputs.push_back(attr);
    for(int i = 0; i < Sensitivitylists.size(); i++){
        if(Sensitivitylists[i]->getRealCode() == "*")
            continue;

        string recentSensityType = Sensitivitylists[i]->getGrammarType();
        int consistIndex = 1;
        int firstReviseIndex = 0;
        if(recentSensityType == "ESensitivityList"){
            consistIndex = 3;
            firstReviseIndex = 2;
        }

        vector<Statement>* sub = Sensitivitylists[i]->getSubStatement();
        vector<Statement*> RightNamesS = Sensitivitylists[i]->getTargetSubStatement("RightName", "ALL_LAYER");

        for(int j = RightNamesS.size() - 1; j >= 0; j--){
            string curName = getTrueName(RightNamesS[j]->getRealCode());
            if(curName == name || curName.substr(0, curName.find("[")) == name){
                vector<Statement>* subConsistNames = (*sub)[consistIndex].getSubStatement();
                if(j == 0){
                    if(subConsistNames->size() != 0){
                        Tool::setCodeinItem(&((*sub)[firstReviseIndex]), RightNamesS[j + 1]->getRealCode());
                        subConsistNames->erase(subConsistNames->begin());
                    }
                    else if(recentSensityType == "ESensitivityList"){   //random choose a input as clk
                        int inputIndex = Tool::getRandomfromClosed(0, chooseInputs.size() - 1);
                        Tool::setCodeinItem(RightNamesS[j], chooseInputs[inputIndex].getName());
                    }
                    else{
                        Sensitivitylists[i]->setGrammar("*");
                        Tool::setCodeinItem(Sensitivitylists[i], "*");
                    }
                }
                else{
                    subConsistNames->erase(subConsistNames->begin() + j - 1);
                }
            }
        }
    }
}

void Builder::deleteDefineStatements(string name) {
    vector<Statement>* mDefine = mainStatement.getTargetSubStatement("*Define")[0]->getSubStatement(); // all the define statements
    for (auto it = mDefine->end() - 1; it >= mDefine->begin(); it--) {
        if (it->getTargetSubStatement("Name")[0]->getRealCode() == name) {
            mDefine->erase(it);
        }
    }
}


/*
    Analyze and generate the code of a grammar item
*/
Statement Builder::generateFromItem(string grammarItem, string generateType, int grammarIndex, bool ifOnlyStructure) {
    int isMultiple = 0;
    string multipleStringChar = "";
    string result = "";
    vector<int> varFlags;
    vector<string> options;
    vector<Statement> proSubStatements;
    int length = grammarItem.size();
    for(int i = 0; i < length; i++) {
        if(grammarItem[i] == '*'){          //Note the multiple situation.
            isMultiple = 1;
            continue;
        }
        else if(grammarItem[i] == '\''){         //if its a string, take it into the result.
            while(i < length && grammarItem[++i] != '\'')
                result += grammarItem[i];
            i++;
            if(i < length && grammarItem[i] == '-'){    //if we meet scope, save the string.
                result += '-';
                i++;
                while(i < length && grammarItem[++i] != '\'')
                    result += grammarItem[i];
            }
            else
                i--;
            varFlags.push_back(0);
        }
        else if(grammarItem[i] == '|'){    //if we meet '|', push the result to the options.
            result = "";
            continue;
        }
        else if(grammarItem[i] >= 'A' && grammarItem[i] <= 'Z'){
            string tempResult = "";
            while(i < length && Tool::isWord(grammarItem[i])){
                tempResult += grammarItem[i];
                i++;
            }
            i--;
            result += tempResult;
            varFlags.push_back(1);
        }
        else{
            Tool::error("Grammar item wrong.");
        }
        options.push_back(result);
        result = "";
    }
    int times = 0;
    if(isMultiple == 1){
        while(Tool::getRandomfromClosed(0, 1))
            times++;
    }
    else{
        times = 1;
    }
    int opNum = options.size();
    if(opNum == 0){
        Tool::error("Grammar item wrong.");
    }
    else{
        result = "";
        while(times > 0){
            //Choose Element
            if(opNum == 1 && varFlags[0] != 1){
                result += scopeHandle(options[0]);
            }
            else if(opNum == 1 && varFlags[0] == 1){
                if(!isMultiple){
                    if(grammarIndex == -1){
                        int randIndex = Tool::getRandomfromClosed(0, Tool::getNumIndex(options[0], generateType));
                        grammarIndex = randIndex;
                    }
                    string recentGrammar = Tool::findGrammar(options[0], generateType, grammarIndex);
                    vector<string> recentGrammarItems = Tool::extractGrammar(recentGrammar);
                    int grammarLength = recentGrammarItems.size();
                    for(int i = 0; i < grammarLength; i++){
                        Statement s;
                        string currentGrammarItem = recentGrammarItems[i];
                        if (ifOnlyStructure && (currentGrammarItem == "*Alwaysaction" || currentGrammarItem == "*EAlwaysaction" || currentGrammarItem == "*Ifaction" || currentGrammarItem == "*EIfaction")) {
                            int randIndex = Tool::getRandomfromClosed(0, Tool::getNumIndex(currentGrammarItem, generateType));
                            s = Statement(currentGrammarItem, generateType, randIndex, vector<Statement>());
                        }
                        else {
                            s = generateFromItem(currentGrammarItem, generateType, -1, ifOnlyStructure);
                        }
                        proSubStatements.push_back(s);
                    }
                }
                else{
                    if(multipleStringChar.size() == 0)
                        multipleStringChar = grammarItem.substr(1, grammarItem.size() - 1);
                    Statement s = generateFromItem(multipleStringChar, generateType, -1, ifOnlyStructure);
                    proSubStatements.push_back(s);
                }
            }
            else{
                int index = Tool::getRandomfromClosed(0, opNum - 1);
                string chooseItem = options[index];
                string randChar = "";
                if(varFlags[index] == 0)
                    randChar = scopeHandle(chooseItem);
                result += randChar;
            }
            times--;
        }
    }
    if(proSubStatements.size() == 0)
        return Statement(grammarItem, generateType, result);
    else
        return Statement(grammarItem, generateType, grammarIndex, proSubStatements);
}

string Builder::scopeHandle(string grammarItem){
    //Scope handling
    if(grammarItem.find("-") != string::npos && grammarItem.size() == 3){
        char start = grammarItem[0];
        char end = grammarItem[2];
        int charScope = end - start;
        int randIndex = Tool::getRandomfromClosed(0, charScope - 1);
        char chooseChar = start + randIndex;
        string result = "";
        result += chooseChar;
        return result;
    }
    return grammarItem;
}


Statement* Builder::getStatement(){
    return &mainStatement;
}

Statement Builder::generateDefine(string physicalValue, vector<string> names, int isPure){
    vector<Statement> vs;
    vs.push_back(Statement("'\n'", generateType, "\n"));
    vs.push_back(Statement("PhysicalDataType", generateType, physicalValue));
    vs.push_back(Statement(" ", generateType, " "));
    vector<Statement> mCV;
    //Randomly decide if use InitialScope
    if(isPure == -1){
        isPure = Tool::getRandomfromClosed(0, 1);
    }
    if(!isPure){
        Statement initialS = generateFromItem("InitialScope", generateType);
        while(initialS.getRealCode() == "[0:0]")
            initialS = generateFromItem("InitialScope", generateType);
        vs.push_back(initialS);
        vs.push_back(Statement(" ", generateType, " "));
    }
    for(int i = 0; i < names.size(); i++){
        if(i == 0){
            vs.push_back(Statement("Name", generateType, names[i]));
        }
        else{
            vector<Statement> cV;
            cV.push_back(Statement("', '", generateType, ", "));
            cV.push_back(Statement("Name", generateType, names[i]));
            mCV.push_back(Statement("ConsistName", generateType, 0, cV));  
        }
        if(i == (names.size() - 1))
            vs.push_back(Statement("*ConsistName", generateType, 0, mCV));
    }
    vs.push_back(Statement("';'", generateType, ";"));
    Statement result("Define", generateType, isPure, vs);
    return result;
}

vector<Attribute> Builder::getAvaliVar(Statement* currS){
    vector<Attribute> result;
    bool flag = 0;

    // initialized innerparameters and output
    vector<Statement*> initialized = (*mainStatement.getSubStatement())[8].getTargetSubStatement_DFS_beforeS("LeftName", &flag, currS);
    for(int i = 0; i < initialized.size(); i++){
        string name = initialized[i]->getRealCode();
        Attribute* var = findAttribute(name, 2);
        int notexist = 1;
        for(int j = 0; j < result.size(); j++){
            if(result[j].getName() == name){
                notexist = 0;
                break;
            }
        }
        if(notexist)
            result.push_back((*var));
    }

    // input
    for(int i = inputStartIndex; i < mParameters.size(); i++){
        result.push_back(mParameters[i]);
    }
    return result;
}

Attribute Builder::findDefineInfo(string name, Statement* mDefine){
    vector<Statement> sub = *mDefine->getSubStatement();
    int end = 0;
    string type = "";
    string dataType = "wire";
    for(auto it = sub.begin(); it < sub.end(); it++){
        string recentDefineName = it->getTargetSubStatement("Name")[0]->getRealCode();
        if(recentDefineName == name){
            if(type == "")//Get type
                type = it->getTargetSubStatement("PhysicalDataType")[0]->getRealCode();
            else{ //Get DataType
                dataType = it->getTargetSubStatement("PhysicalDataType")[0]->getRealCode();
                break;
            }
            //Get end
            vector<Statement*> initialScope = it->getTargetSubStatement("InitialScope");
            if(initialScope.size() >= 1 && initialScope[0]->getExpIndex())
                end = atoi(it->getTargetSubStatement("PositiveNumber", "ALL_LAYER")[0]->getRealCode().c_str());
            else if(initialScope.size() >= 1 && !initialScope[0]->getExpIndex())
                end = atoi(it->getTargetSubStatement("PositiveDoubleNumber", "ALL_LAYER")[0]->getRealCode().c_str());
            else
                end = 0;
        }
    }
    Attribute newAssignA(name, type, end, 0);
    return newAssignA;
}

string Builder::fullPinSet(int length){
    string result = "";
    while(length > 0){
        result += "1";
        length--;
    }
    return result;
}

Attribute* Builder::findAttribute(string name, int type){
    // type = 0; find Attribute from mparameters
    // type = 1; find Attribute from innerparameters
    // type = 2; find Attribute from mparameters and innerparameters
    Attribute* target = nullptr;
    if(type != 1){
        for(int i = 0; i < mParameters.size(); i++){
            if(mParameters[i].getName() == name){
                target = &mParameters[i];
            }
        }
    }
    if(type != 0){
        for(int i = 0; i < innerParameters.size(); i++){
            if(innerParameters[i].getName() == name){
                target = &innerParameters[i];
            }
        }
    }
    if(target == nullptr)
        Tool::error("Can't find this Attribute.\n");
    return target;
}

vector<pair<int, int>> Builder::getComplementSet(vector<pair<int, int>> nullSet, int end){
    vector<pair<int, int>> result;
    int preTail = -1;
    for(int i = 0; i < nullSet.size(); i++){
        pair<int, int> recentPair = nullSet[i];
        if(i == 0 && recentPair.first != 0)
            result.push_back(pair<int, int>(0, recentPair.first - 1));
        else{
            if(preTail != -1)
                result.push_back(pair<int, int>(preTail + 1, recentPair.first - 1));
        }
        preTail = recentPair.second;
        if(i == nullSet.size() - 1 && preTail != end)
            result.push_back(pair<int, int>(preTail + 1, end));
    }
    return result;
}

int Builder::getPairVectorValueLen(vector<pair<int, int>> v){
    int result = 0;
    for(int i = 0; i < v.size(); i++){
        int start = v[i].first;
        int end = v[i].second;
        result += end - start + 1;
    }
    return result;
}

int Builder::getArrow(int originNum, vector<pair<int, int>> set){
    int arrow = 0;
    int preTail = -1;
    for(auto it = set.begin(); it < set.end(); it++){
        int recentStart = it->first;
        int recentEnd = it->second;
        arrow += recentStart - preTail - 1 ;
        if(originNum >= recentStart && originNum <= recentEnd)
            return arrow;
        preTail = recentEnd;
    }
    return -1;
}

vector<pair<int, int>> Builder::getScopeSet(string pinOcuppy){
    vector<pair<int, int>> scopeSet;
    int pre = -1;
    char consistChar = '0';
    for(int j = 0; j < pinOcuppy.size(); j++){
        if(pinOcuppy[j] == consistChar){
            if(pre == -1){
                pre = j;
                consistChar = '1';
            }
            else{
                pair<int, int> newScope(pre, j - 1);
                scopeSet.push_back(newScope);
                pre = -1;
                consistChar = '0';
            }
        }
    }
    if(pre != -1){
        pair<int, int> newScope(pre, pinOcuppy.size() - 1);
        scopeSet.push_back(newScope);
    }
    return scopeSet;
}

int Builder::getUsedPinNumber(string pinOcuppy){
    int result = 0;
    for (int i = 0; i < pinOcuppy.size(); i++) {
        if (pinOcuppy[i] == '1') {
            result++;
        }
    }
    return result;
}

bool Builder::deleteStatementFromAlways(Statement* alwaysStatement, string realCode, string deleteGrammarType) {
    vector<string> allActionTypes = {"*Alwaysaction", "*EAlwaysaction"};
    Statement* mAlways = alwaysStatement->getTargetSubStatement(allActionTypes)[0];
    stack<Statement*> tempStatement;
    tempStatement.push(mAlways);
    bool isProcess = false;
    
    while (tempStatement.size() != 0){
        Statement* recentState = tempStatement.top();
        tempStatement.pop();
        vector<Statement>* recentSubStates = recentState->getSubStatement();
        for (auto it = recentSubStates->end() - 1; it >= recentSubStates->begin(); it--) {
            Statement ss = *it;
            vector<Statement*> targetSubStatement = it->getTargetSubStatement(deleteGrammarType, "FIRST_LAYER");
            if (!targetSubStatement.empty()) {
                string stateRealcode = targetSubStatement[0]->getRealCode();
                if (stateRealcode == realCode) {
                    recentSubStates->erase(it);
                    isProcess = true;
                }
            }
            else {
                // if there are substatements in 'it', push 'it' into the stack(tempStatement)
                if (it->isContain(deleteGrammarType, realCode, "ALL_LAYER")) {
                    tempStatement.push(&(*it));
                }
            }
        }
    }
    return isProcess;
}

bool Builder::deleteEmptySubStatementsInAlways(Statement* alwaysActionStatement) {
    vector<Statement>* subStates = alwaysActionStatement->getSubStatement();
    
    vector<string> allIfTypes = {"If", "EIf"};
    vector<string> allIfactionTypes = {"*Ifaction", "*EIfaction"};

    bool isProcess = false;
    for (auto it = subStates->end() - 1; it >= subStates->begin(); it--) {
        // 2023/12/20 syh begin
        if ((it->getTargetSubStatement(allIfTypes, "FIRST_LAYER")).size() != 0) {
            // 'it' contains an If Statement
            Statement* stateOfIf = it->getTargetSubStatement(allIfTypes, "FIRST_LAYER")[0];
            vector<Statement>* ifBranches = stateOfIf->getSubStatement();
            for (auto sub = ifBranches->end() - 1; sub >= ifBranches->begin(); sub--) {
                if (sub->getGrammarType() == "*ElseIfAction" || sub->getGrammarType() == "*EElseIfAction") { // else if Statements
                    vector<Statement>* elseIfBranches = sub->getSubStatement();
                    if (elseIfBranches->size() == 0) {
                        ifBranches->erase(sub);
                        isProcess = true;
                        continue;
                    }
                    for (auto subElif = elseIfBranches->end() - 1; subElif >= elseIfBranches->begin(); subElif--) {
                        Statement* ifAction = subElif->getTargetSubStatement(allIfactionTypes, "FIRST_LAYER")[0];
                        if (ifAction->isContain("If") || ifAction->isContain("EIf")) {
                            if (deleteEmptySubStatementsInAlways(ifAction)) {
                                isProcess = true;
                            }
                        }
                        vector<Statement>* recentSubStates = ifAction->getSubStatement();
                        if (recentSubStates->size() == 0) {
                            elseIfBranches->erase(subElif);
                            isProcess = true;
                        }
                    }
                    if (sub->getSubStatement()->size() == 0) { // no else-if left in *ElseIfAction
                        ifBranches->erase(sub);
                        isProcess = true;
                    }
                }
                else { // if, else Statement
                    Statement* ifAction = sub->getTargetSubStatement(allIfactionTypes, "FIRST_LAYER")[0];
                    if (ifAction->isContain("If") || ifAction->isContain("EIf")) {
                        if (deleteEmptySubStatementsInAlways(ifAction)) {
                            isProcess = true;
                        }
                    }
                    vector<Statement>* recentSubStates = ifAction->getSubStatement();
                    if (recentSubStates->size() == 0) {
                        ifBranches->erase(sub);
                        isProcess = true;
                    }
                }
            }
            if (it->getTargetSubStatement(allIfactionTypes, "ALL_LAYER").size() == 0) {
                subStates->erase(it);
                isProcess = true;
            }
        }
    }
    return isProcess;
}

bool Builder::adjustIFStatementInAlways(Statement* alwaysActionStatement) {
    // the grammar type of alwaysActionStatement is '*Alwaysaction'
    vector<Statement>* alwaysStates = alwaysActionStatement->getSubStatement();
    bool isProcess = false;

    vector<string> allIfTypes = {"If", "EIf"};

    for (int i = alwaysStates->size() - 1; i >= 0; i--) {
        if ((*alwaysStates)[i].getGrammar() != "If" && (*alwaysStates)[i].getGrammar() != "EIf") {
            continue;
        }
        Statement* ifAction = (*alwaysStates)[i].getTargetSubStatement(allIfTypes, "FIRST_LAYER")[0];

        vector<Statement>* subStates = ifAction->getSubStatement();

        Statement firstSub = (*subStates)[0];
        // process each inner if statement
        for (auto eachIF = subStates->begin(); eachIF < subStates->end(); eachIF++) {
            if (adjustIFStatementInIF(&(*eachIF))) {
                isProcess =  true;
            }
        }
        if (firstSub.getGrammarType() == "ElseIfAction") {
            vector<Statement>* subGrammar = (*subStates)[0].getSubStatement();
            (*subStates)[0].setGrammar("IfAction");
            (*subGrammar)[0].setRealCode("\nif (");
            (*subGrammar)[0].codeReset();
        }
        else if(firstSub.getGrammarType() == "EElseIfAction"){
            vector<Statement>* subGrammar = (*subStates)[0].getSubStatement();
            (*subStates)[0].setGrammar("EIfAction");
            (*subGrammar)[0].setRealCode("\nif (");
            (*subGrammar)[0].codeReset();
        }
        else if (firstSub.getGrammarType() == "*ElseIfAction" || firstSub.getGrammarType() == "*EElseIfAction") {
            string elseIfAction = "ElseIfAction";
            string ifAction = "IfAction";
            if(firstSub.getGrammarType() == "*EElseIfAction"){
                elseIfAction = "EElseIfAction";
                ifAction = "EIfAction";
            }
            Statement* firstElif = (*subStates)[0].getTargetSubStatement(elseIfAction, "FIRST_LAYER")[0];
            vector<Statement>* subGrammar = firstElif->getSubStatement();
            firstElif->setGrammar(ifAction);
            (*subGrammar)[0].setRealCode("\nif (");
            (*subGrammar)[0].codeReset();
            Statement newIfAction = *firstElif;
            vector<Statement>* allSubStates = (*subStates)[0].getSubStatement();
            allSubStates->erase(allSubStates->begin());
            stack<Statement> laterStates;
            for (auto state = subStates->end() - 1; state >= subStates->begin(); state--) {
                if (state->getSubStatement()->size() != 0)
                    laterStates.push(*state);
            }
            subStates->clear();
            subStates->push_back(newIfAction);
            while (!laterStates.empty()) {
                subStates->push_back(laterStates.top());
                laterStates.pop();
            }
        }
        else if (firstSub.getGrammarType() == "ElseAction" || firstSub.getGrammarType() == "EElseAction") {  
            string starIfaction = "*Ifaction";
            string alwaysaction = "Alwaysaction";
            if(firstSub.getGrammarType() == "EElseAction"){
                starIfaction = "*EIfaction";
                alwaysaction = "EAlwaysaction";
            }

            vector<Statement>* innerifAction = firstSub.getTargetSubStatement(starIfaction, "FIRST_LAYER")[0]->getSubStatement();
            // delete this 'else' statement from alwaysAction
            stack<Statement> laterStates;
            for (auto state = alwaysStates->end() - 1; state >= alwaysStates->begin(); state--) {
                if ((*state).getRealCode() == (*alwaysStates)[i].getRealCode()) {
                    alwaysStates->erase(alwaysStates->begin() + i);
                    break;
                }
                else {
                    laterStates.push(*state);
                    alwaysStates->erase(state);
                }
            }
            for (auto eachIFAction = innerifAction->begin(); eachIFAction < innerifAction->end(); eachIFAction++) {
                // move inner statements into alwaysAction
                vector<Statement>* eachStates = eachIFAction->getSubStatement();
                // add should be a Alwaysaction
                for (auto state = eachStates->begin(); state < eachStates->end(); state++) {
                    vector<Statement> sub;
                    sub.push_back(*state);
                    Statement newAlwaysActionState = Statement(alwaysaction, generateType, eachIFAction->getExpIndex(), sub);
                    alwaysStates->push_back(newAlwaysActionState);
                }
            }
            while (!laterStates.empty()) {
                alwaysStates->push_back(laterStates.top());
                laterStates.pop();
            }
            isProcess =  true;
        }
    }
    return isProcess;
}


bool Builder::adjustIFStatementInIF(Statement* IfState) {
    vector<string> allStarIfactionTypes = {"*Ifaction", "*EIfaction"};
    vector<string> allIfTypes = {"If", "EIf"};

    bool isProcess = false;
    // IfState represent a statement of IfAction/ElseIfAction/ElseAction
    if (IfState->getGrammarType() == "*ElseIfAction" || IfState->getGrammarType() == "*EElseIfAction") {
        vector<Statement>* elseifActions = IfState->getSubStatement();
        for (auto ss = elseifActions->end() - 1; ss >= elseifActions->begin(); ss--) { 
            if (adjustIFStatementInIF(&(*ss))) {
                isProcess =  true;
            }
        }
        return isProcess;
    }

    vector<Statement>* ifactions = IfState->getTargetSubStatement(allStarIfactionTypes, "FIRST_LAYER")[0]->getSubStatement();
    vector<Statement> tif = *ifactions;

    for (int i = ifactions->size() - 1; i >= 0; i--) {
        if ((*ifactions)[i].getGrammar() != "If" && (*ifactions)[i].getGrammar() != "EIf") {
            continue;
        }
        Statement* ifAction = (*ifactions)[i].getTargetSubStatement(allIfTypes, "FIRST_LAYER")[0];
        // ifAction is an 'if' statement
        vector<Statement>* subStates = ifAction->getSubStatement();
        // subStates is each brach in 'if': IfAction, ElseIfActon, ElseAction
        Statement firstSub = (*subStates)[0];
        // process each inner if statement
        for (auto eachIF = subStates->begin(); eachIF < subStates->end(); eachIF++) {
            if (adjustIFStatementInIF(&(*eachIF))) {
                isProcess =  true;
            }
        }
        if (firstSub.getGrammarType() == "ElseIfAction") {
            vector<Statement>* subGrammar = (*subStates)[0].getSubStatement();
            (*subStates)[0].setGrammar("IfAction");
            (*subGrammar)[0].setRealCode("\nif (");
            (*subGrammar)[0].codeReset();
        }
        else if (firstSub.getGrammarType() == "EElseIfAction") {
            vector<Statement>* subGrammar = (*subStates)[0].getSubStatement();
            (*subStates)[0].setGrammar("EIfAction");
            (*subGrammar)[0].setRealCode("\nif (");
            (*subGrammar)[0].codeReset();
        }
        else if (firstSub.getGrammarType() == "*ElseIfAction" || firstSub.getGrammarType() == "*EElseIfAction") {
            string IfAction = "IfAction";
            string ElseIfAction = "ElseIfAction";
            if(firstSub.getGrammarType() == "*EElseIfAction"){
                IfAction = "EIfAction";
                ElseIfAction = "EElseIfAction";
            }
            Statement* firstElif = (*subStates)[0].getTargetSubStatement(ElseIfAction, "FIRST_LAYER")[0];
            vector<Statement>* subGrammar = firstElif->getSubStatement();
            firstElif->setGrammar(IfAction);
            (*subGrammar)[0].setRealCode("\nif (");
            (*subGrammar)[0].codeReset();
            Statement newIfAction = *firstElif;
            vector<Statement>* allSubStates = (*subStates)[0].getSubStatement();
            allSubStates->erase(allSubStates->begin());
            stack<Statement> laterStates;
            for (auto state = subStates->end() - 1; state >= subStates->begin(); state--) {
                if (state->getSubStatement()->size() != 0)
                    laterStates.push(*state);
            }
            subStates->clear();
            subStates->push_back(newIfAction);
            while (!laterStates.empty()) {
                subStates->push_back(laterStates.top());
                laterStates.pop();
            }
        }
        else if (firstSub.getGrammarType() == "ElseAction" || firstSub.getGrammarType() == "EElseAction") {       
            vector<string> allStarIfactionTypes = {"*Ifaction", "*EIfaction"};    
            vector<Statement>* ifAction = firstSub.getTargetSubStatement(allStarIfactionTypes, "FIRST_LAYER")[0]->getSubStatement();
            stack<Statement> laterStates;
            for (auto state = ifactions->end() - 1; state >= ifactions->begin(); state--) {
                if ((*state).getRealCode() == (*ifactions)[i].getRealCode()) {
                    ifactions->erase(ifactions->begin() + i);
                    break;
                }
                else {
                    laterStates.push(*state);
                    ifactions->erase(state);
                }
            }
            for (auto innerStates = ifAction->begin(); innerStates < ifAction->end(); innerStates++) {
                // move inner statements out
                ifactions->push_back(*innerStates);
            }
            while (!laterStates.empty()) {
                ifactions->push_back(laterStates.top());
                laterStates.pop();
            }
            isProcess =  true;
        }
    }

    return isProcess;
}

void Builder::checkPinDefineAdj(Attribute param, vector<Statement>* mDefine, int newLen){
    string name = param.getName();
    int handletimes = 0;
    for(int i = 0; i < mDefine->size(); i++){
        //define adjust
        Statement* recentDefine = &(*mDefine)[i];
        if(recentDefine->isContain("Name", name)){
            handletimes++;
            string physical = (recentDefine->getTargetSubStatement("PhysicalDataType", "FIRST_LAYER"))[0]->getRealCode();
            
            if (recentDefine->getExpIndex() == 1) {
                if (newLen == 1) {
                    return;
                }
                else {
                    Tool::error("Error: NewLength is larger than the initial scope.");
                    exit(1);
                }
            }
            Statement* initialScope = (recentDefine->getTargetSubStatement("InitialScope", "ALL_LAYER"))[0];
            Statement* newNum;
            if(newLen == 1){
                vector<string> names;
                names.push_back(name);
                Statement newDefine = generateDefine(physical, names, 1);
                (*mDefine)[i] = newDefine;
            }
            else if(initialScope->getExpIndex() == 0){
                newNum = initialScope->getTargetSubStatement("PositiveDoubleNumber")[0];
                Tool::setCodeinItem(newNum, to_string(newLen - 1));
                if(newLen - 1 < 10){
                    newNum->setGrammar("PositiveNumber");
                    initialScope->setExpIndex(1);
                }
            }
            else{
                newNum = initialScope->getTargetSubStatement("PositiveNumber")[0];
                Tool::setCodeinItem(newNum, to_string(newLen - 1));
            }
            findAttribute(name, 2)->setEnd(newLen - 1);
        }
        if(handletimes == 1)
            handletimes++;
        else if(handletimes > 1)
            break;
    }
}

void Builder::checkPinUsedAdj(string leftName, vector<pair<int, int>> usedPinSet, Statement* mActionState){
    vector<Statement*> revolveS = mActionState->getTargetSubStatement("RightName", "ALL_LAYER");
    for (auto subit = revolveS.begin(); subit < revolveS.end(); subit++){
        Statement* recentRN = *subit;
        string recentName = recentRN->getRealCode();
        string trueName = getTrueName(recentName);
        if(trueName == "K")
            int a = 0;

        if(trueName == recentName)  //No need to adjust
            continue;
        else if (trueName == leftName) {
            int colon = recentName.find(':');
            int closeIndex = recentName.find('[');
            int rightIndex = recentName.find(']');
            if(colon == string::npos){
                int originBit = atoi(recentName.substr(closeIndex + 1, rightIndex - closeIndex - 1).c_str());
                bool contain = false;
                for(pair<int, int> scope : usedPinSet){
                    if(originBit >= scope.first && originBit <= scope.second){
                        contain = true;
                        break;
                    }
                }
                if(!contain){   //combinational logic always sensitivity: directly revise to trueName
                    recentName = leftName;
                }
                else{
                    int reviseBit = originBit - getArrow(originBit, usedPinSet);
                    recentName = leftName + "[" + to_string(reviseBit) + "]";
                    int attriEnd = findAttribute(leftName, 2)->getEnd();
                    if(attriEnd == 0){
                        recentName = leftName;
                    }
                }
            }
            else{
                int originEnd = atoi(recentName.substr(closeIndex + 1, colon - closeIndex - 1).c_str());
                int originStart = atoi(recentName.substr(colon + 1, rightIndex - colon - 1).c_str());
                
                bool contain = false;
                for(pair<int, int> scope : usedPinSet){
                    if(originStart >= scope.first && originEnd <= scope.second){
                        contain = true;
                        break;
                    }
                }

                if(!contain){   //combinational logic always sensitivity: directly revise to trueName
                    recentName = leftName;
                }
                else{
                    int arrow = getArrow(originStart, usedPinSet);
                    int reviseStart = originStart - arrow;
                    int reviseEnd = originEnd - arrow;
                    recentName = leftName + "[" + to_string(reviseEnd) + ":" + to_string(reviseStart) + "]";
                    int attriEnd = findAttribute(leftName, 2)->getEnd();
                    if(attriEnd == reviseEnd && 0 == reviseStart){
                        int randIndex = Tool::getRandomfromClosed(0, 1);
                        if(randIndex == 0)
                            recentName = leftName;
                    }
                }
            }
            Tool::setCodeinItem(recentRN, recentName);
        }
    }
}

vector<Attribute> Builder::getAllMutableVar(Statement* currS){
    vector<Attribute> allMutVar = getAvaliVar(currS);

    for(auto it = allMutVar.end() - 1; it >= allMutVar.begin(); it--){
        if(it->getIsconst())
            allMutVar.erase(it);
    }

    Statement* allaction = findcuralways(currS);
    vector<Statement*> availreg = allaction->getTargetSubStatement("LeftName", "ALL_LAYER");
    for(auto it = allMutVar.end() - 1; it >= allMutVar.begin(); it--){
        if(it->getDataType() == "reg"){
            bool flag = 1;
            for(auto reg = availreg.begin(); reg < availreg.end(); reg++){
                if(it->getName() == (*reg)->getRealCode()){
                    flag = 0;
                    break;
                }
            }
            if(flag)
                allMutVar.erase(it);
        }
    }

    return allMutVar;
}

vector<int> Builder::MutVarMaxWidth(vector<Attribute> mutVar){
    vector<int> Width;
    for(int i = 0; i < mutVar.size(); i++){
        int w = mutVar[i].getEnd() - mutVar[i].getStart() + 1;
        Width.push_back(w);
    }
    return Width;
}

void Builder::replaceSwithA(Statement* statement, Attribute* attribute, int width){
    // random chose a RightName or Num
    vector<Statement*> rnS = statement->getTargetSubStatement("RightName", "ALL_LAYER");
    vector<Statement*> numS = statement->getTargetSubStatement("Num", "ALL_LAYER");
    rnS.insert(rnS.end(), numS.begin(), numS.end());
    int index = Tool::getRandomfromClosed(0, rnS.size() - 1);

    // random a width
    int varW = attribute->getEnd() - attribute->getStart() + 1;
    int genewidth = Tool::getRandomfromClosed(width, varW);

    string replaceRightName = attribute->getName();
    int attrWidth = attribute->getEnd() - attribute->getStart() + 1;
    if(attrWidth < genewidth){
        Tool::error("The width of the variable used for replacement smaller than required");
    }
    else if(attrWidth != genewidth && genewidth != 0){
        int rand = Tool::getRandomfromClosed(0, attrWidth - genewidth);
        if(genewidth == 1)
            replaceRightName += "[" + to_string(rand + attribute->getStart()) + "]";
        else
            replaceRightName += "[" + to_string(rand + attribute->getStart() + genewidth - 1) + ":" + to_string(rand + attribute->getStart()) + "]";
    }
    Tool::setCodeinItem(rnS[index], replaceRightName);
    rnS[index]->setGrammar("RightName");
}

int Builder::RightNamebitwidth(string* recentName){
    int w;
    int closeTax = recentName->find("[");
    if(*recentName == ""){
        Tool::error("Error: Find null RightName");
        return 0;
    }
    else if(closeTax == string::npos){
        Attribute* Att = findAttribute(*recentName, 2);
        w = Att->getEnd() - Att->getStart() + 1;
    }
    else{
        int colon = recentName->find(":");
        int recentStart = 0;
        int recentEnd = 0;
        if(colon != string::npos){
            recentEnd = atoi(recentName->substr(closeTax + 1, colon - closeTax - 1).c_str());
            recentStart = atoi(recentName->substr(colon + 1, recentName->size() - 1 - colon - 1).c_str());
        }
        else{
            recentStart = atoi(recentName->substr(closeTax + 1, recentName->find("]") - closeTax - 1).c_str());
            recentEnd = recentStart;
        }
        *recentName = recentName->substr(0, closeTax);
        w = recentEnd - recentStart + 1;
    }
    return w;
}

vector<Attribute*> Builder::getinitializedreg(Statement* curalways){
    vector<Attribute*> vars;
    for(int i = 0; i < mParameters.size(); i++){
        if(mParameters[i].getDataType() == "reg")
            vars.push_back(&mParameters[i]);
    }
    for(int i = 0; i < innerParameters.size(); i++){
        if(innerParameters[i].getDataType() == "reg")
            vars.push_back(&innerParameters[i]);
    }
    if(vars.size() == 0)
        return vars;

    vector<Statement*> availreg = curalways->getTargetSubStatement("LeftName", "ALL_LAYER");
    for(auto it = vars.end() - 1; it >= vars.begin(); it--){
        if((*it)->getDataType() == "reg"){
            bool flag = 1;
            for(auto reg = availreg.begin(); reg < availreg.end(); reg++){
                if((*it)->getName() == (*reg)->getRealCode()){
                    flag = 0;
                    break;
                }
            }
            if(flag)
                vars.erase(it);
        }
    }
    return vars;
}

int Builder::LeftNametimes(string name){
    vector<Statement*> AllLeftName = mainStatement.getTargetSubStatement("LeftName", "ALL_LAYER");
    int count = 0;
    for(int i = 0; i < AllLeftName.size(); i++){
        if(AllLeftName[i]->getRealCode() == name)
            count++;
    }
    return count;
}

int Builder::deleteStatement_ofthisLeftName(string leftName){
    int deletetimes = 0;
    vector<string> allStarAlwaysactionTypes = {"*Alwaysaction", "*EAlwaysaction"};
    vector<Statement*> allAlways = mainStatement.getTargetSubStatement(allStarAlwaysactionTypes, "ALL_LAYER");
    for(int i = 0; i < allAlways.size(); i++){
        deletetimes += deleteLeftname_in_statement(leftName, allAlways[i]);
    }
    return deletetimes;
}

int Builder::deleteLeftname_in_statement(string leftName, Statement* statement){
    vector<Statement>* actions = statement->getSubStatement();
    vector<string> allStarIfaction = {"*Ifaction", "*EIfaction"};
    int deletetimes = 0;
    for(int n = 0; n < actions->size(); n++){
        Statement* curaction = &(*actions)[n];
        //handle if
        if(curaction->isContain("If") || curaction->isContain("EIf")){
            vector<Statement>* allIf = curaction->getSubStatement();
            allIf = (*allIf)[0].getSubStatement();
            vector<Statement*> allIfactions;
            for(int i = 0; i < allIf->size(); i++){
                if((*allIf)[i].getGrammarType() == "*ElseIfAction" || (*allIf)[i].getGrammarType() == "*EElseIfAction"){
                    vector<Statement>* elseifsub = (*allIf)[i].getSubStatement();
                    for(int j = 0; j < (*elseifsub).size(); j++){
                        Statement* curIfaction = (*elseifsub)[j].getTargetSubStatement(allStarIfaction, "FIRST_LAYER")[0];
                        allIfactions.push_back(curIfaction);
                    }
                }
                else{
                    Statement* curIfaction = (*allIf)[i].getTargetSubStatement(allStarIfaction, "FIRST_LAYER")[0];
                    allIfactions.push_back(curIfaction);
                }
            }
            for(int i = 0; i < allIfactions.size(); i++){
                deletetimes += deleteLeftname_in_statement(leftName, allIfactions[i]);
            }
        }
        else{
            Statement* leftnameS = curaction->getTargetSubStatement("LeftName", "ALL_LAYER")[0];
            Statement b = *curaction;
            if(leftnameS->getRealCode() == leftName){
                deletetimes++;
                actions->erase(actions->begin() + n);
                // info();
                n--;
            }
        }
    }
    return deletetimes;
}

bool Builder::checkisconst(){
    bool iffinish = 1;
    for(int i = 0; i < innerParameters.size(); i++){
        if(innerParameters[i].getIsconst() == -1){
            iffinish = 0;
            break;
        }
    }
    for(int i = 0; i < inputStartIndex && iffinish; i++){
        if(mParameters[i].getIsconst() == -1){
            vector<Statement*> allLN = mainStatement.getTargetSubStatement("LeftName", "ALL_LAYER");
            for(int j = 0; j < allLN.size(); j++){
                if(allLN[j]->getRealCode() == mParameters[i].getName()){
                    iffinish = 0;
                    break;
                }
            }
        }
    }
    return !iffinish;
}

void Builder::findInitialisconst(){
    Statement* mActionState = mainStatement.getTargetSubStatement("*Action")[0];
    vector<Statement>* mAction = mActionState->getSubStatement();
    for(int i = 0; i < mAction->size(); i++){
        Statement* action = &(*mAction)[i];
        if(action->isContain("Assign", "", "FIRST_LAYER")){
            calculateisconst(action);
        }
        else if(action->isContain("Always", "", "FIRST_LAYER")){
            vector<Statement*> alwaysAssign = action->getTargetSubStatement("Block", "ALL_LAYER");
            vector<Statement*> noBlockAssign = action->getTargetSubStatement("NoBlock", "ALL_LAYER");
            alwaysAssign.insert(alwaysAssign.end(), noBlockAssign.begin(), noBlockAssign.end());
            for (auto it = alwaysAssign.begin(); it < alwaysAssign.end(); it++){
                if((*it)->isContain("If") || (*it)->isContain("EIf"))
                    findInitialisconstinif(*it);
                else
                    calculateisconst(*it);
            }
        }
    }
}

void Builder::findInitialisconstinif(Statement* ifS){
    vector<string> allStarIfactionTypes = {"*Ifaction", "*EIfaction"};
    vector<Statement*> Ifactions = ifS->getTargetSubStatement(allStarIfactionTypes, "ALL_LAYER");
    for(auto curraction = Ifactions.begin(); curraction < Ifactions.end(); curraction++){
        vector<Statement>* ifstatements = (*curraction)->getSubStatement();
        for(int i = 0; i < ifstatements->size(); i++){
            Statement* action = &(*ifstatements)[i];
            if(action->isContain("If") || action->isContain("EIf"))
                findInitialisconstinif(action);
            else
                calculateisconst(action);
        }
    }
}

void Builder::calculateisconst(Statement* assignment){
    string LeftName = assignment->getTargetSubStatement("LeftName", "ALL_LAYER")[0]->getRealCode();
    if(findAttribute(LeftName, 2)->getIsconst() != -1) return;
    vector<Statement*> allRightNames = assignment->getTargetSubStatement("RightName", "ALL_LAYER");
    bool isconst = 1;
    for(auto it = allRightNames.begin(); it < allRightNames.end() && isconst != 0; it++){
        string RightName = (*it)->getRealCode();
        RightName = getTrueName(RightName);
        int thisisconst = findAttribute(RightName, 2)->getIsconst();
        if(thisisconst != 1){
            isconst = thisisconst;
        }
    }
    findAttribute(LeftName, 2)->setIsconst(isconst);
}

bool Builder::handleifcondtion_issafe(Statement* statement){
    vector<Statement>* substate = statement->getSubStatement();
    vector<string> allStarIfaction = {"*Ifaction", "*EIfaction"};
    vector<string> allIfTypes = {"If", "EIf"};
    bool issafe = 1;
    for(int i = 0; i < substate->size(); i++){
        Statement* action = &(*substate)[i];
        if(action->isContain("If") || action->isContain("EIf")){
            Statement* Ifaction = action->getTargetSubStatement(allIfTypes, "FIRST_LAYER")[0];
            vector<Statement>* Ifbranches = Ifaction->getSubStatement();
            for(int j = 0; j < Ifbranches->size(); j++){
                int cursafe, subsafe, loopsafe;
                Statement* curbranch = &(*Ifbranches)[j];
                if(curbranch->getGrammarType() == "IfAction" || curbranch->getGrammarType() == "EIfAction"){
                    Statement* next;
                    loopsafe = handlesingleif_issafe_loop(curbranch);
                    cursafe = handlesingleif_issafe(curbranch, &next);
                    subsafe = handleifcondtion_issafe(next);
                    if(issafe){
                        if(!cursafe || !subsafe || !loopsafe)
                            issafe = 0;
                    }
                }
                else if(curbranch->getGrammarType() == "*ElseIfAction" || curbranch->getGrammarType() == "*EElseIfAction"){
                    vector<Statement>* allelseif = curbranch->getSubStatement();
                    for(int k = 0; k < allelseif->size(); k++){
                        Statement* curelseif = &(*allelseif)[k];
                        Statement* next;
                        loopsafe = handlesingleif_issafe_loop(curelseif);
                        cursafe = handlesingleif_issafe(curelseif, &next);
                        subsafe = handleifcondtion_issafe(next);
                        if(issafe){
                            if(!cursafe || !subsafe || !loopsafe)
                                issafe = 0;
                        }
                    }
                }
                else{
                    Statement* next = curbranch->getTargetSubStatement(allStarIfaction, "FIRST_LAYER")[0];
                    subsafe = handleifcondtion_issafe(next);
                    if(issafe){
                        if(!subsafe)
                            issafe = 0;
                    }
                }
            }
        }
    }
    return issafe;
}

bool Builder::handlesingleif_issafe(Statement* statement, Statement** next){
    Statement* conditionS = statement->getTargetSubStatement("Condition", "FIRST_LAYER")[0];
    vector<string> allStarIfaction = {"*Ifaction", "*EIfaction"};
    Statement* nextifaction = statement->getTargetSubStatement(allStarIfaction, "FIRST_LAYER")[0];
    vector<Statement*> allRinC = conditionS->getTargetSubStatement("RightName", "ALL_LAYER");
    bool isconst = 1;

    *next = nextifaction;
    for(auto it = allRinC.begin(); it < allRinC.end() && isconst; it++){
        string name = (*it)->getRealCode();
        name = getTrueName(name);
        if(findAttribute(name, 2)->getIsconst() == 0){
            isconst = 0;
            break;
        }
    }
    if(isconst){
        vector<Statement*> allNinC = conditionS->getTargetSubStatement("Num", "ALL_LAYER");
        int randindex = Tool::getRandomfromClosed(0, allNinC.size() + allRinC.size() - 1);
        Statement* replaced;
        if(randindex < allNinC.size())
            replaced = allNinC[randindex];
        else
            replaced = allRinC[randindex - allNinC.size()];

        string replace;
        vector<Attribute> availvar = getAllMutableVar(conditionS);

        vector<Attribute> initialedvar = getStructfrontAttribute(statement);
        for(int i = 0; i < availvar.size(); i++){
            bool find = 0;
            for(int j = 0; j < initialedvar.size() && !find; j++){
                if(initialedvar[j].getName() == availvar[i].getName())
                    find = 1;
            }
            if(!find){
                availvar.erase(availvar.begin() + i);
                i--;
            }
        }

        int randindex1 = Tool::getRandomfromClosed(0, availvar.size() - 1);
        replace = availvar[randindex1].getName();
        Tool::setCodeinItem(replaced, replace);
        replaced->setGrammar("RightName");
    }
    return !isconst;
}

bool Builder::handlesingleif_issafe_loop(Statement* statement){
    vector<string> allStarIfaction = {"*Ifaction", "*EIfaction"};
    Statement* conditionS = statement->getTargetSubStatement("Condition", "FIRST_LAYER")[0];
    Statement* nextifaction = statement->getTargetSubStatement(allStarIfaction, "FIRST_LAYER")[0];
    vector<Statement*> allRinC = conditionS->getTargetSubStatement("RightName", "ALL_LAYER");
    vector<Statement*> allLinIf = nextifaction->getTargetSubStatement("LeftName", "ALL_LAYER");
    bool isinside = 0;
    
    for(int i = 0; i < allRinC.size(); i++){
        int thisinside = 0;
        string name = allRinC[i]->getRealCode();
        name = getTrueName(name);
        for(int j = 0; j < allLinIf.size(); j++){
            if(allLinIf[j]->getRealCode() == name){
                thisinside = 1;
                break;
            }
        }
        if(thisinside){
            isinside = 1;
            vector<Attribute> availvar = getAvaliVar(conditionS);
            for(int i = 0; i > allLinIf.size(); i++){
                for(int j = 0; j < availvar.size(); j++){
                    if(availvar[j].getName() == allLinIf[i]->getRealCode()){
                        availvar.erase(availvar.begin() + j);
                        j--;
                    }
                }
            }
            int randindex1 = Tool::getRandomfromClosed(0, availvar.size() - 1);
            Statement* replaced = allRinC[i];
            string replace = availvar[randindex1].getName();
            Tool::setCodeinItem(replaced, replace);
        }
    }
    return !isinside;
}



void Builder::addsuitableinput(int width){
    Statement* ConsistnameS = mainStatement.getTargetSubStatement("*ConsistName", "FIRST_LAYER")[0];
    ConsistnameS->setIsterminal(0);
    Statement* DefS = mainStatement.getTargetSubStatement("*Define", "ALL_LAYER")[0];
    string newname = getLeftName();

    // add Attribute
    Attribute newinputA(newname, "input", width - 1, 0);
    mParameters.push_back(newinputA);

    // add ConsistName
    Statement newConsistname = generateFromItem("ConsistName", generateType);
    Statement* Names = newConsistname.getTargetSubStatement("Name")[0];
    Tool::setCodeinItem(Names, newname);
    newConsistname.codeReset();
    vector<Statement>* ConsistSub = ConsistnameS->getSubStatement();
    ConsistSub->push_back(newConsistname);

    // add Define
    vector<string> newnamevec;
    newnamevec.push_back(newname);
    Statement newDefine = generateDefine("input", newnamevec, 0);
    Statement* scopeS = newDefine.getTargetSubStatement("InitialScope", "ALL_LAYER")[0];
    Statement newscope;
    if(width > 10){
        newscope = generateFromItem("InitialScope", generateType, 0);
        Statement* s = newscope.getTargetSubStatement("PositiveDoubleNumber")[0];
        Tool::setCodeinItem(s, to_string(width - 1));
    }
    else{
        newscope = generateFromItem("InitialScope", generateType, 1);
        Statement* s = newscope.getTargetSubStatement("PositiveNumber")[0];
        Tool::setCodeinItem(s, to_string(width - 1));
    }
    *scopeS = newscope;
    vector<Statement>* DefSub = DefS->getSubStatement();
    DefSub->push_back(newDefine);
}


Statement* Builder::findcuralways(Statement* s){
    string type = s->getGrammarType();
    vector<Statement*> allAlways = mainStatement.getTargetSubStatement("Always", "ALL_LAYER");
    for(auto it = allAlways.begin(); it < allAlways.end(); it++){
        vector<Statement*> avails = (*it)->getTargetSubStatement(type, "ALL_LAYER");
        for(int i = 0; i < avails.size(); i++){
            if(avails[i] == s)
                return (*it);
        }
    }
    Tool::error("there's no such statement.");
    return nullptr;
}

int Builder::findmaxWidthofinput(string* name){
    int w = 0;
    for(int i = inputStartIndex; i < mParameters.size(); i++){
        int curW = mParameters[i].getEnd() - mParameters[i].getStart() + 1;
        if(w < curW){
            w = curW;
            (*name) = mParameters[i].getName();
        }
    }
    return w;
}

bool Builder::add_branch(Statement* statement, vector<string> outvar){
    bool is_safe = 1;
    vector<Statement*> firstl_block;
    vector<Statement*> firstl_if;

    string ifSeq = "";
    if(statement->getGrammarType() == "*EAlwaysaction" || statement->getGrammarType() == "*EIfAction" || statement->getGrammarType() == "*EIfaction"){
        ifSeq = "seq";
    }

    renew_pointer(statement, firstl_block, firstl_if);
    if(firstl_if.size() == 0) return 1;

    for(auto curblock = firstl_block.begin(); curblock < firstl_block.end(); curblock++){
        string name = (*curblock)->getTargetSubStatement("LeftName")[0]->getRealCode();
        if(!is_inside(outvar, name))
            outvar.push_back(name);
    }

    for(int n = 0; n < firstl_if.size(); n++){
        vector<Statement*> branchs;
        bool elsesafe = true;
        elsesafe = renew_branchs(firstl_if[n], branchs);

        vector<bool> init;
        vector<string> varname;
        map<string, vector<bool>> var_status;
        for(int i = 0; i < branchs.size(); i++)
            init.push_back(0);

        for(int i = 0; i < branchs.size(); i++){
            if(!branchs[i]->isContain("Block") && !branchs[i]->isContain("NoBlock") ) continue;
            vector<Statement*> LNs = branchs[i]->getTargetSubStatement("LeftName", "ALL_LAYER");
            for(int j = 0; j < LNs.size(); j++){
                string name = LNs[j]->getRealCode();
                if(is_inside(outvar, name)) continue;
                if(var_status.count(name))
                    var_status[name][i] = 1;
                else{
                    var_status[name] = init;
                    varname.push_back(name);
                    var_status[name][i] = 1;
                }
            }
        }

        for(int i = 0; i < varname.size(); i++){
            bool need = 0;
            for(int j = 0; j < branchs.size() && !need; j++){
                if(var_status[varname[i]][j] == 0)
                    need = 1;
            }
            if(!need) continue;
            bool add_in_main = rand() % 2;
            if(add_in_main){
                int if_place = getplace(statement, firstl_if[n]);
                int randplace = rand() % (if_place + 1);
                add_block(varname[i], statement, randplace, ifSeq);
                outvar.push_back(varname[i]);

                renew_pointer(statement, firstl_block, firstl_if);
                renew_branchs(firstl_if[n], branchs);
                for(int j = 0; j < branchs.size() && !need; j++){
                    var_status[varname[i]][j] = 1;
                }
            }
            else{
                elsesafe = 1;
                for(int j = 0; j < branchs.size(); j++){
                    if(!var_status[varname[i]][j]){
                        int place = rand() % (branchs[j]->getSubStatement()->size() + 1);
                        add_block(varname[i], branchs[j], place, ifSeq);
                        is_safe = 0;
                    }
                }
            }
        }
        int index = 0;
        for(; index < branchs.size() && is_safe; index++){
            bool flag = add_branch(branchs[index], outvar);
            is_safe = flag;
        }
        for(; index < branchs.size(); index++){
            add_branch(branchs[index], outvar);
        }
        if(!elsesafe){
            vector<Statement>* sub = firstl_if[n]->getSubStatement();
            sub->erase(sub->end() - 1);
        }
    }
    return is_safe;
}

void Builder::renew_pointer(Statement* statement, vector<Statement*> &firstl_block, vector<Statement*> &firstl_if){
    vector<Statement>* allactions = statement->getSubStatement();
    firstl_block.clear();
    firstl_if.clear();
    for(int i = 0; i < allactions->size(); i++){
        if((*allactions)[i].getGrammar() == "If"){
            Statement* curs = (*allactions)[i].getTargetSubStatement("If", "FIRST_LAYER")[0];
            firstl_if.push_back(curs);
        }
        else if((*allactions)[i].getGrammar() == "EIf"){
            Statement* curs = (*allactions)[i].getTargetSubStatement("EIf", "FIRST_LAYER")[0];
            firstl_if.push_back(curs);
        }
        else if(statement->getGrammarType() == "*EAlwaysaction" || statement->getGrammarType() == "*EIfaction" || statement->getGrammarType() == "*EIfAction"){
            Statement* curs = (*allactions)[i].getTargetSubStatement("NoBlock", "FIRST_LAYER")[0];
            firstl_block.push_back(curs);
        }
        else{
            Statement* curs = (*allactions)[i].getTargetSubStatement("Block", "FIRST_LAYER")[0];
            firstl_block.push_back(curs);
        }
    }
}

bool Builder::renew_branchs(Statement* If, vector<Statement*> &branchs){
    vector<string> allStarIfactionTypes = {"*Ifaction", "*EIfaction"};

    vector<Statement>* Ifbranches = If->getSubStatement();
    bool addelse = 0;
    branchs.clear();
    
    if((*Ifbranches)[Ifbranches->size() - 1].getGrammarType() != "ElseAction" && (*Ifbranches)[Ifbranches->size() - 1].getGrammarType() != "EElseAction" ){
        string IfGrammarType = If->getGrammarType();
        string elseAction = "ElseAction";
        if(IfGrammarType == "EIf"){
            elseAction = "EElseAction";
        }
        Statement elsebranch = generateFromItem(elseAction, generateType, 0, true);
        Ifbranches->push_back(elsebranch);
        addelse = 1;
    }
    
    for(int j = 0; j < Ifbranches->size(); j++){

        Statement* curbranch = &(*Ifbranches)[j];
        if(curbranch->getGrammarType() == "IfAction"){
            Statement* branchif = curbranch->getTargetSubStatement("*Ifaction", "FIRST_LAYER")[0];
            branchs.push_back(branchif);
        }
        else if(curbranch->getGrammarType() == "EIfAction"){
            Statement* branchif = curbranch->getTargetSubStatement("*EIfaction", "FIRST_LAYER")[0];
            branchs.push_back(branchif);
        }
        else if(curbranch->getGrammarType() == "*ElseIfAction"){
            vector<Statement>* elifSub = curbranch->getSubStatement();
            for(int k = 0; k < elifSub->size(); k++){
                Statement* branchif = (*elifSub)[k].getTargetSubStatement("*Ifaction", "FIRST_LAYER")[0];
                branchs.push_back(branchif);
            }
        }
        else if(curbranch->getGrammarType() == "*EElseIfAction"){
            vector<Statement>* elifSub = curbranch->getSubStatement();
            for(int k = 0; k < elifSub->size(); k++){
                Statement* branchif = (*elifSub)[k].getTargetSubStatement("*EIfaction", "FIRST_LAYER")[0];
                branchs.push_back(branchif);
            }
        }
        else{
            Statement* branchif = curbranch->getTargetSubStatement(allStarIfactionTypes, "FIRST_LAYER")[0];
            branchs.push_back(branchif);
        }
    }

    return !addelse;
}

void Builder::add_block(string name, Statement* statement, int place_n, string ifSeq){
    string statementtype = statement->getGrammarType();
    string subtype = statementtype.substr(1, statementtype.size() - 1);
    Statement s = generateFromItem(subtype, generateType, 0);
    Tool::setCodeinItem(s.getTargetSubStatement("LeftName", "ALL_LAYER")[0], name);
    Attribute* LeftNameA = findAttribute(name, 2);
    int LeftW = LeftNameA->getEnd() - LeftNameA->getStart() + 1;

    vector<Statement>* subs = statement->getSubStatement();
    subs->insert(subs->begin() + place_n, s);
    Statement* s_p = &(*subs)[place_n];

    vector<Attribute> frontvarA = getStructfrontAttribute(s_p, name, ifSeq);

    for(auto it = frontvarA.end() - 1; it >= frontvarA.begin(); it--){
        int curW = it->getEnd() - it->getStart() + 1;
        if(curW < LeftW)
            frontvarA.erase(it);
    }

    vector<Statement*> rightnameS = s_p->getTargetSubStatement("RightName", "ALL_LAYER");
    if(frontvarA.size() == 0){
        for(int i = 0; i < rightnameS.size(); i++){
            Tool::setCodeinItem(rightnameS[i], to_string(rand()));
            rightnameS[i]->setGrammar("Num");
        }
    }
    else{
        for(int i = 0; i < rightnameS.size(); i++){
            string rightnameCode = "";
            int index = rand() % frontvarA.size();
            Attribute chosenVar = frontvarA[index];
            rightnameCode += chosenVar.getName();
            int curW = chosenVar.getEnd() - chosenVar.getStart() + 1;
            if(curW > LeftW){
                int downbit = rand() % (curW - LeftW);
                string indexSetStr = "[" + to_string(downbit + LeftW - 1) + ":" + to_string(downbit) + "]";
                if(downbit + LeftW - 1 == downbit)
                    indexSetStr = "[" + to_string(downbit) + "]";
                rightnameCode += indexSetStr;
            }
            Tool::setCodeinItem(rightnameS[i], rightnameCode);
        }
    }

    checkNum(s_p);
}

bool Builder::is_inside(vector<string> vars, string name){
    for(auto it = vars.begin(); it < vars.end(); it++){
        if((*it) == name) return 1;
    }
    return 0;
}

vector<string> Builder::getStructfrontVar(Statement* place, string ifSeq){
    vector<string> frontVar;
    vector<Statement*> frontLeftS;
    vector<Statement*> path = findStatementPath(place);

    vector<Statement>* region = path[0]->getSubStatement();
    Statement* tarbranch = path[1];
    for(int j = 0; j < region->size(); j++){
        Statement* curbranch = &(*region)[j];
        if(curbranch->isContain("Assign"))
            frontLeftS.push_back(curbranch->getTargetSubStatement("LeftName", "ALL_LAYER")[0]);
        else{
            if(curbranch == tarbranch)
                break;
        }
    }
    
    for(int i = 1; i < path.size() / 2; i++){
        bool finding = 1;
        region = path[i * 2]->getSubStatement();
        tarbranch = path[i * 2 + 1];
        for(int j = 0; j < region->size() && finding; j++){
            Statement* curbranch = &(*region)[j];
            if(curbranch == tarbranch)
                finding = 0;
            else{
                vector<Statement*> LeftS = curbranch->getTargetSubStatement("LeftName", "ALL_LAYER");
                frontLeftS.insert(frontLeftS.end(), LeftS.begin(), LeftS.end());
            }
        }
    }

    //When doing no block assignment, don't scan last path node
    if(path.size() % 2 == 0 || ifSeq == "seq"){
        bool finding = 1;
        region = path[path.size() - 1]->getSubStatement();
        tarbranch = place;
        // finding = 1;
        for(int j = 0; j < region->size() && finding; j++){
            Statement* curbranch = &(*region)[j];
            string type = tarbranch->getGrammarType();
            Statement* tmp = curbranch;
            if(type != curbranch->getGrammarType()){
                if(curbranch->isContain(type))
                    tmp = curbranch->getTargetSubStatement(type, "ALL_LAYER")[0];
                else{
                    vector<Statement*> LeftS = curbranch->getTargetSubStatement("LeftName", "ALL_LAYER");
                    frontLeftS.insert(frontLeftS.end(), LeftS.begin(), LeftS.end());
                    continue;
                }
            }
            if(tmp == tarbranch)
                finding = 0;
            else{
                vector<Statement*> LeftS = curbranch->getTargetSubStatement("LeftName", "ALL_LAYER");
                frontLeftS.insert(frontLeftS.end(), LeftS.begin(), LeftS.end());
            }
        }
    }

    for(int i = 0; i < frontLeftS.size(); i++){
        string name = frontLeftS[i]->getRealCode();
        if(!is_inside(frontVar, name))
            frontVar.push_back(name);
    }

    return frontVar;
}

vector<Statement*> Builder::findStatementPath(Statement* place){
    vector<Statement*> curPath;
    stack<Statement*> list;
    stack<bool> modifylist;
    bool finding = 1;

    list.push(&(*mainStatement.getSubStatement())[8]);
    modifylist.push(0);
    while(list.size() != 0 && finding){
        bool modify = modifylist.top();
        modifylist.pop();
        if(modify){
            curPath.pop_back();
            continue;
        }
        Statement* currentS = list.top();
        list.pop();
        string currenttype = currentS->getGrammarType();
        if(currenttype == "*Action" || currenttype == "*Ifaction" || currenttype == "*EIfaction" || currenttype == "*Alwaysaction" || currenttype == "*EAlwaysaction"){
            curPath.push_back(currentS);
            modifylist.push(1);
        }
        else if(currenttype == "Action" || currenttype == "Alwaysaction" || currenttype == "Ifaction"
                || currenttype == "EAlwaysaction" || currenttype == "EIfaction"){
            if(currentS->isContain("Always") || currentS->isContain("If") || currentS->isContain("EIf")){
                curPath.push_back(currentS);
                modifylist.push(1);
            }
        }
        vector<Statement>* subS = currentS->getSubStatement();
        for(int i = 0; i < subS->size(); i++){
            Statement* cursub = &(*subS)[i];
            if(cursub == place){
                finding = 0;
                break;
            }
            string type = cursub->getGrammarType();
            if(!cursub->getIsTerminal()){
                list.push(cursub);
                modifylist.push(0);
            }
        }
        
    }
    if(curPath.size() == 0)
        Tool::error("There's no such statement.");

    return curPath;
}

vector<Attribute> Builder::getStructfrontAttribute(Statement* place, string name, string ifSeq){
    vector<string> frontvar = getStructfrontVar(place, ifSeq);
    vector<Attribute> frontvarA;
    for(int i = 0; i < frontvar.size(); i++){
        if(frontvar[i] != name)
            frontvarA.push_back(*findAttribute(frontvar[i], 2));
    }
    for(int i = inputStartIndex; i < mParameters.size(); i++){
        frontvarA.push_back(mParameters[i]);
    }

    return frontvarA;
}

int Builder::getplace(Statement* actions, Statement* tar){
    vector<string> allIfTypes = {"If", "EIf"};
    vector<Statement>* subs = actions->getSubStatement();
    for(int i = 0; i < subs->size(); i++){
        Statement* cur = &(*subs)[i];
        if(cur->isContain("If") || cur->isContain("EIf")){
            Statement* tmp = cur->getTargetSubStatement(allIfTypes, "ALL_LAYER")[0];
            if(tmp == tar)
                return i;
        }
    }
    Tool::error("Can't find target.");
    return -1;
}

Statement Builder::getnewCondition(vector<Attribute> availvarA, string generateType){
    Statement condition = generateFromItem("Condition", generateType);
    vector<Statement*> RNS = condition.getTargetSubStatement("RightName", "ALL_LAYER");

    for(int i = 0; i < RNS.size(); i++){
        int index = Tool::getRandomfromClosed(0, availvarA.size() - 1);
        string code = "";
        code += availvarA[index].getName();
        int length = availvarA[index].getEnd() - availvarA[index].getStart() + 1;
        if(length > 1){
            int uselength = Tool::getRandomfromClosed(1, length);
            int useStart = Tool::getRandomfromClosed(0, length - uselength) + availvarA[index].getStart();
            code += "[";
            if(uselength != 1)
                code += to_string(useStart + uselength - 1) + ":";
            code += to_string(useStart) + "]";
        }
        Tool::setCodeinItem(RNS[i], code);
    }

    if(condition_overlength(&condition)){
        condition = getnewCondition(availvarA, generateType);
    }

    checkNum(&condition);
    

    return condition;
}

bool Builder::condition_overlength(Statement* condition){
    vector<Statement*> calculator = condition->getTargetSubStatement("DigitDoubleOperator", "ALL_LAYER");
    vector<Statement*> comparator = condition->getTargetSubStatement("Comparator", "ALL_LAYER");
    vector<Statement*> connector = condition->getTargetSubStatement("ConditionConnector", "ALL_LAYER");
    if(calculator.size() > 10 || (comparator.size() + connector.size()) > 10){
        return 1;
    }
    return 0;
}

void Builder::handle_conditions(Statement* S, vector<Attribute> avaliVar, string isSeq){
    string ifAction = "IfAction";
    string elseIfAction = "ElseIfAction";
    if(isSeq == "seq"){
        ifAction = "E" + ifAction;
        elseIfAction = "E" + elseIfAction;
    }

    Statement* curbranch = S->getTargetSubStatement(ifAction, "ALL_LAYER")[0];
    handle_condition(curbranch, avaliVar);

    vector<Statement*> branchs = S->getTargetSubStatement(elseIfAction, "ALL_LAYER");
    for(int i = 0; i < branchs.size(); i++){
        curbranch = branchs[i];
        handle_condition(curbranch, avaliVar);
    }

    S->codeReset();
}

void Builder::handle_condition(Statement* branch, vector<Attribute> avaliVar){
    vector<Statement>* sub = branch->getSubStatement();
    sub->erase(sub->begin() + 1);
    Statement condition = getnewCondition(avaliVar, generateType);
    sub->insert(sub->begin() + 1, condition);
}