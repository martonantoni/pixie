#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cColorServer theColorServer;

void cColorServer::Init()
{
	theGlobalConfig->createSubConfig("colors")->forEachInt([&](const std::string& name, int color)
    {
        mColorMap[name] = cColor(color);
    });
}

cColor cColor::toGrayscale() const
{
    int gray = (0.2126f * GetRed() + 0.7152f * GetGreen() + 0.0722f * GetBlue());
    return cColor(gray, gray, gray);
}

cColor cColor::adjustedRGB(double amount) const
{
    amount = std::clamp(amount, -1.0, 1.0);
    cColor newColor = *this;
    if (amount > 0.0)
    {
        newColor.red() = red() + static_cast<int>(std::round((255 - red()) * amount));
        newColor.green() = green() + static_cast<int>(std::round((255 - green()) * amount));
        newColor.blue() = blue() + static_cast<int>(std::round((255 - blue()) * amount));
    }
    else
    {
        newColor.red() = red() + static_cast<int>(std::round(red() * amount));
        newColor.green() = green() + static_cast<int>(std::round(green() * amount));
        newColor.blue() = blue() + static_cast<int>(std::round(blue() * amount));
    }
    return newColor;
}

// HSV / HSL adjustments written by ChatGPT, based on standard color conversion formulas

cColor cColor::adjustedHSV(double amount) const
{
    amount = std::clamp(amount, -1.0, 1.0);

    double r = red() / 255.0;
    double g = green() / 255.0;
    double b = blue() / 255.0;

    double maxv = std::max({ r, g, b });
    double minv = std::min({ r, g, b });
    double delta = maxv - minv;

    double h = 0.0;
    double s = (maxv == 0.0) ? 0.0 : delta / maxv;
    double v = maxv;

    if (delta != 0.0)
    {
        if (maxv == r)
            h = 60.0 * std::fmod((g - b) / delta, 6.0);
        else if (maxv == g)
            h = 60.0 * (((b - r) / delta) + 2.0);
        else
            h = 60.0 * (((r - g) / delta) + 4.0);

        if (h < 0.0)
            h += 360.0;
    }

    // Adjust value/lightness
    if (amount >= 0.0)
        v += (1.0 - v) * amount;
    else
        v += v * amount;

    // HSV -> RGB

    double c = v * s;
    double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
    double m = v - c;

    double rr = 0.0;
    double gg = 0.0;
    double bb = 0.0;

    if (h < 60.0) { rr = c; gg = x; bb = 0.0; }
    else if (h < 120.0) { rr = x; gg = c; bb = 0.0; }
    else if (h < 180.0) { rr = 0.0; gg = c; bb = x; }
    else if (h < 240.0) { rr = 0.0; gg = x; bb = c; }
    else if (h < 300.0) { rr = x; gg = 0.0; bb = c; }
    else { rr = c; gg = 0.0; bb = x; }

    cColor newColor = *this;

    newColor.red() =
        static_cast<int>(std::round((rr + m) * 255.0));

    newColor.green() =
        static_cast<int>(std::round((gg + m) * 255.0));

    newColor.blue() =
        static_cast<int>(std::round((bb + m) * 255.0));

    return newColor;
}

cColor cColor::adjustedHSL(double amount) const
{
    amount = std::clamp(amount, -1.0, 1.0);

    double r = red() / 255.0;
    double g = green() / 255.0;
    double b = blue() / 255.0;

    double maxv = std::max({ r, g, b });
    double minv = std::min({ r, g, b });
    double delta = maxv - minv;

    double h = 0.0;
    double l = (maxv + minv) * 0.5;
    double s = 0.0;

    if (delta != 0.0)
    {
        s = delta / (1.0 - std::abs(2.0 * l - 1.0));

        if (maxv == r)
            h = 60.0 * std::fmod((g - b) / delta, 6.0);
        else if (maxv == g)
            h = 60.0 * (((b - r) / delta) + 2.0);
        else
            h = 60.0 * (((r - g) / delta) + 4.0);

        if (h < 0.0)
            h += 360.0;
    }

    if (amount >= 0.0)
        l += (1.0 - l) * amount;
    else
        l += l * amount;

    double c = (1.0 - std::abs(2.0 * l - 1.0)) * s;
    double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
    double m = l - c * 0.5;

    double rr = 0.0;
    double gg = 0.0;
    double bb = 0.0;

    if (h < 60.0) { rr = c;   gg = x;   bb = 0.0; }
    else if (h < 120.0) { rr = x;   gg = c;   bb = 0.0; }
    else if (h < 180.0) { rr = 0.0; gg = c;   bb = x; }
    else if (h < 240.0) { rr = 0.0; gg = x;   bb = c; }
    else if (h < 300.0) { rr = x;   gg = 0.0; bb = c; }
    else { rr = c;   gg = 0.0; bb = x; }

    cColor newColor = *this;

    newColor.red() =
        static_cast<int>(std::round((rr + m) * 255.0));

    newColor.green() =
        static_cast<int>(std::round((gg + m) * 255.0));

    newColor.blue() =
        static_cast<int>(std::round((bb + m) * 255.0));

    return newColor;
}
