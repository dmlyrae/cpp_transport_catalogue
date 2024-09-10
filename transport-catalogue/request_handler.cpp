#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>

#include "domain.h"
#include "geo.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace RequestHandler {

    Transport::Catalogue CreateCatalogue(domain::IRequests* requests_ptr) {
        return { requests_ptr };
    }

    Render::RoutesMap CreateRoutesMap(domain::IRequests* requests_ptr) {
        return { requests_ptr };
    }

} // end RequestHandler 