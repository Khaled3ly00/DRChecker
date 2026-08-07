#include "drcheck/geometry/Vector.h"
#include <cmath>

Vector::Vector(double x, double y)
    : x(x), y(y)
{
}

double Vector::getX() const
{
    return x;
}

double Vector::getY() const
{
    return y;
}

double Vector::length() const
{
    return std::sqrt(x * x + y * y);
}

double Vector::dot(const Vector& other) const
{
    return x * other.x + y * other.y;
}

double Vector::cross(const Vector& other) const
{
    return x * other.y - y * other.x;
}