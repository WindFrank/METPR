#pragma once

#include <iostream>
#include <string>

using namespace std;

class SwitchInfo{
public:
    int id;
    string type;
    string name;
    double R;
    double Cout;
    double Cin;
    double Tdel;
    double mux_trans_size;
    double buf_size;    //It maybe a double of string, use string to save.
    double delay = -1.0;

    SwitchInfo();
    SwitchInfo(int id, string type, string name, double R, double Cout, double Cin, double Tdel, double mux_trans_size, double buf_size);
};