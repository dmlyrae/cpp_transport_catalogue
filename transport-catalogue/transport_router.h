#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace Transport {

class RouterCreator;

class RouterA {
	friend RouterCreator;
public:
	const graph::DirectedWeightedGraph<double>& BuildGraph(const Transport::Catalogue& catalogue);
	const std::optional<graph::Router<double>::RouteInfo> FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;
	const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
	int bus_wait_time_ = 0;
	double bus_velocity_ = 0.0;

	RouterA(const domain::RouterSettings& settings, const Transport::Catalogue& catalogue) {
		bus_wait_time_ = settings.GetBusWaitTime();
		bus_velocity_ = settings.GetBusVelocity();
		BuildGraph(catalogue);
	}

	graph::DirectedWeightedGraph<double> graph_;
	std::map<std::string, graph::VertexId> stop_ids_;
	std::unique_ptr<graph::Router<double>> router_;
};

class RouterCreator {
public:
	RouterCreator() = default;

	RouterCreator& SetSettings(domain::RouterSettings& settings) {
		settings_ = &settings;
		return *this;
	}

	RouterCreator& SetCatalogue(Catalogue& catalogue) {
		catalogue_ = &catalogue;
		return *this;
	}

	RouterA Build() {
		if (!settings_ || !catalogue_) {
			throw std::runtime_error("--");
		}
		return { *settings_, *catalogue_ };
	}

private: 
	Catalogue* catalogue_ = nullptr;
	domain::RouterSettings* settings_ = nullptr;
};

}