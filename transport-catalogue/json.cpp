#include "json.h"
#include <variant>
#include <string>

using namespace std;

namespace json {

const std::string TabSpace = "    "s;

bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
};

bool Node::IsDouble() const {
    return std::holds_alternative<int>(*this) || std::holds_alternative<double>(*this);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this) && !std::holds_alternative<int>(*this);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
};

bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
};

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
};

bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
};

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}

const Node::Value& Node::GetValue() const {
    return *this;
}

const Array& Node::AsArray() const {
    if (IsArray()) {
        return std::get<Array>(*this);
    }
    throw  std::logic_error("Not Array");
}

const Dict& Node::AsMap() const {
    if (IsMap()) {
        return std::get<Dict>(*this);
    }
    throw  std::logic_error("Not Dict");
}

int Node::AsInt() const {
    if (IsInt()) {
        return std::get<int>(*this);
    }
    throw  std::logic_error("Not int");
}

const string& Node::AsString() const {
    if (IsString()) {
        return std::get<string>(*this);
    }
    throw  std::logic_error("Not string");
}

bool Node::AsBool() const {
    if (IsBool()) {
        return std::get<bool>(*this);
    }
    throw std::logic_error("Not bool");
}

double Node::AsDouble() const {
    if (std::holds_alternative<double>(*this)) {
        return std::get<double>(*this);
    }
    if (std::holds_alternative<int>(*this)) {
        return static_cast<double>(std::get<int>(*this));
    }
    throw std::logic_error("Not double");
}

bool Node::operator==(const Node& other) const {
    return GetValue() == other.GetValue();
}

bool Node::operator!=(const Node& other) const {
    return !(*this == other);
}

namespace {

Node LoadNode(istream& input);

Node LoadNumber(istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));//std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNull(istream& input) {
    for (size_t i = 0; i < json::Null.size(); i++) {
        if (json::Null.at(i) != input.get()) {
            throw ParsingError(":)");
        }
    }
    return Node(nullptr);
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();

    std::string s;

    ++it;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    // std::cout << "result: " << s << std::endl;

    return s;
}

Node LoadArray(istream& input) {
    Array result;

    input.get();

    if (input.peek() == -1) {
        throw ParsingError("Array parsing error");
    }

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;

    input.get();

    if (input.peek() == -1) {
        throw json::ParsingError("");
    }

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        input.putback(c);
        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    return Node(move(result));
}

Node LoadBool(istream& input, bool type) {
    const string& bool_name = type ? json::True : json::False;
    for (size_t i = 0; i < bool_name.size(); i++) {
        if (bool_name.at(i) != input.get()) {
            throw ParsingError("Bool parsing error");
        }
    }
    return Node(type);
}

Node LoadNode(istream& input) {
    char c;
    input >> c;
    input.putback(c);
    if (c == 'n') {
        return LoadNull(input);
    } else if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 't') {
        return LoadBool(input, true);
    } else if (c == 'f') {
        return LoadBool(input, false);
    }
    return LoadNumber(input);
}

}  // namespace


Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    Document d = Document{LoadNode(input)};
    char c = input.peek();
    if (c > ' ') {
        throw json::ParsingError("");
    }
    return d;
}
    
bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}

bool Document::operator!=(const Document& other) const {
    return !(root_ == other.root_);
}

void ValuePrinter::operator()(std::nullptr_t) {
    out << "null"sv;
}

void ValuePrinter::operator()(std::string value) {
    std::string value_printer_result = "\"";
    for (const char& c : value) {
        if (c == '\n') {
            value_printer_result += "\\n";
            continue;
        }
        if (c == '\r') {
            value_printer_result += "\\r";
            continue;
        }
        if (c == '\"') {
            value_printer_result += "\\";
        }
        if (c == '\t') {
            value_printer_result += "\\t";
            continue;
        }
        if (c == '\\') {
            value_printer_result += "\\";
        }
        value_printer_result += c;
    }
    value_printer_result += "\"";
    out << value_printer_result;
}

void ValuePrinter::operator()(int value) {
    out << value;
}

void ValuePrinter::operator()(double value) {
    out << value;
}

void ValuePrinter::operator()(bool value) {
    out << std::boolalpha << value;
}

void ValuePrinter::operator()(Array array) {
    out << "[\n"sv;
    bool first = true;
    for (const auto& elem : array) {
        if (first) { 
            out << tab_space << TabSpace;
            first = false;
        } else {
            out << ",\n"s;
            out << tab_space << TabSpace;
        }
        std::visit(ValuePrinter{ out, tab_space + TabSpace }, elem.GetValue());
    }
    out << "\n"sv << tab_space << "]"sv;
}

void ValuePrinter::operator()(Dict dict) {
    out << "{\n"sv;
    bool first = true;
    for (auto& [key, node] : dict) {
        if (first) {
            first = false;
        } else {
            out << ", \n"s;
        }
        out << tab_space << TabSpace;
        out << "\"" << key << "\": ";
        std::visit(ValuePrinter{ out, tab_space + TabSpace }, node.GetValue());
    }
    out << "\n"s <<tab_space << "}"sv;
}

void Print(const Document& doc, std::ostream& out) {
    std::visit(ValuePrinter{ out, ""s }, doc.GetRoot().GetValue());
}

}  // namespace json