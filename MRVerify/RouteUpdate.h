/*
    RouteFileUpdate: update the .route file by RouteInfo
*/

#include "../Discription/RouteInfo.h"

class RouteUpdate{
public:
    static bool updateRoute(vector<RouteInfo*> vRoute, RouteInfo* critRoad, RouteInfo* changeRoad);
};