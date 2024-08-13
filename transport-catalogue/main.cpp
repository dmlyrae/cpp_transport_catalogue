#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <sstream>

#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"

using namespace std;

int main() {

    Transport::Catalogue catalogue;
    IO::RequestHandler request_handler;
    request_handler.ParseJSON(std::cin);
    request_handler.ApplyBaseCommands(catalogue);
    Render::RoutesMap routes_map;
    request_handler.ApplyRenderSettings(routes_map);
    request_handler.ApplyStatCommands(catalogue, routes_map);

    return 0;
}