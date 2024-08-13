#pragma once

#include "svg.h"
#include "geo.h"
#include "json.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <algorithm>

namespace Render {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

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

class SphereProjector {
public:

	template <typename PointInputIt>
 	SphereProjector(
		PointInputIt points_begin, 
		PointInputIt points_end,
		double max_width,
		double max_height, 
		double padding
	);
	
	svg::Point operator()(Geo::Coordinates coords) const;
	
private:
	double padding_;
	double min_lon_ = 0;
	double max_lat_ = 0;
	double zoom_coeff_ = 0;
};

class RoutesMap {
public:
	RoutesMap() = default;

	void AppplySettings(domain::Settings& svg_settings);
	
	std::vector<svg::Polyline> GetRouteLines(const Transport::BusesDictionary& buses, const SphereProjector& sp) const;
    std::vector<svg::Text> GetBusLabel(const Transport::BusesDictionary& buses, const SphereProjector& sp) const;
    std::vector<svg::Text> GetStopsLabels(const Transport::StopsDictionary& stops, const SphereProjector& sp) const;
    std::vector<svg::Circle> GetStopsSymbols(const Transport::StopsDictionary& stops, const SphereProjector& sp) const;

	void FillSVG(svg::Document& svg, const Transport::Catalogue& catalogue) const;
	
private:
	RenderSettings render_settings_;
};

} // end Render