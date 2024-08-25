#pragma once

#include "domain.h"
#include "svg.h"
#include "geo.h"
#include "json.h"
#include "transport_catalogue.h"

#include <algorithm>

namespace Render {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

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
	RoutesMap(domain::IRequests* requests) {
		requests->FillRenderSettings(*this);
	}

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