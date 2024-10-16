/*
    Constructor: construct the program follow the tree shape
*/

#pragma once

#include "Executer.h"

#include <string>
#include <filesystem>

using namespace std;

class Constructor{
private:
    string initHDLPath;
    string treeHDLPath;
public:
    Constructor(string setPath);
    
    /*
    string programSetName: The name of programSet, used for rename file.
    int recentDeepLevel: The layer of tree being built. Used for rename file.
    buildTree.  Generate high deep HDL tree using init_program.
                The way of each subtree's combination is independent synthesis.
    connType:   "random", generate random trees, whose number is initNum/subChildNum, of next level.
                "full", generate all the trees possible provided by recent inital program.
    updateType: "overlap", remove all the HDL trees generated before.
    simulate_check: Default off. If set "on", all the new program will be check acivated.
    conRatio: New percentage of the number of each new HDL generated to whole program.
    */
    bool buildTree(string programSetName, int recentDeepLevel, string connType="random", string updateType="overlap", bool simulate_check=true, double conRatio=0.5);
};