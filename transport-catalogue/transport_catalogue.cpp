#include <memory>
#include <string>

#include "transport_catalogue.h"

namespace Transport { 

using namespace std::string_literals;

/*
* Trasport::Stop
*/

const std::string& Stop::GetName() const {
    return name_;
}

const Geo::Coordinates& Stop::GetCoordinates() const {
    return coordinates_;
}

bool Stop::operator==(const Stop* other) const {
    return other->name_ == name_;
}

bool Stop::operator!=(const Stop* other) const {
    return !(other->name_ == name_);
}

bool Stop::operator==(const Stop& other) const {
    return other.name_ == name_;
}

bool Stop::operator!=(const Stop& other) const {
    return !(other.name_ == name_);
}

bool Stop::operator>(const Stop& other) const {
    return other.name_ < name_;
}

bool Stop::operator<(const Stop& other) const {
    return other.name_ > name_;
}

void Stop::AddBus(std::shared_ptr<Bus> bus) {
    std::weak_ptr<Bus> bus_weak_ptr = bus;
    if (unique_buses_.insert(bus_weak_ptr).second) {
        sorted_bus_names_.insert(bus->GetName());
    }
}

void Stop::AddAdjacent(std::string_view stop_name, std::size_t& distance) {
    if (!distance_to_adjacent_stops_.count(stop_name)) {
        distance_to_adjacent_stops_[stop_name] = distance;
    }
}

std::size_t Stop::GetDistanceTo(std::string_view stop_name) const {
    if (distance_to_adjacent_stops_.count(stop_name)) {
        return distance_to_adjacent_stops_.at(stop_name);
    }
    return 0;
}

std::size_t Stop::GetDistanceTo(const Stop* adjacent_stop) const {
    if (distance_to_adjacent_stops_.count(adjacent_stop->GetName())) {
        return distance_to_adjacent_stops_.at(adjacent_stop->GetName());
    }
    return adjacent_stop->GetDistanceTo(name_);
}

const std::set<std::string_view>& Stop::GetBusNames() const {
    return sorted_bus_names_;
}

std::size_t StopHasher::operator()(const std::weak_ptr<Stop>& stop_ptr) const {
    if (auto shared_stop = stop_ptr.lock()) {
        return std::hash<std::string>{}(shared_stop->GetName());
    } 
    return 0;
}


bool StopEqual::operator()(const std::weak_ptr<Stop>& a, const std::weak_ptr<Stop>& b) const {
    auto ptr_a = a.lock();
    auto ptr_b = b.lock();
    if (!ptr_a || !ptr_b) {
        return false;
    }
    return ptr_a->GetName() == ptr_b->GetName();
};

/*
* Transport::RoadMapSegment
*/

bool RoadMapSegment::operator==(const RoadMapSegment& other) const {
    return a.lock()->GetName() == other.a.lock()->GetName() && b.lock()->GetName() == other.b.lock()->GetName();
}

std::size_t RoadMapSegmentHasher::operator()(const RoadMapSegment& stop_pair) const {
    std::hash<std::string> hasher;
    return hasher(std::string(stop_pair.a.lock()->GetName())) + hasher(std::string(stop_pair.b.lock()->GetName())) * 10;
};
 
/*
* Trasport::Bus
*/
Bus::Bus(std::string name, RouteType type, const Catalogue& catalogue) : 
    name_(std::move(name)), 
    type_(type),
    catalogue_(catalogue) {
    // start_ = new Transport::Bus::RouteNode();
};

[[nodiscard]] Bus::RouteNode* Bus::begin() noexcept {
    return start_;
}

[[nodiscard]] Bus::RouteNode* Bus::end() noexcept {
    return end_;
}

[[nodiscard]] Bus::RouteIterator Bus::route_begin() noexcept {
    return RouteIterator{start_};
}

[[nodiscard]] Bus::RouteIterator Bus::route_end() noexcept {
    return RouteIterator{nullptr};
}

[[nodiscard]] Bus::RouteIterator Bus::r_route_end() noexcept {
    return RouteIterator{end_};
}

[[nodiscard]] Bus::RouteIterator Bus::r_route_begin() noexcept {
    return RouteIterator{nullptr};
}

std::size_t Bus::GetSize() const {
    return size_;
}

std::vector<std::weak_ptr<Stop>> Bus::GetUniqueStops() {
    return {unique_stops_.begin(), unique_stops_.end()};
}

const std::string& Bus::GetName() const {
    return name_;
}

RouteType Bus::GetType() const {
    return type_;
}

void Bus::AddStop(std::shared_ptr<Stop> stop) {
    unique_stops_.insert(stop);
    RouteNode* new_node = new RouteNode(stop);
    if (size_ == 0) {
        start_ = new_node;
        end_ = new_node;
    } else {
        if (type_ == RouteType::Line) {
            new_node->prev = end_;
        }
        end_->next = new_node;
        double geo_distance = ComputeDistance(last_stop_.lock()->GetCoordinates(), stop->GetCoordinates());
        length_ += geo_distance; 
        full_length_ += catalogue_.GetDistance(last_stop_, stop);
        if (IsLine()) {
            length_ += geo_distance;
            full_length_ += catalogue_.GetDistance(stop, last_stop_);
        }
        end_ = new_node;
    }
    last_stop_ = stop;
    std::shared_ptr<Bus> shared_this = shared_from_this();
    stop->AddBus(std::shared_ptr<Bus>(shared_this));
    ++size_;
}

double Bus::GetCurvature() {
    return static_cast<double>(full_length_/length_); 
}

std::size_t Bus::GetRouteSize() const {
    return type_ == RouteType::Line ? (size_ * 2) - 1 : size_;
}

std::size_t Bus::GetRouteLength() const {
    return full_length_;
}

bool Bus::IsEmpty() const {
    return size_ == 0; 
}

std::size_t Bus::GetUniqueStopsSize() {
    return unique_stops_.size();
};

inline std::size_t Transport::BusHasher::operator()(const std::weak_ptr<Bus>& bus) const {
    if (auto shared_bus = bus.lock()) {
        return std::hash<std::string>{}(shared_bus->GetName());
    }
    return 0;
}

inline bool Transport::BusComparator::operator()(const std::weak_ptr<Bus>& a, const std::weak_ptr<Bus>& b) const {
    auto ptr_a = a.lock();
    auto ptr_b = b.lock();
    if (!ptr_a || !ptr_b) {
        return false;
    }
    return ptr_a->GetName() < ptr_b->GetName();
}

inline bool Transport::BusEqual::operator()(const std::weak_ptr<Bus>& a, const std::weak_ptr<Bus>& b) const {
    auto ptr_a = a.lock();
    auto ptr_b = b.lock();
    if (!ptr_a || !ptr_b) {
        return false;
    }
    return ptr_a->GetName() == ptr_b->GetName();
}

/*
* Transport::Catalogue
*/
 
void Catalogue::AddStop(std::shared_ptr<Stop> stop) {
    stops_.emplace_back(stop);
    stops_dictionary_[stops_.back()->GetName()] = stops_.back();
}

void Catalogue::AddBus(std::shared_ptr<Bus> bus) {
    buses_.emplace_back(bus);
    buses_dictionary_[buses_.back()->GetName()] = buses_.back();
}
 
const std::shared_ptr<Stop> Catalogue::GetStop(std::string_view stop_name) const {
    if (stops_dictionary_.count(stop_name)) {
        return stops_dictionary_.at(stop_name);
    }
    return nullptr;
}
 
const std::shared_ptr<Bus> Catalogue::GetBus(std::string_view bus_name) const {
    if (buses_dictionary_.count(bus_name)) {
        return buses_dictionary_.at(bus_name);
    }
    return nullptr;
}

const BusesDictionary& Catalogue::GetAllBuses() const {
    return buses_dictionary_;
}

const StopsDictionary& Catalogue::GetAllStops() const {
    return stops_dictionary_;
}

void Catalogue::SetDistance(std::string_view stop_a_name, std::string_view stop_b_name, std::size_t distance) {
    std::shared_ptr<Stop> stop_a = GetStop(stop_a_name);
    std::shared_ptr<Stop> stop_b = GetStop(stop_b_name);
    stop_a->AddAdjacent(stop_b->GetName(), distance);
    stop_b->AddAdjacent(stop_a->GetName(), distance);
    SetDistance(std::weak_ptr(stop_a), std::weak_ptr(stop_b), distance);
}

void Catalogue::SetDistance(const std::weak_ptr<Stop> a, const std::weak_ptr<Stop> b, std::size_t distance) {
    segments_[{a, b}] = distance; 
}

std::size_t Catalogue::GetDistance(std::string_view stop_a_name, std::string_view stop_b_name) {
    std::shared_ptr<Stop> stop_a = GetStop(stop_a_name);
    std::shared_ptr<Stop> stop_b = GetStop(stop_b_name);
    return stop_a->GetDistanceTo(stop_b.get());
}

std::size_t Catalogue::GetDistance(const std::weak_ptr<Stop> a, const std::weak_ptr<Stop> b) const {
    RoadMapSegment segment{a, b};
    if (segments_.count(segment)) {
        return segments_.at(segment);
    }
    RoadMapSegment segment_reverse{b, a};
    if (segments_.count(segment_reverse)) {
        return segments_.at(segment_reverse);
    }
    // Расстояние до самой себя по-умолчанию
    return 0;
}
const Transport::SegmentsMap& Catalogue::GetSegmentsMap() const {
    return segments_; 
}

} // end Transport