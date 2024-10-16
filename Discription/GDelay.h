#include <string>
#include <iostream>

using namespace std;

class GDelay{
private:
    string preType;
    double delay;
    string preMode;
    string firstPin;
    string endPin;

public:
    GDelay(string preType, string preMode, string firstPin, string endPin, double delay);
    string getPreType();
    double getValue();
    string getFirstPin();
    string getEndPin();
    string getPreMode();

    bool isMatch(string preType, string preMode, string firstPin, string endPin);
};