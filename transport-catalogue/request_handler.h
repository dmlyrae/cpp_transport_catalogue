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
    
    Render::RoutesMap CreateRoutesMap(domain::IRequests* requests_ptr, Transport::Catalogue catalogue);

    template<typename T>
    T CreateResponses(const domain::IRequests* request_ptr, Transport::Catalogue catalogue, Render::RoutesMap routes_map) {
        static_assert(std::is_base_of<domain::IStatResponses, T>::value, "T must inherit from IStatResponses");
        T stat_responses;
        request_ptr->FillStatResponses(stat_responses, catalogue, routes_map);
        return stat_responses;
    }

} // end RequestHandler 