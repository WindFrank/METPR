/*
Tool: Some method used frequently.
*/

#pragma once

#include "lib/JSONcpp/include/json/json.h"
#include "Discription/Statement.h"
#include "Generator/Builder.h"
#include "Discription/GraphMaxSub.h"
// #include "lib/rapidXML/rapidxml_ns.hpp"
// #include "lib/rapidXML/rapidxml_ns_print.hpp"
// #include "lib/rapidXML/rapidxml_ns_utils.hpp"
#include "lib/tinyXML/tinyxml.h"

#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <unordered_set>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <set>
#include <ctime>
#include <Python.h>

#include "Boost_recent.h"

using namespace std;
namespace bfs = boost::filesystem;

// using namespace rapidxml_ns;

class Tool{
private:
    static string logFileTime;
    static vector<string> getVariables(string def);
    static void parseGraphLine(const string& line, GraphMaxSub& graph);
    static void findMaxCommonSubgraph(
        const string& node1, const string& node2,
        const GraphMaxSub& g1, const GraphMaxSub& g2,
        unordered_map<string, string>& current_match,
        unordered_map<string, string>& best_match,
        unordered_set<string>& visited1, unordered_set<string>& visited2
    );
    static bool containsRequiredTypes(const unordered_map<string, string>& match, const GraphMaxSub& g1, const GraphMaxSub& g2);
    static pair<string, string> printGraphFromMapping(const unordered_map<string, string>& mapping, const GraphMaxSub& g1, const GraphMaxSub& g2);
    static string generateSubgraphString(const unordered_map<string, set<string>>& subgraph, const GraphMaxSub& g1);
public:
    static string getLogFileTime();
    static vector<string> getAllModuleNames(string verilog);
    //Find grammar from json.
    static string findGrammar(string, string, int expIndex=0);
    //Extract the structure of a statement grsammar
    static vector<string> extractGrammar(string);
    //Judge if the char is a word.
    static int isWord(char c);
    //Report error
    static void error(string info);
    //Get number index from JSON
    static int getNumIndex(string grammarType, string generateType);
    //Generate terminal log information
    static void logMessage(string message);
    //Extract statement from exist Statement pointer vector.
    static vector<Statement*> getTypeStateFromV(vector<Statement*> initialV, string grammarType, string actionType);
    //Check if a pair of statement and value in the vector.
    static int isContianInV(vector<Statement*> initialV, string value, string grammarType);
    //Set realCode in a Statement item.
    static void setCodeinItem(Statement* item, string value);
    //Random generate num from closed interval
    static int getRandomfromClosed(int start, int end);
    //Int to Binary and return its String
    static string intToBinary(int value);
    //Find Statement containing target statement with certain value from multi-State
    static vector<Statement*> findSfromMultiS(Statement* multiS, string grammarType, string value);
    //File write. If get 0, the file was written successfully.
    static bool fileWrite(string filename, string text, bool ifPrint=true);
    //Get Tree of XML file
    static TiXmlElement* getXMLTree(string filename, string type="");
    //Split string with a single syntax. If clean=="clean", replace the \t and ' ' to a single space.
    static vector<string> split(string s, char c, string clean="");
    //Get num from close bracket.
    static int getNumFromClose(string s);
    //Find target node in tinyxml
    static TiXmlElement* getFirstElementByName(TiXmlElement* root, string name);
    //2-dimension split
    static vector<vector<string>> twoDSplit(string s, char smallC, char largeC);
    //Get the pair number from "(x,y)"
    static pair<int, int> getBracketPair(string s);
    //Combine verilog program
    static void verilogCom(string filepath1, string filepath2, string targetDirectory, string newFileName);
    //Judge if the directory exists on linux.
    static bool isDirectoryExists(string path);
    //Judge if the file exists.
    static bool isFileExists(string path);
    //Create mkdir on linux.
    static bool createDirectory(string path);
    //Judge if the director is empty
    static bool isDirectoryEmpty(const char* path);
    //Let two verilogs do series connections
    static void verilogSeiresCom(string filepath1, string filepath2, string targetDirectory, string newFileName, string connStrategy="simple");
    //Extract input and output attributes from text.v
    static bool extractInoutfromTextV(stringstream& module, vector<Attribute*>& outputs, vector<Attribute*>& inputs);
    //Generate decalaration by Attribute. The type assigns if the inout type should be cared about ("inout").
    static string genDecalaration(vector<Attribute*> av, string type);
    //deep copy vector<Attribute>
    static vector<Attribute> deepCopyAttr(vector<Attribute*> avstr);
    //Check if all the string in V is 1
    static bool oneCheckonV(vector<string> sv);
    //Copy file from pathFile1 to pathFile2
    static bool copyFile(string pathFile1, string pathFile2);
    //Get all file from recent directory
    static void getAllFilefromDir(string dirPath, vector<string> &allFilePath);
    //Compare if a elem appears in both two vector
    static bool same_vec(const vector<int>& a, const vector<int>& b);
    static bool same_vec(const vector<string>& a, const vector<string>& b);
    static bool same_vec(vector<pair<int, int>> a, vector<pair<int, int>> b);
    //Find file from path
    static string findFilefromPath(string path);
    //Get recent time
    static string getRecentTime();
    //Get the content from target file
    static string readFile(string path);
    //exclude the nonsense character
    static string washString(string initString);
    //Judge if the string is number
    static bool isNum(string str);
    //Judge if duplicated elements in the vector
    static bool hasDuplicate(const vector<string>& vec);
    //Get formula from .graph
    static string getFormulafromGraph(string filePath);
    //Copy directory from strSourceDir to strDestDir
    static void copyDirectory(const string& strSourceDir, const string& strDestDir);
    //get the stringstream of target module
    static stringstream getModule(string filepath, string moduleName);
    static stringstream getStrModule(string content, string moduleName);
    //Judge if the variable is system keyword.
    static bool isSystemKeyword(string variable);
    //Replace all the assigned string to another in a str
    static string replacePartStr(string str, string oldStr, string newStr);
    //Turn vector<int> to its str expression.
    static string arrayToStr(vector<int> arr);
    //Persist Builder object
    static void serializeBuilder(Builder builder, string targetDirectory);
    static Builder deserializeBuilder(const string& filename);
    //get the distance between two point
    static double twoPointsDistance(pair<int, int> point1, pair<int, int> point2);
    //judge if the condition is absolute
    static int isConditionAbsolute(string verilog, int index);
    //get max sub graph from two graph. first: node; second: type; others: next nodes.
    static pair<string, string> getMaxSubGraph(string graph1, string graph2);
    //find last number in "[]"
    static int getLastNumberInBrackt(string text);
    //turn graph str to graph
    static GraphMaxSub parseGraph(const vector<string>& lines);
    //get manhattan distance from two point
    static double getManhattan(pair<int, int> first, pair<int, int> second);
    //modify the name repeat
    static string repeatModify(string &line, int startIndex, int endIndex, vector<string> existItems);
    //judge if the name should add "\\" front the name which may lost it
    static bool ifEscapeName(string name);
};