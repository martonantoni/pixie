#pragma once

struct cPixieObject::cPropertyValues
{
    std::variant<std::monostate, int, float, cPoint, cRect, cColor> mValue;

    cPropertyValues() : mValue(std::monostate{}) {}
    cPropertyValues(int value) : mValue(value) {}
    cPropertyValues(unsigned int value) : mValue(static_cast<int>(value)) {}
    cPropertyValues(float value) : mValue(value) {}
    cPropertyValues(const cPoint& point) : mValue(point) {}
    cPropertyValues(const cRect& rect) : mValue(rect) {}
    cPropertyValues(const cColor& color) : mValue(color) {}
    const cPropertyValues& operator=(int value) { mValue = value; return *this; }
    const cPropertyValues& operator=(const cPropertyValues& other) { mValue = other.mValue; return *this; }
    const cPropertyValues& operator=(unsigned int value) { mValue = static_cast<int>(value); return *this; }
    const cPropertyValues& operator=(float value) { mValue = value; return *this; }
    const cPropertyValues& operator=(const cPoint& point) { mValue = point; return *this; }
    const cPropertyValues& operator=(const cRect& rect) { mValue = rect; return *this; }
    const cPropertyValues& operator=(const cColor& color) { mValue = color; return *this; }
    bool operator==(const cPropertyValues& other) const { return mValue == other.mValue; }
    float ToFloat() const { return std::get<float>(mValue); }
    int ToInt() const { return std::get<int>(mValue); }
    cPoint ToPoint() const { return std::get<cPoint>(mValue); }
    cRect ToRect() const { return std::get<cRect>(mValue); }
    uint32_t ToRGBColor() const { return std::get<cColor>(mValue).GetRGBColor(); }

    static cPropertyValues lerp(const cPropertyValues& a, const cPropertyValues& b, float t);
};