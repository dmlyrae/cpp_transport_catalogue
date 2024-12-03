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
			const std::shared_ptr<Bus>& busPtr = item.second; // Указатель на Bus
			auto firstRouteIt = busPtr->begin();
			while (firstRouteIt) {
				auto secondRouteIt = firstRouteIt->next;
				while (secondRouteIt) {
					const std::shared_ptr<Stop> stop_from = firstRouteIt->stop;
					const std::shared_ptr<Stop> stop_to = secondRouteIt->stop;

					int dist_sum = 0;
					int dist_sum_inverse = 0;

					// Для подсчета расстояний между остановками
					auto currentIt = firstRouteIt->next; // Начинаем с первой остановки после stop_from
					auto prevIt = firstRouteIt; 
					size_t distance_between_stops = 1;
					while (currentIt != secondRouteIt->next) {
						dist_sum += catalogue.GetDistance(prevIt->stop, currentIt->stop);
						dist_sum_inverse += catalogue.GetDistance(currentIt->stop, prevIt->stop);
						prevIt = currentIt;
						currentIt = currentIt->next;
						++distance_between_stops;
					}

					// Добавляем ребро в граф
					stops_graph.AddEdge({ 
						busPtr->GetName(),
						distance_between_stops, 
						stop_ids_.at(stop_from->GetName()) + 1,
						stop_ids_.at(stop_to->GetName()),
						static_cast<double>(dist_sum) / (bus_velocity_ * (100.0 / 6.0))
					});

					if (busPtr->IsLine()) {
						stops_graph.AddEdge({ 
							busPtr->GetName(),
							distance_between_stops, 
							stop_ids_.at(stop_to->GetName()) + 1,
							stop_ids_.at(stop_from->GetName()),
							static_cast<double>(dist_sum_inverse) / (bus_velocity_ * (100.0 / 6.0))
						});
					}

					secondRouteIt = secondRouteIt->next;
				}
				firstRouteIt = firstRouteIt->next;
			}
		});
	graph_ = std::move(stops_graph);
	router_ = std::make_unique<graph::Router<double>>(graph_);

	return graph_;
}

const std::optional<graph::Router<double>::RouteInfo> RouterA::FindRoute(const std::string_view stop_from, const std::string_view stop_to) const {
	return router_->BuildRoute(stop_ids_.at(std::string(stop_from)), stop_ids_.at(std::string(stop_to)));
}

const graph::DirectedWeightedGraph<double>& RouterA::GetGraph() const {
	return graph_;
}

} // end Transport