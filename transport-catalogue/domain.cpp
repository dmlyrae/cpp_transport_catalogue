#include <stddef.h>
#include <set>
#include <sstream>

#include "domain.h"
#include "geo.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace domain {

    /*
    * Общие методы сущностей
    */
    BaseEntity::BaseEntity(const json::Node& node) {
        node_ = std::make_shared<json::Node>(node);
    }

    std::string BaseEntity::GetType() const {
        return node_->AsMap().at("type").AsString();
    }

    bool BaseEntity::IsStop() const {
        return node_->AsMap().at("type").AsString() == "Stop";
    }

    bool BaseEntity::IsBus() const {
        return node_->AsMap().at("type").AsString() == "Bus";
    }

    bool BaseEntity::IsMap() const {
        return node_->AsMap().at("type").AsString() == "Map";
    }

    const json::Node* BaseEntity::GetNode() const {
        return node_.get();
    }

    const std::string& Entity::GetName() const { 
        return node_->AsMap().at("name").AsString(); 
    }

} // end domain