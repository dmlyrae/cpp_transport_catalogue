#pragma once

#include "domain.h"

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