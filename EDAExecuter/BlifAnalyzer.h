/*
Used to save info from .blif files. (VTR)
*/

#pragma once

#include "../Discription/Subckt.h"
#include "../Discription/NamesFunction.h"
#include "../Discription/BlifModel.h"
#include "../Discription/Latch.h"
#include "../Discription/BlifGraphNode.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

class BlifAnalyzer{
public:
    string mainString = "";
    vector<string> inputs;
    vector<string> outputs;
    vector<Latch> latchs;
    vector<Subckt> subckts;
    vector<NamesFunction> names;
    vector<BlifModel> models;
    map<string, BlifGraphNode> blifNodes;

    string graphResult = "";

    BlifAnalyzer(){};
    BlifAnalyzer(string filePath);

    void loadBlif(string content);

    /* return the .blif formait string*/
    string getBlifContent();

    /*
    Add modulename^ before all internal variables
    */
    bool addModuleTag();

    /*
    Judge if the variable has modulename^
    */
    bool isVarModuleTag(string var);

    /*
    connectCrit: remove the secondinput and connect oldOutput to it
    The method is used for addRouteConsist testing.
    */
    bool connectCrit(string oldOutput, string secondInput);

    /*
    toGraph: turn the recent content to graph format:
    nodename nodetype backnode1 backnode2 ...
    reset: if the graph is generated but the sub content is modified, you need to set it "1";
    */
    string toGraph(bool reset=0);
    
    /*
    commonGraphChange: change the blif structure to the commonGraph
    */
    bool commonGraphChange(string commonGraph);
};