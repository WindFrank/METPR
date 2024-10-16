/*
VTRDrive: Execute the VTR command on VTR
*/
#pragma once

#include "../Generator/Executer.h"
#include "RouteAnalyzer.h"
#include "DelayandArea.h"
#include "Drive.h"
#include "BlifAnalyzer.h"

#include <string>
#include <map>

using namespace std;

class VTRDrive : public Drive{
private:
    string VTRRoutePath = "";
    //Get the value behind the string
    
    static vector<string> getBlockNames(string content);
    static bool isInput(string line, int arraySizeX, int arraySizeY);
    static stringstream getModel(string filepath, string modelName);
    static vector<string> getAllModelNames(string blif);
    static vector<string> getFirstVariables(string names);
public:
    /*
    find value after the signStr
    content: file content
    signStr: the flag string
    */
    static double getValueBehindStr(string content, string signStr);
    /*
    rootPath: the path of VTR root content.
    */
    VTRDrive(string rootPath) : 
    VTRRoutePath(rootPath){};

    /*
    Run the VTR standard flow using run_vtr_flow.py

    programPath: the path of target verilog program file (.v).
    archPath: the path of target EArch file (.xml).
    tempDir: the path of content where the products should be saved.
    topModule: the top module of the verilog.
    */
    string executeRunFlow(string programPath, string archPath, string tempDir, string topModule) override;
    string executeRandomRunFlow(string programPath, string archPath, string tempDir, string topModule, int seed);

    /*
    Run the target .blif using vpr to execute pack, place and route process.

    blifPath: the path of .blif.
    archPath: the path of target EArch file (.xml).
    pinFixedPath: the path of pinFixed file which is used to fix the location of all the IO pins.
    tempPath: the location you want to put the product to.
    topModule: in VTR, it is no use
    */
    string runBlif(string blifPath, string archPath, string pinFixedPath, string tempPath, string topModule="") override;
    string runSeedBlif(string blifPath, string archPath, string pinFixedPath, string tempPath, string topModule="", int seed=0);

    /*
    Get the critical delay and area from file vpr_stdout.log
    logPath: the path of vpr_stdout.log
    */
    static DelayandArea getCritDelayandAreafromLog(string logPath);

    /*
    Combine two locations of pins in two solution.
    testDirectory: target content
    */
    static string pinCommandFixed(string filepath1, string filepath2, string testDirectory);

    /*
    Combine two .blif files
    testDirectory: target content
    */
    static string blifCom(string filepath1, string filepath2, string testDirectory);

    /*
    Get part wire lengths of a solution.
    In VTR, the length of a solution is calculated by do the addition of all segments length.
    partCirName: the name of circuit of which you want to get the solution.
    allRA: the whole RouteAnalyzer containing target circuit.
    deleteWireOut: the outpin of the cirt route in origin solution.
    outX, outY, pad: detailed location of outpin.
    */
    static double getPartWrieLength(string partCirName, RouteAnalyzer allRA, string deleteWireOut, int outX, int outY, int pad);

    /*
    get io nums, blocks, total_nets, fanout_ave, fanout_max and occ from log file.
    vproutPath: the path of target vpr.out
    */
    static string getFeaturesFromLog(string vproutPath);

    /*
    combine setup timing rpt and hold rpt
    */
    static bool timingRptCom(string runDirectory);

    static string writeSDCforExec(string programPath, string outputDir);
    static string writeSDCforBlif(string blifPath, string outputDir);
};