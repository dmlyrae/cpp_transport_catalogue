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

	int bus_wait_time_ = 0;
	double bus_velocity_ = 0.0;

	RouterA(const domain::RouterSettings& settings, const Transport::Catalogue& catalogue) {
		bus_wait_time_ = settings.GetBusWaitTime();
		bus_velocity_ = settings.GetBusVelocity();
		BuildGraph(catalogue);
	}

	RouterA(const RouterA&) = delete;

	// Конструктор перемещения
	RouterA(RouterA&& other) noexcept 
		: bus_wait_time_(other.bus_wait_time_),
		bus_velocity_(other.bus_velocity_),
		graph_(std::move(other.graph_)),
		stop_ids_(std::move(other.stop_ids_)),
		router_(std::move(other.router_)) 
	{}

	// Оператор перемещения
	RouterA& operator=(RouterA&& other) noexcept {
		if (this != &other) {
		bus_wait_time_ = other.bus_wait_time_;
		bus_velocity_ = other.bus_velocity_;
		graph_ = std::move(other.graph_);
		stop_ids_ = std::move(other.stop_ids_);
		router_ = std::move(other.router_);
		}
		return *this;
	}

	const std::pair<std::vector<domain::PassengerAction>, double> GetRoute(graph::Router<double>::RouteInfo& routing) const {
		std::vector<domain::PassengerAction> items;
		double total_time = 0.0;
		items.reserve(routing.edges.size());
		for (auto& edge_id : routing.edges) {
			const graph::Edge<double> edge = graph_.GetEdge(edge_id);
			if (edge.quality == 0) {
				items.emplace_back(
					json::Node(
						json::Builder{}
							.StartDict()
								.Key("time")
								.Value(edge.weight)
								.Key("stop_name")
								.Value(edge.name)
								.Key("type")
								.Value("Wait")
							.EndDict()
						.Build()
					));
				total_time += edge.weight;
			} else {
				items.emplace_back(
					json::Node(
						json::Builder{}
							.StartDict()
								.Key("bus")
								.Value(edge.name)
								.Key("span_count")
								.Value(static_cast<int>(edge.quality))
								.Key("time")
								.Value(edge.weight)
								.Key("type")
								.Value("Bus")
							.EndDict()
						.Build()
					));
				total_time += edge.weight;
			}
		}
		return { items, total_time };
	}

private:
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