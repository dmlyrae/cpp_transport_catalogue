#pragma once

#include "domain.h"
#include "json.h"

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

}