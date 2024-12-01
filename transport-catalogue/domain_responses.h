#pragma once
#include "set"

#include "json.h"
#include "json_builder.h"
#include "svg.h"

namespace domain {


    /* Интерфейс класса oтветов */
    class IStatResponses {
    public:
        virtual void Print(std::ostream& out) const = 0;

        virtual void PushBusResponse(
            int request_id,
            double curvature, 
            int route_length,
            int stop_count,
            int unique_stop_count) = 0;

        virtual void PushStopResponse(
            int request_id,
            const std::set<std::string_view>& buses 
        ) = 0;

        virtual void PushMapResponse(
            int request_id,
            const svg::Document& svg
        ) = 0;

        virtual void PushRouteResponse(
            int request_id,
            double total_time,
            json::Array items
        ) = 0;

        virtual void PushNotFoundResponse(int request_id) = 0;

        virtual ~IStatResponses() = default;
    };

    /* Класс ответов через JSON */
    class JsonResponses : public IStatResponses  {
    public:

        explicit JsonResponses ();

        void Print(std::ostream& out) const override;

        void PushBusResponse(
            int request_id,
            double curvature, 
            int route_length,
            int stop_count,
            int unique_stop_count
        ) override;

        void PushStopResponse(
            int requestId,
            const std::set<std::string_view>& bus_names 
        ) override;

        void PushMapResponse(
            int request_id,
            const svg::Document& svg
        ) override;

        void PushRouteResponse(
            int request_id,
            double total_time,
            json::Array items
        ) override;

        void PushNotFoundResponse(int request_id) override;

    private:
        json::Array responses_;
    };

}