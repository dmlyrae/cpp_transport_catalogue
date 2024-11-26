#include "memory"

#include "domain_responses.h"
#include "domain_requests.h"
#include "domain_render.h"
#include "domain_transport.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace domain {

    /*
    * Сущность "Запрос"
    */
    int Stat::GetRequestId() const {
        return node_->AsDict().at("id").AsInt();
    }

    std::string Stat::GetName() const {
        if (GetType() == "Map") {
            throw std::logic_error("Not map");
        }
        return node_->AsDict().at("name").AsString();
    }

    /*
    * Класс запросов через JSON
    */
    JsonRequests::JsonRequests (std::istream& input) : 
        base_document_(json::Load(input)) {}

    std::pair<std::vector<BusEntity>, std::vector<StopEntity>> JsonRequests::GetBase() const {
        std::pair<std::vector<BusEntity>, std::vector<StopEntity>> base;
        for(const json::Node& node : base_document_.GetRoot().AsDict().at("base_requests").AsArray()) {
            if (node.AsDict().at("type").AsString() == "Stop") {
                base.second.emplace_back(node);
            } else {
                base.first.emplace_back(node);
            }
        }
        return base;
    }

    std::vector<Stat> JsonRequests::GetStats() const {
        std::vector<Stat> stat_requests;  
        for(const json::Node& node : base_document_.GetRoot().AsDict().at("stat_requests").AsArray()) {
            stat_requests.emplace_back(node);
        }
        return stat_requests;
    };

    Settings JsonRequests::GetRenderSettings() const {
        return base_document_.GetRoot().AsDict().at("render_settings");
    };

    domain::RouterSettings JsonRequests::GetRouterSettings() const {
        return base_document_.GetRoot()
            .AsDict()
            .at("routing_settings");
    };

    void JsonRequests::FillTransportCatalogue(Transport::Catalogue& catalogue) const {
        using namespace std::literals;

        std::pair<std::vector<domain::BusEntity>, std::vector<domain::StopEntity>> base_requests = GetBase();
        std::vector<std::tuple<std::string, std::string, std::size_t>> parsed_distances;

        /*
        * Создание остановок
        */
        for(const domain::StopEntity& request : base_requests.second) {
            std::string stop_name = request.GetName();
            Geo::Coordinates coordinates = { request.GetLatitude(), request.GetLongitude() };
            std::shared_ptr<Transport::Stop> stop_ptr = std::make_shared<Transport::Stop>(stop_name, coordinates);
            json::Dict distances = request.GetDistances();
            for (auto& [ adjacent_stop_name, node_distance ] : distances) {
                std::size_t distance = static_cast<int>(node_distance.AsInt());
                stop_ptr->AddAdjacent( adjacent_stop_name, distance );
                parsed_distances.emplace_back(stop_name, adjacent_stop_name, distance);
            }
            catalogue.AddStop(stop_ptr);
        }

        /**
         * Создание сегментов дорожной сети
        */
        for(auto distance : parsed_distances) {
            catalogue.SetDistance(std::get<0>(distance), std::get<1>(distance), std::get<2>(distance));
        }

        /**
         * Создание автобусов
        */
        for(const domain::BusEntity& bus: base_requests.first) {
            Transport::RouteType route_type = bus.IsRoundtrip() ? Transport::RouteType::Ring : Transport::RouteType::Line; 
            std::shared_ptr<Transport::Bus> bus_ptr = std::make_shared<Transport::Bus>(bus.GetName(), route_type, catalogue);
            for (const json::Node& stop_name : bus.GetStops()) {
                std::shared_ptr<Transport::Stop> stop = catalogue.GetStop(stop_name.AsString());
                bus_ptr->AddStop(stop);
            }
            catalogue.AddBus(bus_ptr);
        }
    }

    void JsonRequests::FillRenderSettings(Render::RoutesMap& routes_map) const {
        domain::Settings render_settings = GetRenderSettings();
        routes_map.AppplySettings(render_settings);
    };

    void JsonRequests::FillStatResponses(
        domain::IStatResponses& responses, 
        const Transport::Catalogue& catalogue,
        const Render::RoutesMap& routes_map
    ) const {
        using namespace std::literals;
        std::vector<domain::Stat> stat_requests = GetStats();
        for (const domain::Stat& request : stat_requests) {
            const std::string& type = request.GetType();
            int request_id = request.GetRequestId();
            if (type == "Stop") {
                std::string name = request.GetName();
                const std::shared_ptr<Transport::Stop> stop = catalogue.GetStop(name);
                if (stop) {
                    const std::set<std::string_view>& buses = stop->GetBusNames();
                    responses.PushStopResponse(
                        request.GetRequestId(),
                        buses
                    );
                    continue;
                } 
            } else if (type == "Bus") {
                const std::shared_ptr<Transport::Bus> bus = catalogue.GetBus(request.GetName());
                if (bus) {
                    responses.PushBusResponse(
                        request.GetRequestId(),
                        bus->GetCurvature(),
                        bus->GetRouteLength(),
                        bus->GetRouteSize(),
                        bus->GetUniqueStopsSize()
                    );
                    continue;
                } 
            } else if (type == "Map") {
                svg::Document result_svg;
                routes_map.FillSVG(result_svg, catalogue);
                responses.PushMapResponse(
                    request_id,
                    result_svg
                );
                continue;
            }
            responses.PushNotFoundResponse(request_id);
        }
    }

}