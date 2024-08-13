#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>

#include "request_handler.h"
#include "geo.h"
#include "map_renderer.h"

namespace IO {

using namespace std::literals;

namespace detail {
    const std::string DistanceDelimeter = " to ";
    const std::string DescriptionDelimeter = ",";

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Переводит две подстроки в числа
 */
Geo::Coordinates ParseCoordinates(std::string_view lat_string, std::string_view lng_string) {
    double lat = std::stod(std::string(Trim(lat_string)));
    double lng = std::stod(std::string(Trim(lng_string)));
    return { lat, lng };
}

/**
 * Разбивает строку string на n строк, с помощью указанной строки
 */
std::vector<std::string_view> Split(std::string_view string, std::string_view delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    size_t start_pos = 0;

    while ((pos = string.find(delim, start_pos)) != std::string::npos) {
        result.push_back(Trim(string.substr(start_pos, pos - start_pos)));
        start_pos = pos + delim.length();
    }

    if (start_pos < string.length()) {
        result.push_back(Trim(string.substr(start_pos)));
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::pair<std::vector<std::string_view>, Transport::RouteType> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return {Split(route, ">"), Transport::RouteType::Line};
    }

    auto stops = Split(route, "-");
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return { results, Transport::RouteType::Ring };
}
} // end detail

const json::Node RequestHandler::GetNotFoundResponse(int request_id) const {
    json::Dict response;
    response.emplace("request_id", request_id);
    std::string msg = "not found";
    response.emplace("error_message", msg);
    return { response };
}

void RequestHandler::ApplyStatCommands([[maybe_unused]] Transport::Catalogue& catalogue, Render::RoutesMap& routes_map) {
    using namespace std::literals;
    json::Array result;
    std::vector<domain::Stat> stat_requests = GetRequests()->GetStats();
    for (const domain::Stat& request : stat_requests) {
        const std::string& type = request.GetType();
        if (type == "Stop") {
            result.push_back(GetStop(catalogue, request.GetNode()->AsMap()).AsMap());
        } else if (type == "Bus") {
            result.push_back(GetRoute(catalogue, request.GetNode()->AsMap()).AsMap());
        } else if (type == "Map") {
            result.push_back(GetMap(catalogue, request.GetNode()->AsMap(), routes_map).AsMap());
        }
    }

    json::Print(json::Document{ result }, std::cout);
}

void RequestHandler::ApplyBaseCommands([[maybe_unused]] Transport::Catalogue& catalogue) {
    using namespace std::literals;

    std::pair<std::vector<domain::BusEntity>, std::vector<domain::StopEntity>> base_requests = GetRequests()->GetBase();
    std::vector<std::tuple<std::string, std::string, size_t>> parsed_distances;

    /*
    * Создание остановок
    */
    for(const domain::StopEntity& request : base_requests.second) {
        std::string stop_name = request.GetName();
        Geo::Coordinates coordinates = { request.GetLatitude(), request.GetLongitude() };
        std::shared_ptr<Transport::Stop> stop_ptr = std::make_shared<Transport::Stop>(stop_name, coordinates);
        json::Dict distances = request.GetDistances();
        for (auto& [ adjacent_stop_name, node_distance ] : distances) {
            size_t distance = static_cast<size_t>(node_distance.AsInt());
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

const json::Node RequestHandler::GetRoute(const Transport::Catalogue& catalogue, const json::Dict& stat_request) const {
    json::Dict result;
    const std::string& route_number = stat_request.at("name").AsString();
    result["request_id"] = stat_request.at("id").AsInt();
    if (!catalogue.GetBus(route_number)) {
        return GetNotFoundResponse(stat_request.at("id").AsInt());
    } else {
        std::shared_ptr<Transport::Bus> bus_ptr = catalogue.GetBus(route_number);
        result["curvature"] = bus_ptr->GetCurvature();
        result["route_length"] = static_cast<int>(bus_ptr->GetRouteLength());
        result["stop_count"] = static_cast<int>(bus_ptr->GetRouteSize());
        result["unique_stop_count"] = static_cast<int>(bus_ptr->GetUniqueStopsSize());
    }
    return json::Node{ result };
}

const json::Node RequestHandler::GetMap(const Transport::Catalogue& catalogue, const json::Dict& stat_request, Render::RoutesMap& routes_map) const {
    json::Dict result;

    result["request_id"] = stat_request.at("id").AsInt();
    std::ostringstream strm;
    svg::Document result_svg;
    routes_map.FillSVG(result_svg, catalogue);
    result_svg.Render(strm);
    result["map"] = strm.str();

    return json::Node{ result };
}

const json::Node RequestHandler::GetStop(const Transport::Catalogue& catalogue, const json::Dict& stop_request) const {
    json::Dict result;
    const std::string& stop_name = stop_request.at("name").AsString();
    result["request_id"] = stop_request.at("id").AsInt();
    if (!catalogue.GetStop(stop_name)) {
        return GetNotFoundResponse(stop_request.at("id").AsInt());
    } else {
        std::shared_ptr<Transport::Stop> stop_ptr = catalogue.GetStop(stop_name);
        json::Array buses;
        for (auto& bus : stop_ptr->GetBusNames()) {
            buses.push_back(std::string(bus));
        }
        result["buses"] = buses;
    }
    return json::Node{ result };
}

void RequestHandler::ApplyRenderSettings(Render::RoutesMap& routes_map) const {
    domain::Settings render_settings = requests_->GetRenderSettings();
    routes_map.AppplySettings(render_settings);
};

} // end IO