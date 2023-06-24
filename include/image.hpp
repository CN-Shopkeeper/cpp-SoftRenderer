#pragma once
#include <iostream>
#include <vector>

#include "math.hpp"

template <typename T>
class PureElementImage {
   public:
    std::vector<T> data;
    uint32_t width;
    uint32_t height;
    PureElementImage(std::vector<T> data, uint32_t width, uint32_t height)
        : data(data), width(width), height(height) {}
};

template <>
class PureElementImage<uint8_t> {
   public:
    std::vector<uint8_t> data;
    uint32_t width;
    uint32_t height;

    PureElementImage(std::vector<uint8_t> data, uint32_t width, uint32_t height)
        : data(data), width(width), height(height) {}

    PureElementImage(uint32_t w, uint32_t h) {
        PureElementImage<uint8_t>(std::vector<uint8_t>(w * h * 4, 0), w, h);
    }

    void Set(uint32_t x, uint32_t y, Vec4 color) {
        data[(x + y * width) * 4] = color.r * 255.0;
        data[(x + y * width) * 4 + 1] = color.g * 255.0;
        data[(x + y * width) * 4 + 2] = color.b * 255.0;
        data[(x + y * width) * 4 + 3] = color.a * 255.0;
    }

    void Clear(Vec4 color) {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                Set(i, j, color);
            }
        }
    }
};

template <>
class PureElementImage<float> {
   public:
    std::vector<float> data;
    uint32_t width;
    uint32_t height;

    PureElementImage(std::vector<float> data, uint32_t width, uint32_t height)
        : data(data), width(width), height(height) {}

    PureElementImage(uint32_t w, uint32_t h) {
        PureElementImage(std::vector<float>(w * h, FLT_MAX), w, h);
    }

    void Set(uint32_t x, uint32_t y, float value) {
        data[x + y * width] = value;
    }

    float Get(uint32_t x, uint32_t y) { return data[x + y * width]; }

    void Clear(float value) {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                Set(i, j, value);
            }
        }
    }
};

typedef PureElementImage<uint8_t> ColorAttachment;
typedef PureElementImage<float> DepthAttachment;
