#include "domain.h"
#include "svg.h"

namespace domain {

    /*
    * Общие методы сущностей
    */
    BaseEntity::BaseEntity(const json::Node& node) {
        node_ = std::make_shared<json::Node>(node);
    }

    std::string BaseEntity::GetType() const {
        return node_->AsMap().at("type").AsString();
    }

    bool BaseEntity::IsStop() const {
        return node_->AsMap().at("type").AsString() == "Stop";
    }

    bool BaseEntity::IsBus() const {
        return node_->AsMap().at("type").AsString() == "Bus";
    }

    bool BaseEntity::IsMap() const {
        return node_->AsMap().at("type").AsString() == "Map";
    }

    const json::Node* BaseEntity::GetNode() const {
        return node_.get();
    }

    const std::string& Entity::GetName() const { 
        return node_->AsMap().at("name").AsString(); 
    }

    /*
    * Сущность "Настройки"
    */
    double Settings::GetWidth() const {
        return node_->AsMap().at("width").AsDouble();
    }; 

    double Settings::GetHeight() const {
        return node_->AsMap().at("height").AsDouble();
    }; 

    double Settings::GetPadding() const {
        return node_->AsMap().at("padding").AsDouble();
    };

    double Settings::GetRadius() const {
        return node_->AsMap().at("stop_radius").AsDouble();
    };

    double Settings::GetLineWidth() const {
        return node_->AsMap().at("line_width").AsDouble();
    };

    int Settings::GetBusLabelFontSize() const {
        return node_->AsMap().at("bus_label_font_size").AsInt();
    };

    const json::Array& Settings::GetBusLabelOffset() const {
        return node_->AsMap().at("bus_label_offset").AsArray();
    };

    int Settings::GetStopLabelFontSize() const {
        return node_->AsMap().at("stop_label_font_size").AsInt();
    };

    const json::Array& Settings::GetStopLabelOffset() const {
        return node_->AsMap().at("stop_label_offset").AsArray();
    };

    svg::Color Settings::GetUnderlayerColor() const {
        const json::Node& undelayer_node = node_->AsMap().at("underlayer_color");
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
		const json::Array& color_palette = node_->AsMap().at("color_palette").AsArray();
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
        return node_->AsMap().at("underlayer_width").AsDouble();
    }

    /*
    * Сущность "Автобус"
    */
    const json::Array& BusEntity::GetStops() const {
        return node_->AsMap().at("stops").AsArray(); 
    } 

    bool BusEntity::IsRoundtrip() const {
        return node_->AsMap().at("is_roundtrip").AsBool(); 
    }

    /*
    * Сущность "Остановка"
    */
    const json::Dict& StopEntity::GetDistances() const { 
        return node_->AsMap().at("road_distances").AsMap(); 
    }

    double StopEntity::GetLongitude() const { 
        return node_->AsMap().at("longitude").AsDouble(); 
    }

    double StopEntity::GetLatitude() const { 
        return node_->AsMap().at("latitude").AsDouble(); 
    }

    /*
    * Общий класс запросов.
    */
    std::pair<std::vector<BusEntity>, std::vector<StopEntity>> Requests::GetBase() const {
        std::pair<std::vector<BusEntity>, std::vector<StopEntity>> base;
        for(const json::Node& node : GetRoot().AsMap().at("base_requests").AsArray()) {
            if (node.AsMap().at("type").AsString() == "Stop") {
                base.second.emplace_back(node);
            } else {
                base.first.emplace_back(node);
            }
        }
        return base;
    }

    std::vector<Stat> Requests::GetStats() const {
        std::vector<Stat> stat_requests;  
        for(const json::Node& node : GetRoot().AsMap().at("stat_requests").AsArray()) {
            stat_requests.emplace_back(node);
        }
        return stat_requests;
    }

    Settings Requests::GetRenderSettings() const {
        return GetRoot().AsMap().at("render_settings");
    }
} // end domain