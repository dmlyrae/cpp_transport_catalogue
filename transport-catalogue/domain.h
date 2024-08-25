#pragma once

#include "memory"
#include <set>
#include <sstream>

#include "json.h"
#include "svg.h"

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

        const json::Node* GetNode() const;

    protected:
        std::shared_ptr<json::Node> node_;
    };
}