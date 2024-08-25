#pragma once

#include "domain_responses.h"
#include "domain_transport.h"
#include "domain_render.h"
#include "json.h"

namespace Transport {
    class Stop;
    class Bus;
    class Catalogue;
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
        virtual void FillStatResponses(
            domain::IStatResponses& responses, 
            const Transport::Catalogue& catalogue,
            const Render::RoutesMap& routes_map
        ) const = 0;
        virtual ~IRequests() = default;
    };

    /* Класс запросов через JSON */
    class JsonRequests : public IRequests  {
    public:
        explicit JsonRequests (std::istream& input);

        std::pair<std::vector<BusEntity>, std::vector<StopEntity>> GetBase() const;
        std::vector<Stat> GetStats() const;
        Settings GetRenderSettings() const;

        void FillTransportCatalogue(Transport::Catalogue& catalogue) const override;
        void FillRenderSettings(Render::RoutesMap& routes_map) const override;
        void FillStatResponses(
            domain::IStatResponses& responses, 
            const Transport::Catalogue& catalogue,
            const Render::RoutesMap& routes_map
        ) const override;

    private:
        json::Document base_document_;
    };

}