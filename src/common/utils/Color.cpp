#include <cassert>

#include "common/math/math.hpp"
#include "common/utils/Color.hpp"

const Color Color::Black = Color::fromRGBA32(0, 0, 0);
const Color Color::White = Color::fromRGBA32(255, 255, 255);
const Color Color::Red = Color::fromRGBA32(255, 0, 0);
const Color Color::Green = Color::fromRGBA32(0, 255, 0);
const Color Color::Blue = Color::fromRGBA32(0, 0, 255);
const Color Color::Yellow = Color::fromRGBA32(255, 255, 0);
const Color Color::Magenta = Color::fromRGBA32(255, 0, 255);
const Color Color::Cyan = Color::fromRGBA32(0, 255, 255);
const Color Color::Transparent = Color::fromRGBA32(0, 0, 0, 0);

const Color Color::TextBlue = Color::fromRGBA32(32, 168, 248);

Color::Color(float r, float g, float b, float a)
{
    assert(r <= 1 && g <= 1 && b <= 1 && a <= 1);

    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

Color Color::mix(const Color &other, float ratio)
{
    Color c;
    c.r = math::lerpf(r, other.r, ratio);
    c.g = math::lerpf(g, other.g, ratio);
    c.b = math::lerpf(b, other.b, ratio);
    c.a = math::lerpf(a, other.a, ratio);
    return c;
}

Color Color::fromRGBA32(u8 r, u8 g, u8 b, u8 a)
{
    return Color{
        (float)r / 255.0f,
        (float)g / 255.0f,
        (float)b / 255.0f,
        (float)a / 255.0f,
    };
}
