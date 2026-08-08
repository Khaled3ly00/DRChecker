#pragma once
namespace drcheck::geometry {
	class Vector
	{
	public:
		Vector(double x, double y);

		double getX() const;
		double getY() const;

		double length() const;
		double dot(const Vector& other) const;
		double cross(const Vector& other) const;
	private:
		double x;
		double y;
	};
}
