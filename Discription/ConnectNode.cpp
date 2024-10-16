#include "ConnectNode.h"

ConnectNode::ConnectNode(){}

string ConnectNode::getIndexPin(){
    return nextPort->getPortExp();
}

double ConnectNode::getDelay(string firstEnd, string secondEnd)
{
    return delay;
}

void ConnectNode::setDelay(double delay)
{
    this->delay = delay;
}

void ConnectNode::setId(string s){
    id = s;
}

string ConnectNode::getInstance()
{
    return nextInstance;
}

GNode *ConnectNode::getNextBlockNode()
{
    return nextNode;
}

string ConnectNode::getStartPort()
{
    return startPort->getPortExp();
}

string ConnectNode::getID()
{
    return id;
}

string ConnectNode::getNextPort()
{
    return nextPort->getPortExp();
}

string ConnectNode::getStartType()
{
    return startPort->type;
}

string ConnectNode::getNextType()
{
    return nextPort->type;
}
