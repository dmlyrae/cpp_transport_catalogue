#pragma once

#include <cassert>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
 
#include "domain.h"
#include "geo.h"

namespace Transport {

enum class RouteType { Line, Ring };

class Bus;
class Stop;
class RoadMapSegment;
class Catalogue;

struct RoadMapSegment {
    std::weak_ptr<Stop> a;
    std::weak_ptr<Stop> b;
    bool operator==(const RoadMapSegment& other) const;
};

struct RoadMapSegmentHasher {
    std::size_t operator()(const RoadMapSegment& stop_pair) const;
};

using SegmentsMap = std::unordered_map<RoadMapSegment, std::size_t, RoadMapSegmentHasher>;
using StopsDictionary = std::map<std::string_view, std::shared_ptr<Stop>>;
using BusesDictionary = std::map<std::string_view, std::shared_ptr<Bus>>;

struct BusHasher {
    std::size_t operator()(const std::weak_ptr<Bus>& bus) const; 
};

struct BusComparator {
    bool operator()(const std::weak_ptr<Bus>& a, const std::weak_ptr<Bus>& b) const;
};

struct BusEqual {
    bool operator()(const std::weak_ptr<Bus>& a, const std::weak_ptr<Bus>& b) const;
};

using StopsBuses = std::unordered_set<std::weak_ptr<Bus>, BusHasher, BusEqual>;

class Stop : public std::enable_shared_from_this<Stop> {

public:
    explicit Stop(std::string name, Geo::Coordinates coordinates) : name_(name), coordinates_(coordinates) {}

    const std::string& GetName() const;

    const Geo::Coordinates& GetCoordinates() const;

    bool operator==(const Stop* other) const;
    bool operator!=(const Stop* other) const;
    bool operator==(const Stop& other) const;
    bool operator!=(const Stop& other) const;
    bool operator>(const Stop& other) const;
    bool operator<(const Stop& other) const;

    void AddBus(std::shared_ptr<Bus> bus);

    void AddAdjacent(std::string_view stop, std::size_t& distance);
    std::size_t GetDistanceTo(const Stop* adjacent_stop) const;
    std::size_t GetDistanceTo(std::string_view stop_name) const;

    const std::set<std::string_view>& GetBusNames() const;

private:
    std::string name_;
    Geo::Coordinates coordinates_;
    StopsBuses unique_buses_;
    std::unordered_map<std::string_view, std::size_t> distance_to_adjacent_stops_;
    std::set<std::string_view> sorted_bus_names_;
};

struct StopHasher {
    std::size_t operator()(const std::weak_ptr<Stop>& stop) const;
};

struct StopEqual {
    bool operator()(const std::weak_ptr<Stop>& a, const std::weak_ptr<Stop>& b) const;
};

class Bus : public std::enable_shared_from_this<Bus> {
public:

    struct RouteNode {
        explicit RouteNode() : next(nullptr), stop(nullptr), prev(nullptr) {}
        explicit RouteNode(std::shared_ptr<Stop> stop): next(nullptr), stop(stop), prev(nullptr) {}
        RouteNode* next;
        std::shared_ptr<Stop> stop;
        RouteNode* prev;
    };

    [[nodiscard]] RouteNode* begin() noexcept;
    [[nodiscard]] RouteNode* end() noexcept;

    class RouteIterator {
    friend class Bus;
    explicit RouteIterator(Bus::RouteNode* node) {
        node_ = node;
    }

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Bus::RouteNode;
        // Тип, используемый для хранения смещения между итераторами
        using difference_type = std::ptrdiff_t;
        // Тип указателя на итерируемое значение
        using pointer = Bus::RouteNode*;
        // Тип ссылки на итерируемое значение
        using reference = const Bus::RouteNode&;

        RouteIterator() = default;

        // Конвертирующий конструктор/конструктор копирования
        // При ValueType, совпадающем с Type, играет роль копирующего конструктора
        // При ValueType, совпадающем с const Type, играет роль конвертирующего конструктора
        RouteIterator(const RouteIterator& other) noexcept {
            node_ = other.node_;
        }

        // Чтобы компилятор не выдавал предупреждение об отсутствии оператора = при наличии
        // пользовательского конструктора копирования, явно объявим оператор = и
        // попросим компилятор сгенерировать его за нас
        RouteIterator& operator=(const RouteIterator& rhs) = default;

        // Оператор сравнения итераторов (в роли второго аргумента выступает константный итератор)
        // Два итератора равны, если они ссылаются на один и тот же элемент списка либо на end()
        [[nodiscard]] bool operator==(const RouteIterator& rhs) const noexcept {
            return rhs.node_ == this->node_;
            // Заглушка. Реализуйте оператор самостоятельно
        }

        // Оператор проверки итераторов на неравенство
        // Противоположен !=
        [[nodiscard]] bool operator!=(const RouteIterator& rhs) const noexcept {
            return rhs.node_ != this->node_;
        }

        // Операция разыменования. Возвращает ссылку на остановку.
        [[nodiscard]] reference operator*() const noexcept {
            return *node_;
        }

        // Операция доступа к члену класса. Возвращает указатель на текущий элемент списка
        [[nodiscard]] pointer operator->() const noexcept {
            assert(node_ != nullptr);
            return node_;
        }

        RouteIterator& operator++() noexcept {
            node_ = node_->next;
            return *this;
        }

        RouteIterator operator++(int) noexcept {
            auto o(*this);
            node_ = node_->next;
            return o;
            // Заглушка. Реализуйте оператор самостоятельно
        }

        RouteIterator& operator--() noexcept {
            node_ = node_->prev;
            return *this;
        }

        RouteIterator operator--(int) noexcept {
            auto o(*this);
            node_ = node_->prev;
            return o;
        }

    private:
        Bus::RouteNode* node_ = nullptr;
    };

    RouteIterator route_begin() noexcept;
    RouteIterator route_end() noexcept;
    RouteIterator r_route_end() noexcept;
    RouteIterator r_route_begin() noexcept;

    explicit Bus(std::string name, RouteType type, const Catalogue& catalogue); 

    void AddStop(std::shared_ptr<Stop> stop);

    double GetCurvature();

    bool IsEmpty() const;

    std::size_t GetSize() const;

    std::vector<std::weak_ptr<Stop>> GetUniqueStops();

    bool IsLine() {
        return type_ == RouteType::Line;
    }

    RouteType GetType() const;

    std::size_t GetRouteLength() const ;
    std::size_t GetRouteSize() const;
    std::size_t GetUniqueStopsSize();
    const std::string& GetName() const;

    ~Bus() {
        RouteNode* current = begin();
        while (current != nullptr) {
            RouteNode* temp = current;
            current = current->next;
            delete temp;
        }
    }

private:
    std::string name_;
    RouteType type_;
    RouteNode* current_;
    std::size_t size_ = 0;
    double length_ = 0;
    double full_length_ = 0;
    RouteNode* start_ = nullptr;
    RouteNode* end_ = nullptr;
    std::weak_ptr<Stop> last_stop_;
    std::unordered_set<std::weak_ptr<Stop>, StopHasher, StopEqual> unique_stops_;
    const Catalogue& catalogue_; 
};

class Catalogue {
public:
    explicit Catalogue() = default;
    Catalogue(domain::IRequests* requests) {
        requests->FillTransportCatalogue(*this);
    };

    void AddStop(std::shared_ptr<Stop> stop);
    void AddBus(std::shared_ptr<Bus> bus);

    const std::shared_ptr<Stop> GetStop(std::string_view stop_name) const;
    const std::shared_ptr<Bus> GetBus(std::string_view bus_name) const;
    const BusesDictionary& GetAllBuses() const;
    const StopsDictionary& GetAllStops() const;

    void SetDistance(std::string_view stop_a, std::string_view stop_b, std::size_t distance);
    std::size_t GetDistance(std::string_view stop_a, std::string_view stop_b);

    void SetDistance(const std::weak_ptr<Stop> a, const std::weak_ptr<Stop> b, std::size_t distance);
    std::size_t GetDistance(const std::weak_ptr<Stop> a, const std::weak_ptr<Stop> b) const;

    const SegmentsMap& GetSegmentsMap() const;

    ~Catalogue() = default;

private:
    std::deque<std::shared_ptr<Bus>> buses_;
    std::deque<std::shared_ptr<Stop>> stops_;
    StopsDictionary stops_dictionary_;
    BusesDictionary buses_dictionary_;
    SegmentsMap segments_;
};

} // end Transport