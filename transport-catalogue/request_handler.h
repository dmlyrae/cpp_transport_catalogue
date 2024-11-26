#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "domain.h"
#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace RequestHandler {

    Transport::Catalogue CreateCatalogue(domain::IRequests* requests_ptr);
    
    Render::RoutesMap CreateRoutesMap(domain::IRequests* requests_ptr);

    template<typename T>
    T CreateResponses(const domain::IRequests* request_ptr, Transport::Catalogue catalogue, Render::RoutesMap routes_map) {
        static_assert(std::is_base_of<domain::IStatResponses, T>::value, "T must inherit from IStatResponses");
        T stat_responses;
        request_ptr->FillStatResponses(stat_responses, catalogue, routes_map);
        return stat_responses;
    };

    Transport::RouterA CreateRouterAAA(domain::IRequests* requests_ptr, Transport::Catalogue catalogue);

    // Transport::Router CreateR(domain::IRequests* req_ptr, Transport::Catalogue cat) {
    //     domain::RouterSettings settings = req_ptr->GetRouterSettings();
    //     Transport::Router router = Transport::RouterCreator()
    //         .SetCatalogue(cat)
    //         .SetSettings(settings)
    //         .Build();
    //     return router;
    // }


} // end RequestHandler 