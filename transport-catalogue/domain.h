#pragma once
#include "memory"

#include "json.h"
#include "svg.h"

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

        const json::Node* GetNode() const;

    protected:
        std::shared_ptr<json::Node> node_;
    };
  
    /* Запрос информации */
    class Stat : public BaseEntity {
    public:
        using BaseEntity::BaseEntity;
    };

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

    /* Общий класс запросов */
    class Requests : public json::Document {
    public:
        std::pair<std::vector<BusEntity>, std::vector<StopEntity>> GetBase() const;
        std::vector<Stat> GetStats() const;
        Settings GetRenderSettings() const;
    };
}