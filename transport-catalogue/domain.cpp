#include <stddef.h>
#include <set>
#include <sstream>
#include "memory"
#include "geo.h"
#include "map_renderer.h"
#include "svg.h"
#include <sstream>
#include <set>
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace domain {

    /*
    * Общие методы сущностей
    */
    BaseEntity::BaseEntity(const json::Node& node) {
        node_ = std::make_shared<json::Node>(node);
    }

    std::string BaseEntity::GetType() const {
        return node_->AsDict().at("type").AsString();
    }

    bool BaseEntity::IsStop() const {
        return node_->AsDict().at("type").AsString() == "Stop";
    }

    bool BaseEntity::IsBus() const {
        return node_->AsDict().at("type").AsString() == "Bus";
    }

    bool BaseEntity::IsRoute() const {
        return node_->AsDict().at("type").AsString() == "Route";
    }

    bool BaseEntity::IsMap() const {
        return node_->AsDict().at("type").AsString() == "Map";
    }

    const json::Node* BaseEntity::GetNode() const {
        return node_.get();
    }

    const std::string& Entity::GetName() const { 
        return node_->AsDict().at("name").AsString(); 
    }

} // end domain


namespace domain {

    /*
    * Сущность "Автобус"
    */
    const json::Array& BusEntity::GetStops() const {
        return node_->AsDict().at("stops").AsArray(); 
    } 

    bool BusEntity::IsRoundtrip() const {
        return node_->AsDict().at("is_roundtrip").AsBool(); 
    }

    /*
    * Сущность "Остановка"
    */
    const json::Dict& StopEntity::GetDistances() const { 
        return node_->AsDict().at("road_distances").AsDict(); 
    }

    double StopEntity::GetLongitude() const { 
        return node_->AsDict().at("longitude").AsDouble(); 
    }

    double StopEntity::GetLatitude() const { 
        return node_->AsDict().at("latitude").AsDouble(); 
    }

    /*
    * Сущность "Настройка маршрутов"
    */
    int RouterSettings::GetBusWaitTime() const { 
        return node_->AsDict().at("bus_wait_time").AsInt(); 
    }

    int RouterSettings::GetBusVelocity() const { 
        return node_->AsDict().at("bus_velocity").AsInt(); 
    }

}

namespace domain {
    /* 
    * Класс ответов через JSON 
    */

   JsonResponses::JsonResponses () : responses_({}) {};

    void JsonResponses::Print(std::ostream& out) const {
        json::Print(
            json::Document(
                json::Builder{}
                .Value(responses_)
                .Build()
            ),
            out
        );
    }

    void JsonResponses::PushBusResponse(
        int request_id,
        double curvature, 
        int route_length,
        int stop_count,
        int unique_stop_count
    ) {
        responses_.push_back(
            json::Builder{}
                .StartDict()
                    .Key("request_id").Value(request_id)
                    .Key("curvature").Value(curvature)
                    .Key("route_length").Value(route_length)
                    .Key("stop_count").Value(stop_count)
                    .Key("unique_stop_count").Value(unique_stop_count)
                .EndDict()
                .Build()
        );
    };

    void JsonResponses::PushStopResponse(
        int request_id,
        const std::set<std::string_view>& bus_names 
    ) {
        json::Builder buses_builder;
        auto buses = buses_builder.StartArray();

        for (auto& bus : bus_names) {
            buses.Value(std::string(bus));
        }

        responses_.push_back(
                json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(request_id)
                        .Key("buses").Value(
                                buses
                                    .EndArray()
                                    .Build()
                                    .GetValue()
                            )
                    .EndDict()
                    .Build()
            );
    };

    void JsonResponses::PushMapResponse(
        int request_id,
        const svg::Document& svg
    ) {
        std::ostringstream strm;
        svg.Render(strm);
        responses_.push_back(
            json::Builder{}
                .StartDict()
                    .Key("map")
                    .Value(strm.str())
                    .Key("request_id")
                    .Value(request_id)
                .EndDict()
                .Build()
            );
    };

    void JsonResponses::PushNotFoundResponse(int request_id) {
        json::Dict result;
        result["request_id"] = request_id;
        result["error_message"] = "not found";
        responses_.push_back(
            json::Builder{}
                .StartDict()
                    .Key("request_id")
                    .Value(request_id)
                    .Key("error_message")
                    .Value("not found")
                .EndDict()
                .Build()

        );
    }

    void JsonResponses::PushRouteResponse(
        int request_id,
        double total_time,
        json::Array items
    ) {
        json::Dict result;
        result["request_id"] = request_id;
        responses_.push_back(
            json::Builder{}
                .StartDict()
                    .Key("request_id")
                    .Value(request_id)
                    .Key("total_time")
                    .Value(total_time)
                    .Key("items")
                    .Value(items)
                .EndDict()
                .Build()
        );
    }
};

namespace domain {
    /*
    * Сущность "Настройки"
    */
    double Settings::GetWidth() const {
        return node_->AsDict().at("width").AsDouble();
    }; 

    double Settings::GetHeight() const {
        return node_->AsDict().at("height").AsDouble();
    }; 

    double Settings::GetPadding() const {
        return node_->AsDict().at("padding").AsDouble();
    };

    double Settings::GetRadius() const {
        return node_->AsDict().at("stop_radius").AsDouble();
    };

    double Settings::GetLineWidth() const {
        return node_->AsDict().at("line_width").AsDouble();
    };

    int Settings::GetBusLabelFontSize() const {
        return node_->AsDict().at("bus_label_font_size").AsInt();
    };

    const json::Array& Settings::GetBusLabelOffset() const {
        return node_->AsDict().at("bus_label_offset").AsArray();
    };

    int Settings::GetStopLabelFontSize() const {
        return node_->AsDict().at("stop_label_font_size").AsInt();
    };

    const json::Array& Settings::GetStopLabelOffset() const {
        return node_->AsDict().at("stop_label_offset").AsArray();
    };

    svg::Color Settings::GetUnderlayerColor() const {
        const json::Node& undelayer_node = node_->AsDict().at("underlayer_color");
        if (undelayer_node.IsString()) {
            return undelayer_node.AsString();
        } else if (undelayer_node.IsArray()) {
            const json::Array& underlayer_color = undelayer_node.AsArray();
            if (underlayer_color.size() == 3) {
                return svg::Rgb(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt());
            } else if (underlayer_color.size() == 4) {
                return svg::Rgba(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt(), underlayer_color[3].AsDouble());
            } 
        }
        throw std::logic_error("Invalid");
    }

    std::vector<svg::Color> Settings::GetPalette() const {
        const json::Array& color_palette = node_->AsDict().at("color_palette").AsArray();
        std::vector<svg::Color> colors;
        for (const auto& color_element : color_palette) {
            if (color_element.IsString()) {
                colors.push_back(color_element.AsString());
            } else if (color_element.IsArray()) {
                const json::Array& color_type = color_element.AsArray();
                if (color_type.size() == 3) {
                    svg::Rgb rgb(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt());
                    colors.emplace_back(rgb);
                } else if (color_type.size() == 4) {
                    svg::Rgba rgba(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt(), color_type[3].AsDouble());
                    colors.emplace_back(rgba);
                } else {
                    throw std::logic_error("wrong color_palette type");
                }
            } else {
                throw std::logic_error("wrong color_palette");
            }
        }
        return colors;
    }

    double Settings::GetUnderlayerWidth() const {
        return node_->AsDict().at("underlayer_width").AsDouble();
    }

}

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
        const Render::RoutesMap& routes_map,
        const Transport::RouterA& router
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
            } else if (type == "Route") {
                const std::string from_stop = request.GetNode()->AsDict().at("from").AsString();
                const std::string to_stop = request.GetNode()->AsDict().at("to").AsString();
                std::optional<graph::Router<double>::RouteInfo> route = router.FindRoute(from_stop, to_stop);
                if (!route) {
                    responses.PushNotFoundResponse(request_id);
                    continue;
                }
                const std::pair<std::vector<domain::PassengerAction>, double> route_info = router.GetRoute(route.value());
                json::Array items;
                for(auto item : route_info.first) {
                    items.push_back(*item.GetNode());
                }
                responses.PushRouteResponse(
                    request.GetRequestId(),
                    route_info.second,
                    items
                );
                continue;
            }
            responses.PushNotFoundResponse(request_id);
        }
    }

}