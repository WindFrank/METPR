/*
    Node: All the data structure in xml
*/

#include "../Arch.h"

#include <vector>
#include <iostream>

using namespace std;

class GNode{
public:
    virtual double getDelay(string firstEnd, string secondEnd) = 0;
    virtual string getInstance() = 0;
};