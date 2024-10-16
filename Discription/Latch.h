/*
Used to descirpe .latch structure in .blif file
*/

#pragma once

#include <string>

using namespace std;

class Latch{
public:
    string input;
    string output;
    string type;
    string control;
    int init_val;

    Latch(){};
    Latch(string input, string output, string type, string control, int init_val) :
        input(input), output(output), type(type), control(control), init_val(init_val){};
    string toString(){
        string result = ".latch " + input + " " + output + " " + type + " " + control + " " + to_string(init_val);
        return result;
    }
};