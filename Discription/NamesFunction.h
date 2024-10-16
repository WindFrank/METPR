/*
Used to save the names function in .blif files.
*/

#pragma once

#include <string>
#include <vector>

using namespace std;

class NamesFunction{
public:
    vector<string> funcInputs;
    string funcOutput;
    vector<string> mapRules;

    string toString(){
        string result = ".names ";
        for(int i = 0; i < funcInputs.size(); i++){
            string recentPad = funcInputs[i] + " ";
            result += recentPad;
            if(i != 0 && i % 5 == 0 )
                result += "\\\n ";
        }
        result += funcOutput + " \n";
        for(string rule : mapRules){
            result += rule + "\n";
        }

        return result;
    }
};