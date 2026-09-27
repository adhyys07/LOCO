#pragma once
#include <cmath>

struct Vector2 {
    float x = 0.0f, y = 0.0f;
    Vector2() = default;
    Vector2(float x_, float y_) : x(x_), y(y_) {}

    Vector2 operator+(const Vector2& o) const { return {x + o.x, y + o.y}; }
    Vector2 operator-(const Vector2& o) const { return {x - o.x, y - o.y}; }
    Vector2 operator*(float s) const { return {x * s, y * s}; }
    Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }

    float length() const { return std::sqrt(x * x + y * y); }
    Vector2 normalized() const {
        float l = length();
        return l > 0.0f ? Vector2(x / l, y / l) : Vector2();
    }
};

struct Transform2D {
    Vector2 col_x{1, 0};
    Vector2 col_y{0, 1};
    Vector2 origin{0, 0};

    static Transform2D from_trs(const Vector2& pos, float rot, const Vector2& scale) {
        Transform2D t;
        float c = std::cos(rot), s = std::sin(rot);
        t.col_x = Vector2(c * scale.x,  s * scale.x);
        t.col_y = Vector2(-s * scale.y, c * scale.y);
        t.origin = pos;
        return t;
    }

    Vector2 xform(const Vector2& v) const {
        return Vector2(col_x.x * v.x + col_y.x * v.y + origin.x,
                       col_x.y * v.x + col_y.y * v.y + origin.y);
    }

    Transform2D operator*(const Transform2D& o) const {
        Transform2D r;
        r.col_x  = Vector2(col_x.x * o.col_x.x + col_y.x * o.col_x.y,
                           col_x.y * o.col_x.x + col_y.y * o.col_x.y);
        r.col_y  = Vector2(col_x.x * o.col_y.x + col_y.x * o.col_y.y,
                           col_x.y * o.col_y.x + col_y.y * o.col_y.y);
        r.origin = xform(o.origin);
        return r;
    }
};

struct Rect2 {
    Vector2 position, size;
    Rect2() = default;
    Rect2(const Vector2& p, const Vector2& s) : position(p), size(s) {}
    bool intersects(const Rect2& o) const {
        return position.x < o.position.x + o.size.x &&
               position.x + size.x > o.position.x &&
               position.y < o.position.y + o.size.y &&
               position.y + size.y > o.position.y;
    }
};

struct Color {
    float r = 1, g = 1, b = 1, a = 1;
    Color() = default;
    Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
};