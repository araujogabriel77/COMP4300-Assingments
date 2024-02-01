#include <cmath>
#include "Vec2.h"

Vec2::Vec2(float xin, float yin)
    : x(xin), y(yin)
{
}

bool Vec2::operator==(const Vec2& rhs) const
{
    return (x == rhs.x && y == rhs.y);
}

bool Vec2::operator!=(const Vec2& rhs) const
{
    return (x != rhs.x || y != rhs.y);
}

Vec2 Vec2::operator+(const Vec2& rhs) const
{
    return Vec2(x + rhs.x, y + rhs.y);
}

Vec2 Vec2::operator-(const Vec2& rhs) const
{
    return Vec2(x - rhs.x, y - rhs.y);
}

Vec2 Vec2::operator/(const float val) const
{
    return Vec2(x / val, y / val);
}

Vec2 Vec2::operator*(const float val) const
{
    return Vec2(x * val, y * val);
}

void Vec2::operator+=(const Vec2& rhs)
{
    x += rhs.x;
    y += rhs.y;
}

void Vec2::operator-=(const Vec2& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
}

void Vec2::operator*=(const float val)
{
    x *= val;
    y *= val;
}

void Vec2::operator/=(const float val)
{
    x /= val;
    y /= val;
}

float Vec2::dist(const Vec2& rhs) const
{
    return sqrtf((rhs.x - x) * (rhs.x - x) + (rhs.y - y) * (rhs.y - y));
}

void Vec2::normalize()
{
  double L{std::sqrt(x * x + y * y)};
  x = x / L;
  y = y / L;
}

Vec2 Vec2::normalize(Vec2 vector)
{
  double L{std::sqrt(vector.x * vector.x + vector.y * vector.y)};
  return Vec2(vector.x / L, vector.x / L);
}