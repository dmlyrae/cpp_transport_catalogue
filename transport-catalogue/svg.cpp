#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, Rgb rgb) {
    out << "rgb("sv;
    out << static_cast<int>(rgb.red) << ","sv;
    out << static_cast<int>(rgb.green) << ","sv;
    out << static_cast<int>(rgb.blue) << ")"sv;
    return out;
}

std::ostream& operator<<(std::ostream& out, Rgba rgba) {
    out << "rgb("sv;
    out << static_cast<int>(rgba.red) << ","sv;
    out << static_cast<int>(rgba.green) << ","sv;
    out << static_cast<int>(rgba.blue) << ","sv;
    out << rgba.opacity << ")"sv;
    return out;
}

std::ostream& operator<<(std::ostream& out, Color& color) {
    std::visit(ColorOutput{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    out << (line_cap == StrokeLineCap::BUTT ? "butt"sv : (line_cap == StrokeLineCap::ROUND ? "round"sv : "square"));
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    out << (line_join == StrokeLineJoin::ARCS ? "arcs"sv : (line_join == StrokeLineJoin::BEVEL ? "bevel"sv 
        : (line_join == StrokeLineJoin::MITER ? "miter"sv : (line_join == StrokeLineJoin::MITER_CLIP ? "miter-clip" : "round"))));
    return out;
}

std::string escapeSpecialCharacters(const std::string& text) {
    std::string result;

    for (char c : text) {
        switch (c) {
            case '"':
                result += "&quot;";
                break;
            case '\'':
                result += "&apos;";
                break;
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            case '&':
                result += "&amp;";
                break;
            default:
                result += c;
        }
    }

    return result;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- ObjectContainter ------------------

// template <typename T>
// void ObjectContainer::Add(T obj) {
//     objects_.emplace_back(std::make_unique(std::move(obj)));
// }

// ---------- Drawable ------------------

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    std::ostream& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (Point p : points_) {
        if (is_first) {
            out << p.x << ',' << p.y;
            is_first = false;
        } else {
            out << ' ' << p.x << ',' << p.y;
        }
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv; 
}
// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }
    
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) out << " font-family=\""sv << font_family_ << "\" "sv;
        if (!font_weight_.empty()) out << "font-weight=\""sv << font_weight_ << "\""sv;
        out << ">"sv << data_ << "</text>"sv;
    }


// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& output) const {
    RenderContext ctx{output, 2, 2};
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    output << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    output << "</svg>"sv;
}

}  // namespace svg
