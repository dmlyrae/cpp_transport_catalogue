#pragma once

#include "memory"
#include <set>
#include <sstream>
#include "json.h"
#include "svg.h"
#include "set"
#include "json_builder.h"

namespace Render {

struct RenderSettings {
	double width = 0.0;
	double height = 0.0;
	double padding = 0.0;
	double line_width = 0.0;
	double stop_radius = 0.0;
	int bus_label_font_size = 0;
	svg::Point bus_label_offset = { 0.0, 0.0 };
	int stop_label_font_size = 0;
	svg::Point stop_label_offset = { 0.0, 0.0 };
	svg::Color underlayer_color = { svg::NoneColor };
	double underlayer_width = 0.0;
	std::vector<svg::Color> color_palette {};
};

class RoutesMap;

class SphereProjector;

}

namespace Transport {
    class Stop;
    class Bus;
    class Catalogue;

    struct RouterSettings {
        int bus_wait_time;
        int bus_velocity;
    };
}

namespace domain {

    enum class EntityType { Stop, Bus, Map };

    /* Общий класс для всех сущностей и запросов (карта, автобус, остановка). */
    class BaseEntity {
    public:

        [[nodiscard]] BaseEntity(const json::Node& node);

        std::string GetType() const;

        bool IsStop() const;

        bool IsBus() const;

        bool IsMap() const;

        bool IsRoute() const;

        const json::Node* GetNode() const;

    protected:
        std::shared_ptr<json::Node> node_;
    };
}

namespace domain {

    /* Общая сущность для остановки и автобуса. */
    class Entity : public BaseEntity {
    public:
        using BaseEntity::BaseEntity;

        const std::string& GetName() const;
    };

    /* Сущность "Автобус" */
    class BusEntity : public Entity {
    public:
        using Entity::Entity;

        const json::Array& GetStops() const;
        bool IsRoundtrip() const;
    };

    /* Сущность "Остановка" */
    class StopEntity : public Entity {
    public:
        using Entity::Entity;

        const json::Dict& GetDistances() const;
        double GetLongitude() const;
        double GetLatitude() const;
    };

    /* Настройка маршрутов */
    class RouterSettings : public BaseEntity {
    public:
        using BaseEntity::BaseEntity;
        int GetBusWaitTime() const; 
        int GetBusVelocity() const; 
    };

    /* Действие пассажира */
    class PassengerAction : public BaseEntity {
    public:
        using BaseEntity::BaseEntity;
    };

}

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

namespace domain {
    /* Настройка карты */
    class Settings : public BaseEntity {
    public:
        using BaseEntity::BaseEntity;
        double GetWidth() const; 
        double GetHeight() const; 
        double GetPadding() const; 
        double GetRadius() const; 
        double GetLineWidth() const; 
        int GetBusLabelFontSize() const; 
        const json::Array& GetBusLabelOffset() const;
        int GetStopLabelFontSize() const; 
        const json::Array& GetStopLabelOffset() const;
        svg::Color GetUnderlayerColor() const;
        std::vector<svg::Color> GetPalette() const;
        double GetUnderlayerWidth() const; 
    };
}

namespace Transport {
    class Stop;
    class Bus;
    class Catalogue;
    class RouterA;
}

namespace domain {

    /* Сущность "Запрос" */
    class Stat : public BaseEntity {
    public:
        using BaseEntity::BaseEntity;
        int GetRequestId() const;
        std::string GetName() const;
    };

    /* Интерфейс класса запросов */
    class IRequests {
    public:
        virtual void FillTransportCatalogue(Transport::Catalogue& catalogue) const = 0;
        virtual void FillRenderSettings(Render::RoutesMap& routes_map) const = 0;
        virtual RouterSettings GetRouterSettings() const = 0;
        virtual void FillStatResponses(
            domain::IStatResponses& responses, 
            const Transport::Catalogue& catalogue,
            const Render::RoutesMap& routes_map,
            const Transport::RouterA& router
        ) const = 0;
        virtual ~IRequests() = default;
    };

    /* Класс запросов через JSON */
    class JsonRequests : public IRequests  {
    public:
        explicit JsonRequests(std::istream& input);

        std::pair<std::vector<BusEntity>, std::vector<StopEntity>> GetBase() const;
        std::vector<Stat> GetStats() const;
        Settings GetRenderSettings() const;
        RouterSettings GetRouterSettings() const;

        void FillTransportCatalogue(Transport::Catalogue& catalogue) const override;
        void FillRenderSettings(Render::RoutesMap& routes_map) const override;
        void FillStatResponses(
            domain::IStatResponses& responses, 
            const Transport::Catalogue& catalogue,
            const Render::RoutesMap& routes_map,
            const Transport::RouterA& router
        ) const override;

    private:
        json::Document base_document_;
    };
}
