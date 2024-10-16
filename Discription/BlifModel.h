/*
Used to save type model in .blif file.
*/

#pragma once

#include <string>
#include <vector>

using namespace std;

class BlifModel{
public:
    string name;
    vector<string> inpads;
    vector<string> outpads;

    string toString(){
        string result = ".model " + name + "\n\n";

        //inpads
        result += ".inputs ";
        for(int i = 0; i < inpads.size(); i++){
            result += inpads[i] + " ";
            if(i != 0 && i % 20 == 0 && i != inpads.size() - 1)
                result += "\\\n ";
        }
        result += "\n\n";

        //outpads
        result += ".outputs ";
        for(int i = 0; i < outpads.size(); i++){
            result += outpads[i] + " ";
            if(i != 0 && i % 20 == 0 && i != outpads.size() - 1)
                result += "\\\n ";
        }
        result += "\n\n.blackbox\n\n.end";

        return result;
    }
};