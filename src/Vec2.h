#pragma once

class Vec2
{
public:

    float x = 0;
    float y = 0;

    Vec2();
    Vec2(float xin, float yin);
 
    bool operator == (const Vec2 & rhs) const;
    bool operator != (const Vec2 & rhs) const;

    Vec2 operator + (const Vec2 & rhs) const;
    Vec2 operator - (const Vec2 & rhs) const;
    Vec2 operator / (const float & val) const;
    Vec2 operator * (const float & val) const;
    float operator * (const Vec2 & rhs) const;

    
    bool operator < (const Vec2 & v2);
    void operator += (const Vec2 & rhs);
    void operator -= (const Vec2 & rhs);
    void operator *= (const float & val);
    void operator /= (const float & val);

    float clockwise_angle (const Vec2 & Vec) const;
    Vec2 norm() const;
    Vec2 abs() const;
    float length() const;

    float dist(const Vec2 & rhs) const;
};