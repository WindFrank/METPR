/*
Drive: all the interface of EDA systme testing drives.
*/
#pragma once

#include "DelayandArea.h"

#include <string>

using namespace std;

class Drive{
public:
    /*
    Run the standard flow using EDA system.

    programPath: the path of target verilog program file (.v).
    archPath: the path of target arch file.
    tempDir: the path of content where the products should be saved.
    topModule: the top module of the verilog.
    return error message.
    */
    virtual string executeRunFlow(string programPath, string archPath, string tempDir, string topModule) = 0;

    /*
    Run the target primitive netlist.v using EDA system to execute place and route process.
    In VTR, it is described by .blif.

    netlistPath: the path of primitive netlist.v.
    archPath: the path of target arch file.
    pinFixedPath: the path of pinFixed file which is used to fix the location of all the IO pins.
    tempPath: the location you want to put the product to.
    */
    virtual string runBlif(string netlistPath, string archPath, string pinFixedPath, string tempPath, string topModule) = 0;


    /*
    Copy test case to target content
    treatFilePath anotherFilePath: full path and filename of test cases.
    testDirectory: the path of content where you execute MR verification.
    targetContent: the path of folder save test cases.
    */
    static void copyTestCases(string treatFilePath, string anotherFilePath, string testDirectory, string targetContent){
        string copyTreat = "cp -r " + treatFilePath + " " + testDirectory + ";";
        string copyAnother = "cp -r " + anotherFilePath + " " + testDirectory + ";";
        string copyTestDirectory = "cp -r " + testDirectory + " " + targetContent; 
        string full_command = copyTreat + copyAnother + copyTestDirectory;
        system(full_command.c_str());
    }
};