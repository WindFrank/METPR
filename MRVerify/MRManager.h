/*
    MRManager: recognize, call and use the MR
*/
#include "../EDAExecuter/VTRDrive.h"
#include "../Arch.h"
#include "MREntity.h"
#include "MRConsistLayout.h"
#include "MRAddRouteConsist.h"

#include <string>
#include <vector>

using namespace std;

class MRManager{
private:
    vector<string> callMRs;
    VTRDrive& vtr;
    string treatProgram;
    string treeHDLPath;
    string productPath;
    vector<string> programs;
    Arch arch;
    // vector<int> rrGraphUsed;
    vector<MREntity*> targetMRs;
    string errorReportPath;
public:
    MRManager(vector<string> callMRs, VTRDrive& vtr, string programName, 
        string treeHDLPath, vector<string> allPrograms, Arch arch, string errorReportPath);
    bool MRVerify(int seq, int& caseNum, double& defect, string defectTestCase);
    // /*
    // seq: The test No.
    // */
    // bool MRConsistLayout(int seq);
    // string blifCom(string filepath1, string filepath2, string outPath);
    // string pinCommandFixed(string filepath1, string filepath2, string outPath);
};