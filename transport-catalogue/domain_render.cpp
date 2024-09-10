#include "domain_render.h"
#include "json.h"

namespace domain {
    /*
    * Сущность "Настройки"
    */
    double Settings::GetWidth() const {
        return node_->AsDict().at("width").AsDouble();
    }; 

    double Settings::GetHeight() const {
        return node_->AsDict().at("height").AsDouble();
    }; 

    double Settings::GetPadding() const {
        return node_->AsDict().at("padding").AsDouble();
    };

    double Settings::GetRadius() const {
        return node_->AsDict().at("stop_radius").AsDouble();
    };

    double Settings::GetLineWidth() const {
        return node_->AsDict().at("line_width").AsDouble();
    };

    int Settings::GetBusLabelFontSize() const {
        return node_->AsDict().at("bus_label_font_size").AsInt();
    };

    const json::Array& Settings::GetBusLabelOffset() const {
        return node_->AsDict().at("bus_label_offset").AsArray();
    };

    int Settings::GetStopLabelFontSize() const {
        return node_->AsDict().at("stop_label_font_size").AsInt();
    };

    const json::Array& Settings::GetStopLabelOffset() const {
        return node_->AsDict().at("stop_label_offset").AsArray();
    };

    svg::Color Settings::GetUnderlayerColor() const {
        const json::Node& undelayer_node = node_->AsDict().at("underlayer_color");
        if (undelayer_node.IsString()) {
            return undelayer_node.AsString();
        } else if (undelayer_node.IsArray()) {
            const json::Array& underlayer_color = undelayer_node.AsArray();
            if (underlayer_color.size() == 3) {
                return svg::Rgb(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt());
            } else if (underlayer_color.size() == 4) {
                return svg::Rgba(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt(), underlayer_color[3].AsDouble());
            } 
        }
        throw std::logic_error("Invalid");
    }

    std::vector<svg::Color> Settings::GetPalette() const {
        const json::Array& color_palette = node_->AsDict().at("color_palette").AsArray();
        std::vector<svg::Color> colors;
        for (const auto& color_element : color_palette) {
            if (color_element.IsString()) {
                colors.push_back(color_element.AsString());
            } else if (color_element.IsArray()) {
                const json::Array& color_type = color_element.AsArray();
                if (color_type.size() == 3) {
                    svg::Rgb rgb(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt());
                    colors.emplace_back(rgb);
                } else if (color_type.size() == 4) {
                    svg::Rgba rgba(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt(), color_type[3].AsDouble());
                    colors.emplace_back(rgba);
                } else {
                    throw std::logic_error("wrong color_palette type");
                }
            } else {
                throw std::logic_error("wrong color_palette");
            }
        }
        return colors;
    }

    double Settings::GetUnderlayerWidth() const {
        return node_->AsDict().at("underlayer_width").AsDouble();
    }

}