#include "transport_router.h"

namespace Transport {

const graph::DirectedWeightedGraph<double>& RouterA::BuildGraph(const Transport::Catalogue& catalogue) {

	const std::map<std::string_view, std::shared_ptr<Transport::Stop>>& all_stops = catalogue.GetAllStops();
	const std::map<std::string_view, std::shared_ptr<Transport::Bus>>& all_buses = catalogue.GetAllBuses();
	graph::DirectedWeightedGraph<double> stops_graph(all_stops.size() * 2);
    std::map<std::string, graph::VertexId> stop_ids;
    graph::VertexId vertex_id = 0;

    for (const auto& [stop_name, stop_info] : all_stops) {
        stop_ids[stop_info->GetName()] = vertex_id;
        stops_graph.AddEdge({
                stop_info->GetName(),
                0,
                vertex_id,
                ++vertex_id,
                static_cast<double>(bus_wait_time_)
            });
        ++vertex_id;
    }
    stop_ids_ = std::move(stop_ids);

    for_each(
        all_buses.begin(),
        all_buses.end(),
        [&stops_graph, this, &catalogue](const auto& item) {
            // item - это std::pair<const std::string_view, std::shared_ptr<Bus>>
            const std::string_view& key = item.first; // Ключ
            const std::shared_ptr<Bus>& busPtr = item.second; // Указатель на Bus
            const auto& stops = busPtr->GetStops();
            size_t stops_count = stops.size();
            for (size_t i = 0; i < stops_count; ++i) {
                for (size_t j = i + 1; j < stops_count; ++j) {
                    const Stop* stop_from = stops[i];
                    const Stop* stop_to = stops[j];
                    int dist_sum = 0;
                    int dist_sum_inverse = 0;
                    for (size_t k = i + 1; k <= j; ++k) {
                        dist_sum += catalogue.GetDistance(stops[k - 1], stops[k]);
                        dist_sum_inverse += catalogue.GetDistance(stops[k], stops[k - 1]);
                    }
                    stops_graph.AddEdge({ busPtr->number,
                                          j - i,
                                          stop_ids_.at(stop_from->GetName()) + 1,
                                          stop_ids_.at(stop_to->GetName()),
                                          static_cast<double>(dist_sum) / (bus_velocity_ * (100.0 / 6.0))});

                    if (!busPtr->is_circle) {
                        stops_graph.AddEdge({ bus_info->number,
                                              j - i,
                                              stop_ids_.at(stop_to->GetName()) + 1,
                                              stop_ids_.at(stop_from->GetName()),
                                              static_cast<double>(dist_sum_inverse) / (bus_velocity_ * (100.0 / 6.0))});
                    }
                }
            }
        });

    graph_ = std::move(stops_graph);
    router_ = std::make_unique<graph::Router<double>>(graph_);

    return graph_;
}

const std::optional<graph::Router<double>::RouteInfo> RouterA::FindRoute(const std::string_view stop_from, const std::string_view stop_to) const {
	return router_->BuildRoute(stop_ids_.at(std::string(stop_from)),stop_ids_.at(std::string(stop_to)));
}

const graph::DirectedWeightedGraph<double>& RouterA::GetGraph() const {
	return graph_;
}

} // end Transport