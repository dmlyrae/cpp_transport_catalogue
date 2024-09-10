#pragma once

#include <optional>
#include "json.h"

namespace json {

using namespace std::literals;

class DictValueContext;
class DictKeyContext;
class ArrayContext;

class Builder {
public:

    Builder();

    class BaseContext;
    class DictKeyContext;
    class DictValueContext;
    class ArrayContext;

    Builder& Value(Node::Value value);
    DictValueContext StartDict();
    ArrayContext StartArray();
    Node Build();
    Node GetNode(Node::Value value);

private:

    Node root_{ nullptr };
    std::vector<Node*> nodes_stack_;

    DictKeyContext Key(std::string key);
    Builder& EndDict();
    Builder& EndArray();

    Node::Value& GetCurrentValue();
    const Node::Value& GetCurrentValue() const;
    void Add(Node::Value value, bool one_shot);
};

class Builder::BaseContext {
public:
    BaseContext(Builder& builder);

    DictValueContext StartDict();
    ArrayContext StartArray();

protected:
    Builder& builder_;
};

class Builder::DictKeyContext : public Builder::BaseContext {
public:
    DictKeyContext(Builder& builder);
    DictValueContext Value(Node::Value value);
    DictValueContext StartDict() = delete;
    ArrayContext StartArray() = delete;
};

class Builder::DictValueContext : public Builder::BaseContext {
public:
    DictValueContext(Builder& builder);
    DictKeyContext Key(std::string key);
    Builder& EndDict();
};

class Builder::ArrayContext : public Builder::BaseContext {
public:
    ArrayContext(Builder& builder);

    Builder::ArrayContext Value(Node::Value value);
    Builder& EndArray();
};

} // namespace json