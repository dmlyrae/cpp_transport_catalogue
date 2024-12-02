#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "domain.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

using namespace std;

int main() {

    domain::JsonRequests requests(std::cin);
    Transport::Catalogue catalogue = RequestHandler::CreateCatalogue(&requests);
    Render::RoutesMap routes_map = RequestHandler::CreateRoutesMap(&requests);
    Transport::RouterA router = RequestHandler::CreateRouter(&requests, catalogue);
    domain::JsonResponses responses = RequestHandler::CreateResponses<domain::JsonResponses>(&requests, catalogue, routes_map, router);
    responses.Print(std::cout);

    return 0;
}