#ifndef COLOR_HPP_
#define COLOR_HPP_

#include "common/utils/IntTypes.hpp"

class Color
{
  public:
    Color() = default;
    Color(float r, float g, float b, float a = 1);

    Color mix(const Color &other, float ratio);

    Color operator+(const Color &c)
    {
        return Color(r + c.r, g + c.g, b + c.b, a + c.a);
    }
    Color operator-(const Color &c)
    {
        return Color(r - c.r, g - c.g, b - c.b, a - c.a);
    }

    bool operator==(const Color &color) const
    {
        return r == color.r && g == color.g && b == color.b && a == color.a;
    }

    bool operator!=(const Color &color) const
    {
        return !operator==(color);
    }

    static Color fromRGBA32(u8 r, u8 g, u8 b, u8 a = 255);

    // Only used in Asylia
    u8 r255() const
    {
        return u8(r * 255.f);
    }
    u8 g255() const
    {
        return u8(g * 255.f);
    }
    u8 b255() const
    {
        return u8(b * 255.f);
    }
    u8 a255() const
    {
        return u8(a * 255.f);
    }

    // FIXME: Use u8 instead of float and normalize when sending to OpenGL
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    static const Color Transparent;

    static const Color TextBlue;
};

#endif // COLOR_HPP_
