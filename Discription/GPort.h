/*
    Port: Struct. Save the port strings which are used to locate itself.
*/

#include <iostream>
#include <string>

using namespace std;

class GPort{
public:
    string type;
    string portName;
    int pinIndex;
    GPort(string type, string portName, int pinIndex);
    string getPortExp();
};