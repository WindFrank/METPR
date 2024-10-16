#include "TimingAnalyzer.h"
#include "../Tool.h"

void TimingAnalyzer::loadTimingRpt(string filePath){
    string content = Tool::readFile(filePath);
    vector<string> lines = Tool::split(content, '\n');

    TimingPath* recentPath;
    int pointIndex = 0;
    bool startLoadPoint = false;
    bool data_arrival_point_settle = true;  //control the insert vector of TimingPoint

    for(string recentLine : lines){
        recentLine = Tool::washString(recentLine);
        vector<string> items = Tool::split(recentLine, ' ');
        if(items.size() == 0 || (items[0] == "" && items.size() == 1))   //empyt line
            continue;
        string firstItem = items[0];
        if(firstItem == "#Path"){
            recentPath = new TimingPath();
        }
        else if(firstItem == "Point"){  //Start to load point
            pointIndex = 0;
            startLoadPoint = true;
        }
        else if(startLoadPoint){
            if(firstItem == "data" && items[1] == "required"){
                int arr = items.size() - 1;
                while(items[arr] == "")
                    arr--;
                recentPath->data_required_time = stof(items[arr--]);
                startLoadPoint = false;
            }
            else if(firstItem == "data" && items[1] == "arrival"){
                int arr = items.size() - 1;
                while(items[arr] == "")
                    arr--;
                recentPath->data_arrival_time = stof(items[arr--]);
                recentPath->data_path_delay = recentPath->data_arrival_time;
                if(critPathDelay < recentPath->data_path_delay){
                    critPathDelay = recentPath->data_path_delay;
                    critPath = recentPath;
                }

                data_arrival_point_settle = false;
                pointIndex = 0;
            }
            else{
                if(recentLine[0] == '-' && recentLine[1] == '-')
                    continue;
                TimingPoint tp;
                bool pathLoad = false;
                bool IncrLoad = false;
                string pointName = "";
                string lastItem = "";
                for(int r = items.size() - 1; r >= 0; r--){
                    string recentItem = items[r];
                    lastItem = recentItem;
                    if(Tool::isNum(recentItem)){
                        if(!pathLoad){
                            tp.Path = stof(recentItem);
                            pathLoad = true;
                        }
                        else if(!IncrLoad){
                            tp.Incr = stof(recentItem);
                            IncrLoad = true;
                        }
                    }
                    else{
                        pointName = recentItem + " " + pointName;
                        tp.pointName = pointName;
                    }
                }

                bool isMain =false;
                if(!isTimingKeyword(lastItem))
                    isMain = true;

                if(data_arrival_point_settle){
                    recentPath->dataArrivalPoints.push_back(tp);
                    if(isMain)
                        recentPath->mainArrivalIndexes.push_back(pointIndex);
                    pointIndex++;
                }
                else{
                    recentPath->dataRequiredPoints.push_back(tp);
                    if(isMain)
                        recentPath->mainRequiredIndexes.push_back(pointIndex);
                    pointIndex++;
                }
            }
        }
        else if(firstItem == "Startpoint:"){ //Start point and clk
            string startPoint = items[1];
            string clock = items[7];
            clock.pop_back();
            recentPath->startPointandClk = pair<string, string>(startPoint, clock);
            if(startPoint.find(".inpad") == string::npos)
                recentPath->isStartRegister = true;
        }
        else if(firstItem == "Endpoint"){  //end point and clk
            string endPoint = items[2];
            string clock = items[8];
            clock.pop_back();
            recentPath->endPointandClk = pair<string, string>(endPoint, clock);
            if(endPoint.find(".outpad") == string::npos)
                recentPath->isEndRegister = true;
        }
        else if(firstItem == "Path" && items.size() >= 2 && items[1] == "Type"){    //Path type
            recentPath->pathType = items[3];
        }
        else if(firstItem == "slack"){  //slack note and path end
            recentPath->slack = stof(items[2]);
            paths.push_back(*recentPath);
            data_arrival_point_settle = true;
            recentPath->logic_levels = (recentPath->mainArrivalIndexes.size() - 2) / 2;
            if(recentPath->slack < worstSlack)
                worstSlack = recentPath->slack;
        }
    }
}

bool TimingAnalyzer::isTimingKeyword(string firstItem){
    vector<string> keywords = {"clock", "input", "output", "library", "data",
                               "MUX4", "MUX8", "MUX16", "MUX32", "SWBOX", "|"};
    firstItem = firstItem.substr(0, firstItem.find("_"));
    for(string keyword : keywords){
        if(firstItem == keyword)
            return true;
    }
    return false;
}

TimingPath TimingAnalyzer::getTimingPathByIOs(string inpad, string outpad){
    for(TimingPath tp : paths){
        string recentStart = tp.startPointandClk.first;
        recentStart = recentStart.substr(0, recentStart.find("."));
        string recentEnd = tp.endPointandClk.first;
        recentEnd = recentEnd.substr(0, recentEnd.find("."));
        if(inpad == recentStart && "out:" + outpad == recentEnd)
            return tp;
    }
    return TimingPath();
}

double TimingAnalyzer::getAssignedPartDelay(string programModule){
    string moduleFlag = programModule + "^";
    double result = 0.0;
    for(TimingPath tp : paths){
        string recentStart = tp.startPointandClk.first;
        string recentEnd = tp.endPointandClk.first;
        if(recentStart.find(moduleFlag) != string::npos && recentEnd.find(moduleFlag) != string::npos){
            double recentDelay = tp.data_path_delay;
            if(result < recentDelay)
                result = recentDelay;
        }
    }
    return result;
}

double TimingAnalyzer::getAssignedPartSlack(string programModule)
{
    string moduleFlag = programModule + "^";
    double result = 218920000;
    for(TimingPath tp : paths){
        string recentStart = tp.startPointandClk.first;
        string recentEnd = tp.endPointandClk.first;
        if(recentStart.find(moduleFlag) != string::npos && recentEnd.find(moduleFlag) != string::npos){
            double recentSlack = tp.slack;
            if(result > recentSlack)
                result = recentSlack;
        }
    }
    return result;
}
