#include "domain_transport.h"

namespace domain {

    /*
    * Сущность "Автобус"
    */
    const json::Array& BusEntity::GetStops() const {
        return node_->AsDict().at("stops").AsArray(); 
    } 

    bool BusEntity::IsRoundtrip() const {
        return node_->AsDict().at("is_roundtrip").AsBool(); 
    }

    /*
    * Сущность "Остановка"
    */
    const json::Dict& StopEntity::GetDistances() const { 
        return node_->AsDict().at("road_distances").AsDict(); 
    }

    double StopEntity::GetLongitude() const { 
        return node_->AsDict().at("longitude").AsDouble(); 
    }

    double StopEntity::GetLatitude() const { 
        return node_->AsDict().at("latitude").AsDouble(); 
    }

}