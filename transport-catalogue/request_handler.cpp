#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>

#include "domain.h"
#include "geo.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace RequestHandler {

    Transport::Catalogue CreateCatalogue(domain::IRequests* requests_ptr) {
        return { requests_ptr };
    }

    Render::RoutesMap CreateRoutesMap(domain::IRequests* requests_ptr) {
        return { requests_ptr };
    }

    Transport::RouterA CreateRouter(domain::IRequests* requests_ptr, Transport::Catalogue catalogue) {
        domain::RouterSettings settings = requests_ptr->GetRouterSettings();
        Transport::RouterA router = Transport::RouterCreator()
            .SetCatalogue(catalogue)
            .SetSettings(settings)
            .Build();
        return router;
    }

} // end RequestHandler 