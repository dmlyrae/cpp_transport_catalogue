#include "domain_transport.h"

namespace domain {

    /*
    * Сущность "Автобус"
    */
    const json::Array& BusEntity::GetStops() const {
        return node_->AsMap().at("stops").AsArray(); 
    } 

    bool BusEntity::IsRoundtrip() const {
        return node_->AsMap().at("is_roundtrip").AsBool(); 
    }

    /*
    * Сущность "Остановка"
    */
    const json::Dict& StopEntity::GetDistances() const { 
        return node_->AsMap().at("road_distances").AsMap(); 
    }

    double StopEntity::GetLongitude() const { 
        return node_->AsMap().at("longitude").AsDouble(); 
    }

    double StopEntity::GetLatitude() const { 
        return node_->AsMap().at("latitude").AsDouble(); 
    }

}