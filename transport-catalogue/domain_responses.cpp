#include <sstream>
#include <set>

#include "domain_responses.h"
#include "json.h"
#include "svg.h"

namespace domain {
    /* 
    * Класс ответов через JSON 
    */

    JsonResponses::JsonResponses () : responses_({}) {};

    void JsonResponses::Print(std::ostream& out) const {
        json::Print(json::Document{ responses_ }, out);
    }

    void JsonResponses::PushBusResponse(
        int request_id,
        double curvature, 
        int route_length,
        int stop_count,
        int unique_stop_count
    ) {
        json::Dict result;
        result["request_id"] = request_id;
        result["curvature"] = curvature;
        result["route_length"] = route_length;
        result["stop_count"] = stop_count;
        result["unique_stop_count"] = unique_stop_count;
        responses_.push_back(result);
    };

    void JsonResponses::PushStopResponse(
        int request_id,
        const std::set<std::string_view>& bus_names 
    ) {
        json::Dict result;
        json::Array buses;
        result["request_id"] = request_id;
        for (auto& bus : bus_names) {
            buses.push_back(std::string(bus));
        }
        result["buses"] = buses;
        responses_.push_back(result);
    };

    void JsonResponses::PushMapResponse(
        int request_id,
        const svg::Document& svg
    ) {
        json::Dict result;
        std::ostringstream strm;
        svg.Render(strm);
        result["map"] = strm.str();
        result["request_id"] = request_id;
        responses_.push_back(result);
    };

    void JsonResponses::PushNotFoundResponse(int request_id) {
        json::Dict result;
        result["request_id"] = request_id;
        result["error_message"] = "not found";
        responses_.push_back(result);
    }



}