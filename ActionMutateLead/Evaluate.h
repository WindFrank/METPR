#pragma once

#include "../Discription/VerilogGraph.h"
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include <utility>
#include <Python.h>//absolute path

using namespace std;


class Evaluate{
public:
    static VerilogGraph graphConstruct(string formalDescripe); 
    //calculate distance between two graphs. timeout: timing limit of each computation process.
    static double graphEditDistance(string formulaGraph1, string formulaGraph2, double timeout);
    //change vector to tuple
    static PyObject* vectorToTuple(const vector<string> &data);
    //getJaccard distance
    static double getJaccard(string filePath1, string filePath2, double adjustRatio);
};