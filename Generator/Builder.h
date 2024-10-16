/*
Firstly, we use the builder to provide a single statement of a program each time.
The code newly generated will be tested in filter and checker.
*/
#pragma once

#include "../Discription/Statement.h"

#include <iostream>
#include <ctime>
#include <string>
#include <math.h>
#include <map>
using namespace std;

class Builder{
private:
    string programName = "";
    string summaryCode = "";
    string generateType = "";
    Statement mainStatement;
    vector<Attribute> mParameters;
    vector<Attribute> innerParameters;
    int inputStartIndex = 1;
    //If the output is iniital
    vector<int> isOutpuInitial;
    //Max Width
    int varMaxw = 0;
    int out_begin = 0;
    int randaddtimes = 0;
    vector<string> clks;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & programName;
        ar & summaryCode;
        ar & mainStatement;
        ar & mParameters;
        ar & innerParameters;
        ar & inputStartIndex;
        ar & isOutpuInitial;
        ar & varMaxw;
        ar & out_begin;
        ar & randaddtimes;
    }
public:
    Builder(string programName, int addCommand, int varMaxW = 10, int out_begin = 15);
    ~Builder();
    //set generate program type. If set "VTR", the program generated will lose some grammar
    bool setGenerateType(string type){
        generateType = type;
        return true;
    }

    //Get the information report.
    void info();
    void testinfo();
    // check if param is in paramVector
    bool isNameInParamVector(string name, vector<Attribute> parameterVector);
    //Get the summary code
    string getSummaryCode();
    //Get the program name.
    string getProgramName();

    //Get a proper leftname
    string getLeftName();
    Statement SureLeftName(string* LeftName, string NewName, string datatype, int out_use);
    Statement getNewDefine(string chooseName, string datatype);
    Attribute getTargetVar(string name);
    bool HandlerightExpr(vector<Attribute> avaliVar, vector<Statement*> subVar, Statement* s, int newVarEnd, string type);

    string GetNewInput();
    // get name without scope
    string getTrueName(string name);
    void ReplaceRightname(string name, Statement* replaced);
    void ReplaceNumber(string name, Statement* replaced);
    void replaceInput(string name, Statement* replaced, int Width);
    int Numbitwidth(int x);
    Statement* getTargetDefine(string name);

    string getRandomVarName(vector<Attribute> avaliVar);

    // get the outputs that haven't been initialled or their type is reg
    vector<Attribute> getRegAndNotInitialOutputs();
    // get the inner vars with the reg datatype
    vector<Attribute> getInnerRegVariables();

    // delete a parameter from mParameters/innerParameters
    void deleteParameter(string name, bool isIO);
    void findanddeleteParameterinsensitivitylist(string name);
    // delete a substatement with realCode of deleteGrammarType in *Alwaysaction from Always Statement
    bool deleteStatementFromAlways(Statement* alwaysStatement, string realCode, string deleteGrammarType);
    // delete all the empty statements in Always
    bool deleteEmptySubStatementsInAlways(Statement* alwaysActionStatement);
    // delete all the define statements of parameter with name
    void deleteDefineStatements(string name);

    bool adjustIFStatementInAlways(Statement* alwaysActionStatement);
    bool adjustIFStatementInIF(Statement* IfState);

    //Add a statement in recent program.
    Statement* randomAddStatement();

    //Checker: check if the recent program is valud
    int checker(int maxRandaddTimes, int stateIter);
    /*
    checkPin: check if all the wire 
    return vector<int>: [checkPinState, conditionReset]
    stateIter: need number of program lines
    */
    vector<int> checkPin(int maxRandaddTimes, int ifCheck, int stateIter);
    void checkNum(Statement* NowStatement);
    void resetIsconst();

    //checkSimulate: use synthesis eda software to verify the program
    int checkSimulate();
   
    // Analyze and generate the code of a grammar item, ifOnlyStructure means generate a statement whose *Alwaysaction is empty
    static Statement generateFromItem(string grammarItem, string generateType, int grammarIndex = -1, bool ifOnlyStructure = false);
    //Scope handling
    static string scopeHandle(string grammarItem);

    //Get statement in builder
    Statement* getStatement();
    //The function used to specially define the Define statement.
    Statement generateDefine(string physicalValue, vector<string> names, int isPure = -1);
    //Statement* generateDefine(string physicalValue, vector<string> names, int end);

    //Find avaliable parameters in the recent location.
    vector<Attribute> getAvaliVar(Statement* currS);
    //Tiny method, return string whose all bits is "1"
    string fullPinSet(int length);
    //Find the var info from define
    Attribute findDefineInfo(string name, Statement* mDefine);
    //Tiny method, find Attribute
    Attribute* findAttribute(string name, int type);
    //Tiny method, find the complement set of scopeSet
    vector<pair<int, int>> getComplementSet(vector<pair<int, int>> nullSet, int end);
    //Tiny method, calculate the pair vector's value len
    int getPairVectorValueLen(vector<pair<int, int>> v);
    //Tiny method, get the arrow of the Pin No.
    int getArrow(int originNum, vector<pair<int, int>> set);
    //Process method, scopeSet statistic
    vector<pair<int, int>> getScopeSet(string pinOcuppy);

    // get the used pin number
    int getUsedPinNumber(string pinOcuppy);
    //Process method, the define adjustment in pin check
    void checkPinDefineAdj(Attribute param, vector<Statement>* mDefine, int newLen);
    //Process method, the used adjustment in pin check
    void checkPinUsedAdj(string leftName, vector<pair<int, int>> usedPinSet, Statement* mActionState);
    // bool check_successful();

    // randomly select var from mutable variables
    vector<Attribute> getAllMutableVar(Statement* currS);
    vector<int> MutVarMaxWidth(vector<Attribute> mutVar);
    void replaceSwithA(Statement* statement, Attribute* attribute, int width);
    int RightNamebitwidth(string* recentName);
    vector<Attribute*>  getinitializedreg(Statement* curalways);
    int LeftNametimes(string name);

    int deleteStatement_ofthisLeftName(string leftName);
    int deleteLeftname_in_statement(string leftName, Statement* statement);

    bool checkisconst();
    void findInitialisconst();
    void findInitialisconstinif(Statement* ifS);
    void calculateisconst(Statement* assignment);

    bool handleifcondtion_issafe(Statement* statement);
    bool handlesingleif_issafe(Statement* statement, Statement** next);
    bool handlesingleif_issafe_loop(Statement* statement);

    // void addOnebitreg();
    void addsuitableinput(int width);

    Statement* findcuralways(Statement* s);

    int findmaxWidthofinput(string* name);

    bool add_branch(Statement* statement, vector<string> outvar);
    void renew_pointer(Statement* statement, vector<Statement*> &firstl_block, vector<Statement*> &firstl_if);
    bool renew_branchs(Statement* If, vector<Statement*> &branchs);
    void add_block(string name, Statement* statement, int place_n, string ifSeq = "");
    bool is_inside(vector<string> vars, string name);
    vector<string> getStructfrontVar(Statement* place, string ifSeq = "");
    vector<Statement*> findStatementPath(Statement* place);
    vector<Attribute> getStructfrontAttribute(Statement* place, string name = "", string ifSeq = "");

    int getplace(Statement* actions, Statement* tar);
    Statement getnewCondition(vector<Attribute> availvarA, string generateType);
    bool condition_overlength(Statement* condition);

    void handle_conditions(Statement* S, vector<Attribute> avaliVar, string isSeq="block");
    void handle_condition(Statement* branch, vector<Attribute> avaliVar);

};