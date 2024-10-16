#include "Attribute.h"
#include "../Tool.h"

#include <string>
#include <iostream>

using namespace std;

Attribute::Attribute(string name, string type, string datatype){
    this->name = name;
    this->type = type;
    this->dataType = datatype;
    this->start = 0;
    this->end = 0;
}

Attribute::Attribute(string name, string type, int end, int start, string datatype){
    if (start > end){
        Tool::error("Error: The input index of start pin bigger than it of end pin");
        exit(1);
    }
    this->name = name;
    this->type = type;
    this->dataType = datatype;
    this->start = start;
    this->end = end;
}

Attribute::~Attribute(){}

void Attribute::info(){
    if(end == 0)
        cout << "Attribute name: " << name << "  type: " << type << endl;
    else{
        string scope = "[" + to_string(end) + ":" + to_string(start) + "]";
        cout << "Attribute name: " << name <<  "  type: " << type << "  Scope: " << scope << endl;
    }
}

string Attribute::getName(){
    return name;
}

string Attribute::getType(){
    return type;
}

string Attribute::getDataType(){
    return dataType;
}

int Attribute::getStart(){
    return start;
}

int Attribute::getEnd(){
    return end;
}

int Attribute::getIsPublic(){
    return isPublic;
}

int Attribute::getIsconst(){
    return isconst;
}

void Attribute::setStart(int value){
    start = value;
}

void Attribute::setEnd(int value){
    end = value;
}

void Attribute::setType(string value){
    type = value;
}

void Attribute::setIsconst(int value){
    isconst = value;
}

void Attribute::setIsPublic(int boolValue){
    if(boolValue == 0 || boolValue == 1)
        isPublic = boolValue;
    else{
        Tool::error("Error: isPublic is a bool value.");
        exit(1);
    }
}

void Attribute::setDataType(string dataType){
    this->dataType = dataType;
}

void Attribute::setName(string setName)
{
    this->name = setName;
}
