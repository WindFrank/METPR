#include "GDelay.h"

GDelay::GDelay(string preType, string preMode, string firstPin, string endPin, double delay){
    this->preType = preType;
    this->delay = delay;
    this->preMode = preMode;
    this->firstPin = firstPin;
    this->endPin = endPin;
}

string GDelay::getPreType()
{
    return preType;
}

double GDelay::getValue(){
    return delay;
}

string GDelay::getFirstPin()
{
    return firstPin;
}

string GDelay::getEndPin()
{
    return endPin;
}

string GDelay::getPreMode()
{
    return preMode;
}

bool GDelay::isMatch(string preType, string preMode, string firstPin, string endPin){
    return (this->preType == preType) && (this->preMode == preMode) && (this->firstPin == firstPin) && (this->endPin == endPin);
}