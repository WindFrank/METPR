/*
    VarietyEvaluation: static method set to evaluate the variety of a verilog program.
*/

#pragma once

#include <set>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

using namespace std;

class VarietyEvaluation{
private:
    static int moduleProcess(vector<string> items, string& moduleName);
    static string moduleReplace(string callLine, string content);
public:
    //Turn verilog to formular expression. Only the verilog processed by it can be used to build graph.
    static string fromVtoFormularG(string filePath, string mainModule);
    //is operate syntax
    static bool isOperateSyntax(string note);
    //change split var to single name
    static string washVarName(string rawVar);
    //Split var with any syntax
    static vector<string> splitVar(string line);
    //Extract first var from statement
    static vector<string> extractFirstVar(string statement);
    // split string with char into vector
    static vector<string> split(string line, char ch);
    // replace var in items and connect them with " "
    static string replaceVar(vector<string> items, map<string, string> replaceRelation);
    // format the verilog to standard content, return the string
    static string formatVerilog(string filePath);
    // turn extra grammar to learned grammar
    static string turnGrammar(string content);
};