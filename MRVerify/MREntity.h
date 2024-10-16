//The interface of various MR.

#pragma once

#include <string>

using namespace std;

class MREntity{
public:
    /*
        verify: Execute verify process
        0: find defect
        -1: no need to checked by other MR
        1: find no defect
        2: don't satisfy condition
        3: single verilog program error
    */
    virtual int verify(int seq, bool& onceVerify, string& errorReport, int& caseNum, string defectCaseDirectory) = 0;
    virtual string getMRName() = 0;
    virtual string getRecentTestName() = 0;
};