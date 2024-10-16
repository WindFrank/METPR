/*
Statement: a single program action.
*/
#pragma once

#include "Attribute.h"

#include <iostream>
#include <string>
#include <vector>
#include <queue>


using namespace std;

class Statement{
public:
    Statement();
    Statement(string, string, string);
    Statement(string, string, int, vector<Statement>);
    // Statement(string, int, int, vector<Statement>, string);
    ~Statement();
    void setRealCode(string value);
    void setGrammar(string grammarType);
    string getRealCode();
    string getGrammarType();
    string getGrammar();
    void setIsterminal(int value);
    int getIsTerminal();
    void setExpIndex(int value);
    int getExpIndex();
    vector<Statement>* getSubStatement();
    void resetSubStatement();
    vector<Attribute*>* getParameters();
    void codeReset();
    //Turn the Statement into xml file.
    string save(int tabNum = 0, string filename="");
    /*
    Get sub-statements with target Type.
    The FISRT_LAYER means only return the statement with same type in the recent subset.
    The ALL_LAYER means find all statement with the same type in the all subset and their subset.
    */
    vector<Statement*> getTargetSubStatement(string grammarType, string action = "FIRST_LAYER");
    vector<Statement*> getTargetSubStatement(vector<string> grammarTypes, string action = "FIRST_LAYER");
    vector<Statement*> getTargetSubStatement_DFS(string grammarType);
    vector<Statement*> getTargetSubStatement_DFS_beforeS(string grammarType, bool* isfinish, Statement* currentS = nullptr);
    /*
    Check if the Statement or subStatement have target value of a type
    */
    int isContain(string grammarType, string value = "", string findType = "ALL_LAYER");
private:
    string grammarType;
    string grammar;
    int expIndex;
    int isTerminal;
    vector<Statement> subStatement;
    string realCode = "";
    string generateType = "";
    //The parameters saves the Attributes newly defined here.
    vector<Attribute*> parameters;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & grammarType;
        ar & grammar;
        ar & expIndex;
        ar & isTerminal;
        ar & subStatement;
        ar & realCode;
        ar & parameters;
    }
};