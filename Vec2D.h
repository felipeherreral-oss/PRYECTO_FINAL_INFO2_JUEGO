#pragma once
#include <cmath>

/**
 * @brief Vector 2D para posiciones, velocidades y fuerzas en el juego.
 *        Usado por el motor de físicas y todos los GameEntity.
 */
struct Vec2D {
    float x, y;

    Vec2D(float x = 0.f, float y = 0.f) : x(x), y(y) {}

    Vec2D operator+(const Vec2D& o) const { return {x + o.x, y + o.y}; }
    Vec2D operator-(const Vec2D& o) const { return {x - o.x, y - o.y}; }
    Vec2D operator*(float s)        const { return {x * s,   y * s};   }
    Vec2D operator/(float s)        const { return {x / s,   y / s};   }
    Vec2D& operator+=(const Vec2D& o) { x += o.x; y += o.y; return *this; }
    Vec2D& operator-=(const Vec2D& o) { x -= o.x; y -= o.y; return *this; }
    Vec2D& operator*=(float s)        { x *= s;   y *= s;   return *this; }

    float length()    const { return std::sqrt(x*x + y*y); }
    float lengthSq()  const { return x*x + y*y; }
    float dot(const Vec2D& o) const { return x*o.x + y*o.y; }

    Vec2D normalized() const {
        float len = length();
        if (len < 1e-6f) return {0.f, 0.f};
        return {x / len, y / len};
    }

    float distanceTo(const Vec2D& o) const {
        return (*this - o).length();
    }

    // Perpendicular (rotado 90°)
    Vec2D perp() const { return {-y, x}; }

    static Vec2D zero() { return {0.f, 0.f}; }
};
