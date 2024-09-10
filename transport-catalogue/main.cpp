#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "domain.h"
#include "domain_render.h"
#include "domain_responses.h"
#include "domain_requests.h"
#include "domain_transport.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"

using namespace std;

int main() {

    domain::JsonRequests requests(std::cin);
    Transport::Catalogue catalogue = RequestHandler::CreateCatalogue(&requests);
    Render::RoutesMap routes_map = RequestHandler::CreateRoutesMap(&requests);
    domain::JsonResponses responses = RequestHandler::CreateResponses<domain::JsonResponses>(&requests, catalogue, routes_map);
    responses.Print(std::cout);

    return 0;
}