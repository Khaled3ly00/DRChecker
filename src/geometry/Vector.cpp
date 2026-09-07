#include "drcheck/geometry/Vector.h"
#include "drcheck/geometry/Constants.h"

#include <cmath>
#include <stdexcept>

namespace drcheck::geometry {
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

    Vector Vector::normalized() const
    {
        const double vectorLength = length();

        if (vectorLength <= EPSILON) {
            throw std::logic_error("Cannot normalize a zero-length vector");
        }

        return Vector(x / vectorLength, y / vectorLength);
    }

    double Vector::dot(const Vector& other) const
    {
        return x * other.x + y * other.y;
    }

    double Vector::cross(const Vector& other) const
    {
        return x * other.y - y * other.x;
    }
}
