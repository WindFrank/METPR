/*
Used to save the info of a single subckt in .blif file
*/

#pragma once

#include <string>
#include <vector>

using namespace std;

class Subckt{
public:
    string name;
    vector<pair<string, string>> ports;
    string toString(){
        string result = ".subckt " + name + " ";
        for(int i = 0; i < ports.size(); i++){
            string recentPort = ports[i].first + "=" + ports[i].second + " ";
            result += recentPort;
            if(i != 0 && i % 5 == 0 && i != ports.size() - 1)
                result += "\\\n ";
        }
        return result;
    }
};