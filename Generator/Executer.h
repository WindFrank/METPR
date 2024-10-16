#pragma once

#include "../Discription/Attribute.h"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <unistd.h>

class Executer{
public:
    // generate tb.v and simulation to check if the RTL valid
    static int generateTbandVerify(string mName, vector<Attribute> params, string path="");
};