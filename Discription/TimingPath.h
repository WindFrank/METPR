/*
Save the info of a single timing path
*/

#pragma once

#include "TimingPoint.h"

#include <string>
#include <vector>

using namespace std;

class TimingPath{
public:
    pair<string, string> startPointandClk;
    pair<string, string> endPointandClk;
    string pathType;

    double destination_clock_delay;
    double source_clock_delay;
    double clock_pessimism_removel;

    double clock_uncertainty;

    double data_path_delay;
    int logic_levels;

    double data_required_time;
    double data_arrival_time;

    double slack;

    bool isClockPath = false;
    bool isStartRegister = false;
    bool isEndRegister = false;

    vector<TimingPoint> dataArrivalPoints;
    vector<TimingPoint> dataRequiredPoints;
    vector<int> mainArrivalIndexes;
    vector<int> mainRequiredIndexes;

    double getClockSkewPath(){
        return destination_clock_delay - source_clock_delay + clock_pessimism_removel;
    }

    double getDataPathDelay(){return data_path_delay;}

    pair<string, string> getStartPointandClk(){return startPointandClk;}

    pair<string, string> getEndPointandClk(){return endPointandClk;}

    double getSlack(){return slack;}

    double getWireLength(){
        //Programming according to timing report
        return 0.0;
    };
};