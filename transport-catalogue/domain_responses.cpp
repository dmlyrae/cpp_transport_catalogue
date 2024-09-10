#include <sstream>
#include <set>

#include "domain_responses.h"
#include "json.h"
#include "json_builder.h"
#include "svg.h"

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

}