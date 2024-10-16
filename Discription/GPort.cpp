#include "GPort.h"

GPort::GPort(string type, string portName, int index){
    this->type = type;
    this->portName = portName;
    this->pinIndex = index;
}

string GPort::getPortExp(){
    return portName + "[" + to_string(pinIndex) + "]";
}