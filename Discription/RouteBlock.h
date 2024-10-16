#include<vector>
#include<string>

using namespace std;

class RouteBlock{
    public:
        string BlockName;
        //portName, wire
        vector<pair<string, string>> inputPortAndWire;
        vector<pair<string, string>> outputPortAndWire;
};