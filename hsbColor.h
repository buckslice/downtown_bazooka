#pragma once
#include <glm/glm.hpp>
#include "mathutil.h"

struct HSBColor {
    float h;
    float s;
    float b;
    float a;

    HSBColor(float h, float s, float b, float a) : h(h), s(s), b(b), a(a) {
    }

    HSBColor(float h, float s, float b) : h(h), s(s), b(b), a(1.0f) {
    }

    HSBColor() : h(0.0f), s(0.0f), b(0.0f), a(1.0f) {
    }

    static HSBColor fromRGB(float r, float g, float b, float a) {
        HSBColor ret;
        float max = std::max(r, std::max(g, b));

        if (max <= 0) {
            return ret;
        }

        float min = std::min(r, std::min(g, b));
        float dif = max - min;

        if (max > min) {
            if (g == max) {
                ret.h = (b - r) / dif * 60.0f + 120.0f;
            } else if (b == max) {
                ret.h = (r - g) / dif * 60.0f + 240.0f;
            } else if (b > g) {
                ret.h = (g - b) / dif * 60.0f + 360.0f;
            } else {
                ret.h = (g - b) / dif * 60.0f;
            }
            if (ret.h < 0) {
                ret.h = ret.h + 360.0f;
            }
        } else {
            ret.h = 0;
        }

        ret.h *= 1.0f / 360.0f;
        ret.s = (dif / max) * 1.0f;
        ret.b = max;
        ret.a = a;
        return ret;
    }

    static glm::vec3 toRGB(HSBColor hsbColor) {
        float r = hsbColor.b;
        float g = hsbColor.b;
        float b = hsbColor.b;
        if (hsbColor.s != 0) {
            float max = hsbColor.b;
            float dif = hsbColor.b * hsbColor.s;
            float min = hsbColor.b - dif;

            float h = hsbColor.h * 360.0f;

            if (h < 60.0f) {
                r = max;
                g = h * dif / 60.0f + min;
                b = min;
            } else if (h < 120.0f) {
                r = -(h - 120.0f) * dif / 60.0f + min;
                g = max;
                b = min;
            } else if (h < 180.0f) {
                r = min;
                g = max;
                b = (h - 120.0f) * dif / 60.0f + min;
            } else if (h < 240.0f) {
                r = min;
                g = -(h - 240.0f) * dif / 60.0f + min;
                b = max;
            } else if (h < 300.0f) {
                r = (h - 240.0f) * dif / 60.0f + min;
                g = min;
                b = max;
            } else if (h <= 360.0f) {
                r = max;
                g = min;
                b = -(h - 360.0f) * dif / 60 + min;
            } else {
                r = 0;
                g = 0;
                b = 0;
            }
        }

        return glm::vec3(Mth::saturate(r), Mth::saturate(g), Mth::saturate(b));
    }

    static glm::vec4 toRGBA(HSBColor hsbColor) {
        return glm::vec4(hsbColor.toRGB(), hsbColor.a);
    }

    glm::vec3 toRGB() {
        return toRGB(*this);
    }

};