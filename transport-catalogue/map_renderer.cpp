#include "map_renderer.h"

namespace Render {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

template <typename PointIt>
SphereProjector::SphereProjector(
		PointIt points_begin, 
		PointIt points_end,
		double max_width,
		double max_height, 
		double padding
	) : padding_(padding) {


		if (points_end == points_begin) {
			return;
		}
		
		/* Границы окна */
		const auto [bottom_it, top_it] = std::minmax_element( points_begin, points_end, [](auto lhs, auto rhs) { 
			return lhs.lat < rhs.lat; 
		});
		max_lat_ = top_it->lat;
		const double min_lat = bottom_it->lat;

		const auto [ left_it, right_it ] = std::minmax_element( points_begin, points_end, [](auto lhs, auto rhs) { 
			return lhs.lng < rhs.lng; 
		});
		min_lon_ = left_it->lng;
		const double max_lon = right_it->lng;
		
		std::optional<double> width_zoom;
		if (!IsZero(max_lon - min_lon_)) {
			width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
		}
		
		/* Коэффициент зума */
		std::optional<double> height_zoom;
		if (!IsZero(max_lat_ - min_lat)) {
			height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
		}
		
		if (width_zoom && height_zoom) {
			zoom_coeff_ = std::min(*width_zoom, *height_zoom);
		} else if (width_zoom) {
			zoom_coeff_ = *width_zoom;
		} else if (height_zoom) {
			zoom_coeff_ = *height_zoom;
		}
	}

svg::Point SphereProjector::operator()(Geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

void RoutesMap::AppplySettings(domain::Settings& svg_settings) {
    /* Общие настройки */
    render_settings_.width = svg_settings.GetWidth();
    render_settings_.height = svg_settings.GetHeight();
    render_settings_.padding = svg_settings.GetPadding();
    render_settings_.stop_radius = svg_settings.GetRadius();
    render_settings_.line_width = svg_settings.GetLineWidth();

    /* Лейблы */
    render_settings_.bus_label_font_size = svg_settings.GetBusLabelFontSize();
    const json::Array& bus_label_offset = svg_settings.GetBusLabelOffset();
    render_settings_.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
    render_settings_.stop_label_font_size = svg_settings.GetStopLabelFontSize();
    const json::Array& stop_label_offset = svg_settings.GetStopLabelOffset();
    render_settings_.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

    /* Подложка */
    render_settings_.underlayer_color = svg_settings.GetUnderlayerColor();
    render_settings_.color_palette = svg_settings.GetPalette();
    render_settings_.underlayer_width = svg_settings.GetUnderlayerWidth();
}

std::vector<svg::Polyline> RoutesMap::GetRouteLines(const Transport::BusesDictionary& buses, const SphereProjector& sphere_projector) const {
    std::vector<svg::Polyline> result;
    int color_num = 0;
    for (const auto& [ bus_name, bus ] : buses) {
        if (bus->IsEmpty()) {
            continue;
        }
        std::vector<std::shared_ptr<Transport::Stop>> route_stops;
        for(auto it = bus->route_begin(); it != bus->route_end(); ++it) {
            route_stops.push_back(it->stop);
        }
        if (bus->IsLine() && bus->GetUniqueStopsSize() > 1) {
            auto it = bus->r_route_end();
            --it;
            while(it != bus->r_route_begin()) {
                route_stops.push_back(it->stop);
                --it;
            }
        }
        
        svg::Polyline line;
        for (const auto& stop : route_stops) {
            line.AddPoint(sphere_projector(stop->GetCoordinates()));
        }
        line.SetStrokeColor(render_settings_.color_palette[color_num]);
        line.SetFillColor("none");
        line.SetStrokeWidth(render_settings_.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        color_num = (color_num + 1) % render_settings_.color_palette.size();
        
        result.push_back(line);
    }
    
    return result;
}

void RoutesMap::FillSVG(svg::Document& svg, const Transport::Catalogue& catalogue) const {
    const Transport::BusesDictionary& buses = catalogue.GetAllBuses();
    const Transport::StopsDictionary& stops = catalogue.GetAllStops();

    std::vector<Geo::Coordinates> route_stops_coord;
    for (auto [ bus_name, bus ] : buses) {
        for (auto it = bus->route_begin(); it != bus->route_end(); ++it) {
            route_stops_coord.push_back(it->stop->GetCoordinates());
        }
    }
    SphereProjector sp(route_stops_coord.begin(), route_stops_coord.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    for (const auto& line : GetRouteLines(buses, sp)) {
        svg.Add(line);
    }

    for (const auto& label : GetBusLabel(buses, sp)) {
        svg.Add(label);
    }

    for (const auto& label : GetStopsSymbols(stops, sp)) {
        svg.Add(label);
    }

    for (const auto& label : GetStopsLabels(stops, sp)) {
        svg.Add(label);
    }
}

std::vector<svg::Text> RoutesMap::GetBusLabel(const Transport::BusesDictionary& buses, const SphereProjector& sp) const {
    std::vector<svg::Text> result;
    int color_num = 0;
    for (const auto& [bus_number, bus] : buses) {
        if (bus->IsEmpty()) {
            continue;
        }
        svg::Text text;
        svg::Text text_underlayer;

        /* Основной текст */
        text.SetData(bus->GetName());
        text.SetPosition(sp(bus->route_begin()->stop->GetCoordinates()));
        text.SetOffset(render_settings_.bus_label_offset);
        text.SetFontSize(render_settings_.bus_label_font_size);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetFillColor(render_settings_.color_palette[color_num]);

        color_num = (color_num + 1) % render_settings_.color_palette.size();
        
        /* Подложка */
        text_underlayer.SetData(bus->GetName());
        text_underlayer.SetFontSize(render_settings_.bus_label_font_size);
        text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_underlayer.SetPosition(sp(bus->route_begin()->stop->GetCoordinates()));
        text_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        text_underlayer.SetFontWeight("bold");
        text_underlayer.SetOffset(render_settings_.bus_label_offset);
        text_underlayer.SetFontFamily("Verdana");
        text_underlayer.SetFillColor(render_settings_.underlayer_color);
        text_underlayer.SetStrokeColor(render_settings_.underlayer_color);
        text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        
        result.push_back(text_underlayer);
        result.push_back(text);
        
        if (bus->IsLine() && bus->begin()->stop->GetName() != bus->end()->stop->GetName()) {
            svg::Text text_second {text};
            svg::Text text_underlayer_second {text_underlayer};
            text_second.SetPosition(sp(bus->r_route_end()->stop->GetCoordinates()));
            text_underlayer_second.SetPosition(sp(bus->r_route_end()->stop->GetCoordinates()));

            result.push_back(text_underlayer_second);
            result.push_back(text_second);
        }
    }
    
    return result;
}

std::vector<svg::Circle> RoutesMap::GetStopsSymbols(const Transport::StopsDictionary& stops, const SphereProjector& sp) const {
    std::vector<svg::Circle> result;
    for (const auto& [stop_name, stop] : stops) {
        if (stop->GetBusNames().empty()) {
            continue;
        }
        svg::Circle symbol;
        symbol.SetCenter(sp(stop->GetCoordinates()));
        symbol.SetRadius(render_settings_.stop_radius);
        symbol.SetFillColor("white");
        
        result.push_back(symbol);
    }
    
    return result;
}

std::vector<svg::Text> RoutesMap::GetStopsLabels(const Transport::StopsDictionary& stops, const SphereProjector& sp) const {
    std::vector<svg::Text> result;
    svg::Text text;
    svg::Text text_underlayer;

    for (const auto& [stop_name, stop] : stops) {
        if (stop->GetBusNames().empty()) {
            continue;
        }

        /* Основной текст */
        text.SetData(stop->GetName());
        text.SetPosition(sp(stop->GetCoordinates()));
        text.SetOffset(render_settings_.stop_label_offset);
        text.SetFontSize(render_settings_.stop_label_font_size);
        text.SetFontFamily("Verdana");
        text.SetFillColor("black");
        
        /* Подложка */
        text_underlayer.SetData(stop->GetName());
        text_underlayer.SetFontFamily("Verdana");
        text_underlayer.SetOffset(render_settings_.stop_label_offset);
        text_underlayer.SetPosition(sp(stop->GetCoordinates()));
        text_underlayer.SetFontSize(render_settings_.stop_label_font_size);
        text_underlayer.SetStrokeColor(render_settings_.underlayer_color);
        text_underlayer.SetFillColor(render_settings_.underlayer_color);
        text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        
        result.push_back(text_underlayer);
        result.push_back(text);
    }
    
    return result;
}

} // end Render