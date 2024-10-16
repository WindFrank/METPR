#include "RouteUpdate.h"

bool RouteUpdate::updateRoute(vector<RouteInfo*> vRoute, RouteInfo *critRoad, RouteInfo *changeRoad)
{
    bool critFound = false;
    bool changeFound = false;
    for(auto it = vRoute.begin(); it < vRoute.end() && !(critFound && changeFound); it++){
        RouteInfo* recentRoute = *it;
        if(!critFound && recentRoute->netName == critRoad->netName)
            (*it) = new RouteInfo(critRoad);
        else if(!changeFound && recentRoute->netName == changeRoad->netName)
            (*it) = new RouteInfo(changeRoad);
    }
    return critFound && changeFound;
}
