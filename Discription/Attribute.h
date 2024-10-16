/*
Attribute: A variable with its type, name and more information.
*/
#pragma once

#include <string>

#include "../Boost_recent.h"

using namespace std;

class Attribute{
private:
    string name;
    string type;
    //reg or wire
    string dataType;
    int start;
    int end;
    //If the parameter is visible for outer space.
    int isPublic = 0;
    int isconst = -1;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & name;
        ar & type;
        ar & dataType;
        ar & start;
        ar & end;
        ar & isPublic;
        ar & isconst;
    }
public:
    bool isBus = false;
    Attribute(){};
    Attribute(string name, string type, string datatype = "wire");
    Attribute(string name, string type, int end, int start, string datatype = "wire");
    ~Attribute();
    void info();
    string getName();
    string getType();
    string getDataType();
    int getStart();
    int getEnd();
    int getIsPublic();
    int getIsconst();
    void setStart(int value);
    void setEnd(int value);
    void setType(string value);
    void setIsPublic(int boolValue);
    void setDataType(string dataType);
    void setName(string setName);
    void setIsconst(int value);
};