#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
    using namespace std::literals;

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
const std::string Null = "null"s;
const std::string True = "true"s;
const std::string False = "false"s;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : public std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    using Value = variant;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;    
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    const Value& GetValue() const;

    bool operator==(const Node& other) const;

    bool operator!=(const Node& other) const;

};

class Document {
public:
    Document(Node root);
    const Node& GetRoot() const;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

Document Load(std::istream& input);

// Контекст вывода, хранит ссылку на поток вывода и текущий отступ
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }
};

struct ValuePrinter {
    std::ostream& out;
    std::string tab_space;
    void operator()(std::nullptr_t);
    void operator()(std::string value);
    void operator()(int value);
    void operator()(double value);
    void operator()(bool value);
    void operator()(Array array);
    void operator()(Dict dict);
};

void Print(const Document& doc, std::ostream& out);

}  // namespace json