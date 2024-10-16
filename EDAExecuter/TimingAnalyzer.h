/*
Used to analyze info of VTR timing report.
*/

#pragma once

#include "../Discription/TimingPath.h"

#include <string>
#include <vector>

using namespace std;

class TimingAnalyzer{
public:
    vector<TimingPath> paths;
    double critPathDelay = 0.0;
    TimingPath* critPath;
    double worstSlack = 2148790;

    bool isTimingKeyword(string firstItem);
    void loadTimingRpt(string filePath);
    TimingPath getTimingPathByIOs(string inpad, string outpad);
    double getAssignedPartDelay(string programModule);
    double getAssignedPartSlack(string programModule);
};