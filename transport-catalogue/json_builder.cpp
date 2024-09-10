#include "optional"
#include "variant"

#include "json_builder.h"

namespace json {

/**
 * Builder
 */

Builder::Builder() {
    Node* root_ptr = &root_;
    nodes_stack_.emplace_back(root_ptr);
}

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Attempt to build JSON which isn't finalized"s);
    }
    return std::move(root_);
}

Builder::DictKeyContext Builder::Key(std::string key) {

    Node::Value& host_value = GetCurrentValue();
    
    if (!std::holds_alternative<Dict>(host_value)) {
        throw std::logic_error("Key() outside a dict"s);
    }
    
    nodes_stack_.push_back(
        &std::get<Dict>(host_value)[std::move(key)]
    );

    return *this;
}

Builder& Builder::Value(Node::Value value) {
    Add(std::move(value), true);
    return *this;
}

Node::Value& Builder::GetCurrentValue() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Attempt to change finalized JSON"s);
    }
    return nodes_stack_.back()->GetValue();
}

const Node::Value& Builder::GetCurrentValue() const {
    return const_cast<Builder*>(this)->GetCurrentValue();
}

Builder& Builder::EndDict() {
    if (!std::holds_alternative<Dict>(GetCurrentValue())) {
        throw std::logic_error("EndDict() outside a dict"s);
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    if (!std::holds_alternative<Array>(GetCurrentValue())) {
        throw std::logic_error("EndDict() outside an array"s);
    }
    nodes_stack_.pop_back();
    return *this;
}

void Builder::Add(Node::Value value, bool one_shot) {
    Node::Value& host_value = GetCurrentValue();
    if (std::holds_alternative<Array>(host_value)) {
        Node& node = std::get<Array>(host_value).emplace_back(std::move(value));
        if (!one_shot) {
            nodes_stack_.push_back(&node);
        }

    } else {
        host_value = std::move(value);
        if (one_shot) {
            nodes_stack_.pop_back();
        }
    }
}

Builder::DictValueContext Builder::StartDict() {
    Add(Dict{}, false);
    return *this;
}

Builder::ArrayContext Builder::StartArray() {
    Add(Array{}, false);
    return *this;
}

/**
 * BaseContext
 */

Builder::BaseContext::BaseContext(Builder& builder)
    : builder_(builder) {}

Builder::ArrayContext Builder::BaseContext::StartArray() {
    return builder_.StartArray();
}

Builder::DictValueContext Builder::BaseContext::StartDict() {
    return builder_.StartDict();
}

/**
 * DictContext 
 */

Builder::DictValueContext::DictValueContext(Builder& builder) : BaseContext(builder) {}

Builder::DictKeyContext::DictKeyContext(Builder& builder) : BaseContext(builder) {}

Builder::DictValueContext Builder::DictKeyContext::Value(Node::Value value) {
    return builder_.Value(value);
}

Builder::DictKeyContext Builder::DictValueContext::Key(std::string key) {
    return builder_.Key(key);
}

Builder& Builder::DictValueContext::EndDict() {
    return builder_.EndDict();
}

/**
 * ArrayContext
 */

Builder::ArrayContext::ArrayContext(Builder& builder) : BaseContext(builder) {}

Builder::ArrayContext Builder::ArrayContext::Value(Node::Value value) {
    return ArrayContext(builder_.Value(value));
}

Builder& Builder::ArrayContext::EndArray() {
    return builder_.EndArray();
}

} // end namespace json