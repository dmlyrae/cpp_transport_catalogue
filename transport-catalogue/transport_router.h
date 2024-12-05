#pragma once
#include "chrono"
#include <memory>

#include "router.h"
#include "transport_catalogue.h"

namespace Transport {

class RouterCreator;

class Router {
    friend RouterCreator;
public:
    const graph::DirectedWeightedGraph<double>& BuildGraph(const Transport::Catalogue& catalogue);
    const std::optional<graph::Router<double>::RouteInfo> FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;
    const graph::DirectedWeightedGraph<double>& GetGraph() const;

    Router(const domain::RouterSettings& settings, const Transport::Catalogue& catalogue) {
        bus_wait_time_ = settings.GetBusWaitTime();
        bus_velocity_ = settings.GetBusVelocity();
        BuildGraph(catalogue);
    }

    Router(const Router&) = delete;

    // Конструктор перемещения
    Router(Router&& other) noexcept 
        : bus_wait_time_(other.bus_wait_time_),
        bus_velocity_(other.bus_velocity_),
        graph_(std::move(other.graph_)),
        stop_ids_(std::move(other.stop_ids_)),
        id_stops_(std::move(other.id_stops_)),
        router_(std::move(other.router_)) 
    {}

    // Оператор перемещения
    Router& operator=(Router&& other) noexcept;

    const std::pair<std::vector<domain::PassengerAction>, double> GetRoute(graph::Router<double>::RouteInfo& routing) const;

private:
    int bus_wait_time_ = 0;
    double bus_velocity_ = 0.0;
    graph::DirectedWeightedGraph<double> graph_;
    std::map<std::string, graph::VertexId> stop_ids_;
    std::map<graph::VertexId, std::string> id_stops_;
    std::unique_ptr<graph::Router<double>> router_;
};

class RouterCreator {
public:
    RouterCreator() = default;

    RouterCreator& SetSettings(domain::RouterSettings& settings) {
        settings_ = &settings;
        return *this;
    }

    RouterCreator& SetCatalogue(Catalogue* catalogue) {
        catalogue_ = catalogue;
        return *this;
    }

    Router Build() {
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