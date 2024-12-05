#include "transport_router.h"

namespace Transport {

const graph::DirectedWeightedGraph<double>& Router::BuildGraph(const Transport::Catalogue& catalogue) {

    const std::map<std::string_view, std::shared_ptr<Transport::Stop>>& all_stops = catalogue.GetAllStops();
    const std::map<std::string_view, std::shared_ptr<Transport::Bus>>& all_buses = catalogue.GetAllBuses();
    graph::DirectedWeightedGraph<double> stops_graph(all_stops.size());
    std::map<std::string, graph::VertexId> stop_ids;
    std::map<graph::VertexId, std::string> id_stops;
    graph::VertexId vertex_id = 0;

    for (const auto& [stop_name, stop_info] : all_stops) {
        stop_ids[stop_info->GetName()] = vertex_id;
        id_stops[vertex_id] = stop_info->GetName();
        ++vertex_id;
    }

    stop_ids_ = std::move(stop_ids);
    id_stops_ = std::move(id_stops);

    for_each(
        all_buses.begin(),
        all_buses.end(),
        [&stops_graph, this, &catalogue](const auto& item) {
            const std::shared_ptr<Bus>& bus_ptr = item.second; // Указатель на Bus
            auto firstRouteIt = bus_ptr->begin(); // Стартовая остановка маршрута
            auto secondRouteIt = firstRouteIt->next;
            size_t distance_between_stops = 1;
            size_t start_window_length = 0;
            size_t start_reverse_window_length = 0;
            if (secondRouteIt) {
                start_window_length += catalogue.GetDistance(firstRouteIt->stop, secondRouteIt->stop);
                if (bus_ptr->IsLine()) {
                    start_reverse_window_length += catalogue.GetDistance(secondRouteIt->stop, firstRouteIt->stop);
                }
            }
            while (secondRouteIt) {
                /**
                 * Формирование окна
                 */
                auto start_window_it = firstRouteIt;
                auto end_window_it = secondRouteIt;
                size_t window_length = start_window_length; 
                size_t reverse_window_length = start_reverse_window_length; 

                /**
                 * Проход по остановкам, добавление рёбер графа.
                 */
                while (end_window_it) {
                    /**
                     * Добавление рёбер
                     */
                    stops_graph.AddEdge({ 
                        bus_ptr->GetName(),
                        distance_between_stops, 
                        stop_ids_.at(start_window_it->stop->GetName()),
                        stop_ids_.at(end_window_it->stop->GetName()),
                        static_cast<double>(window_length) / (bus_velocity_ * (100.0 / 6.0)) + bus_wait_time_
                    });
                    if (bus_ptr->IsLine()) {
                        stops_graph.AddEdge({ 
                            bus_ptr->GetName(),
                            distance_between_stops, 
                            stop_ids_.at(end_window_it->stop->GetName()),
                            stop_ids_.at(start_window_it->stop->GetName()),
                            static_cast<double>(reverse_window_length) / (bus_velocity_ * (100.0 / 6.0)) + bus_wait_time_
                        });
                    }
                    /**
                     * Сдвиг окна
                     */
                    if (end_window_it->next) {
                        window_length += catalogue.GetDistance(end_window_it->stop, end_window_it->next->stop);
                        window_length -= catalogue.GetDistance(start_window_it->stop, start_window_it->next->stop);
                        if (bus_ptr->IsLine()) {
                            reverse_window_length += catalogue.GetDistance(end_window_it->next->stop, end_window_it->stop);
                            reverse_window_length -= catalogue.GetDistance(start_window_it->next->stop, start_window_it->stop);
                        }
                    }
                    start_window_it = start_window_it->next;
                    end_window_it = end_window_it->next;
                }

                /**
                 * Увеличение окна
                 */
                ++distance_between_stops;
                if (secondRouteIt->next) {
                    start_window_length += catalogue.GetDistance(secondRouteIt->stop, secondRouteIt->next->stop);
                    start_reverse_window_length += catalogue.GetDistance(secondRouteIt->next->stop, secondRouteIt->stop);
                }
                secondRouteIt = secondRouteIt->next;
            }
        });
    graph_ = std::move(stops_graph);
    router_ = std::make_unique<graph::Router<double>>(graph_);

    return graph_;
}

const std::optional<graph::Router<double>::RouteInfo> Router::FindRoute(const std::string_view stop_from, const std::string_view stop_to) const {
    return router_->BuildRoute(stop_ids_.at(std::string(stop_from)), stop_ids_.at(std::string(stop_to)));
}

const graph::DirectedWeightedGraph<double>& Router::GetGraph() const {
    return graph_;
}

/**
 * Оператор перемещения Router
 */
Router& Router::operator=(Router&& other) noexcept {
    if (this != &other) {
        bus_wait_time_ = other.bus_wait_time_;
        bus_velocity_ = other.bus_velocity_;
        graph_ = std::move(other.graph_);
        stop_ids_ = std::move(other.stop_ids_);
        id_stops_ = std::move(other.id_stops_);
        router_ = std::move(other.router_);
    }
    return *this;
}

const std::pair<std::vector<domain::PassengerAction>, double> Router::GetRoute(graph::Router<double>::RouteInfo& routing) const {
    std::vector<domain::PassengerAction> items;
    double total_time = 0.0;
    items.reserve(routing.edges.size());
    for (auto& edge_id : routing.edges) {
        const graph::Edge<double> edge = graph_.GetEdge(edge_id);
        items.emplace_back(
            json::Node(
                json::Builder{}
                    .StartDict()
                        .Key("time")
                        .Value(bus_wait_time_)
                        .Key("stop_name")
                        .Value(id_stops_.at(edge.from))
                        .Key("type")
                        .Value("Wait")
                    .EndDict()
                .Build()
            )
        );
        items.emplace_back(
            json::Node(
                json::Builder{}
                    .StartDict()
                        .Key("bus")
                        .Value(edge.name)
                        .Key("span_count")
                        .Value(static_cast<int>(edge.quality))
                        .Key("time")
                        .Value(edge.weight - bus_wait_time_)
                        .Key("type")
                        .Value("Bus")
                    .EndDict()
                .Build()
            )
        );
        total_time += edge.weight;
    }
    return { items, total_time };
}

} // end Transport