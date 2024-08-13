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

namespace IO {

struct CommandDescription {

    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
    std::string description;  // Параметры команды
};

class RequestHandler {
public:

    explicit RequestHandler() : 
        requests_(nullptr),
        base_document_(json::Document{nullptr})
    {
        empty_node_ = json::Node{};
    }

    void ParseLine(std::string_view line);

    void ParseJSON(std::istream& input) {
        base_document_ = json::Load(input);
        requests_ = static_cast<domain::Requests*>(&base_document_);
    }

    domain::Requests* GetRequests() {
        return requests_;
    }

    void ApplyBaseCommands(Transport::Catalogue& catalogue);
    void ApplyStatCommands([[maybe_unused]] Transport::Catalogue& catalogue, Render::RoutesMap& routes_map);
    void ApplyRenderSettings(Render::RoutesMap& routes_map) const;

private:
    domain::Requests* requests_;
    json::Document base_document_;
    json::Node empty_node_ = nullptr;

    const json::Node GetRoute(const Transport::Catalogue& catalogue, const json::Dict& request_map) const;
    const json::Node GetStop(const Transport::Catalogue& catalogue, const json::Dict& request_map) const;
    const json::Node GetMap(const Transport::Catalogue& catalogue, const json::Dict& stat_request, Render::RoutesMap& routes_map) const;
    const json::Node GetNotFoundResponse(int request_id) const;
};

} // end IO