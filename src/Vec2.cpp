#include "Vec2.h"
#include <math.h>
#include <string>
#include <sstream>

Vec2::Vec2()
{

}

Vec2::Vec2(float xin, float yin)
	: x(xin)
	, y(yin)
{

}

Vec2 Vec2::operator + (const Vec2 & rhs) const
{
	return Vec2(x + rhs.x, y + rhs.y);
}

Vec2 Vec2::operator - (const Vec2 & rhs) const
{
	return Vec2(x - rhs.x, y - rhs.y);
}

Vec2 Vec2::operator / (const float & val) const
{
	return Vec2(x / val, y / val);
}

Vec2 Vec2::operator * (const float & val) const
{
	return Vec2(x * val, y * val);
}

bool Vec2::operator == (const Vec2 & rhs) const
{
	return x == rhs.x && y == rhs.y;
}

bool Vec2::operator != (const Vec2 & rhs) const
{
	return !(*this == rhs);
}

void Vec2::operator += (const Vec2 & rhs)
{
	x += rhs.x;
	y += rhs.y;
}

bool Vec2::operator < (const Vec2 & v2)
{
	return clockwise_angle(Vec2(x,y).norm()) < clockwise_angle(v2.norm());
}

void Vec2::operator -= (const Vec2 & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
}

void Vec2::operator *= (const float & val)
{
	x *= val;
	y *= val;
}

void Vec2::operator /= (const float & val)
{
	x /= val;
	y /= val;
}

float Vec2::operator * (const Vec2 & rhs) const
{
	return x * rhs.y - rhs.x * y;
}

float Vec2::dist(const Vec2 & rhs) const
{
	return sqrt((x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y));
}


float Vec2::fastDist(const Vec2 & rhs) const
{
	return (x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y);
}
 
Vec2 Vec2::norm() const
{
	float len = this->length();
	float x = this->x / len;
	float y = this->y / len;
    return Vec2(x, y);
}



float Vec2::length() const
{
	return sqrt(this->x * this->x + this->y * this->y);
};

Vec2 Vec2::abs() const
{
	return Vec2(x < 0 ? -x : x, y < 0 ? -y : y);
}

float Vec2::clockwise_angle(const Vec2 & vec) const
{
    float angle = 0.0;
    angle = atan2(vec.x, vec.y);
    return angle;
}

