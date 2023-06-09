#pragma once

#include <functional>
#include <map>

#include "math.hpp"
#include "texture.hpp"

const size_t MAX_ATTRIBUTES_NUM = 4;

class Attributes {
   public:
    //    使用map可能会有性能问题，
    // 使用map，framework模式fps=3，非framework模式fps=1
    // std::map<unsigned int, float> varyingFloat;
    // std::map<unsigned int, Vec2> varyingVec2;
    // std::map<unsigned int, Vec3> varyingVec3;
    // std::map<unsigned int, Vec4> varyingVec4;
    std::array<float, MAX_ATTRIBUTES_NUM> varyingFloat;
    std::array<Vec2, MAX_ATTRIBUTES_NUM> varyingVec2;
    std::array<Vec3, MAX_ATTRIBUTES_NUM> varyingVec3;
    std::array<Vec4, MAX_ATTRIBUTES_NUM> varyingVec4;

    Attributes()
        : varyingFloat(std::array<float, MAX_ATTRIBUTES_NUM>()),
          varyingVec2(std::array<Vec2, MAX_ATTRIBUTES_NUM>()),
          varyingVec3(std::array<Vec3, MAX_ATTRIBUTES_NUM>()),
          varyingVec4(std::array<Vec4, MAX_ATTRIBUTES_NUM>()) {}
};

class Uniforms {
   public:
    std::map<unsigned int, int> varyingInt;
    std::map<unsigned int, float> varyingFloat;
    std::map<unsigned int, Vec2> varyingVec2;
    std::map<unsigned int, Vec3> varyingVec3;
    std::map<unsigned int, Vec4> varyingVec4;
    std::map<unsigned int, Mat44> varyingMat44;
    std::map<unsigned int, unsigned int> varyingTexuture;

    void clear() {
        varyingInt.clear();
        varyingFloat.clear();
        varyingVec2.clear();
        varyingVec3.clear();
        varyingVec4.clear();
        varyingMat44.clear();
        varyingTexuture.clear();
    }
};

class Vertex {
   public:
    Vec4 position;
    Attributes attributes;
    Vertex(Vec3 position, Attributes attributes)
        : position(Vec4::FromVec3(position, 1.0)), attributes(attributes) {}
    Vertex(Vec4 position, Attributes attributes)
        : position(position), attributes(attributes) {}
};

Attributes InterpAttributes(Attributes& attr1, Attributes& attr2,
                            std::function<float(float, float, float)> f,
                            float t);

void AttributesForeach(Attributes& attr, std::function<float(float)> f);

Vertex LerpVertex(Vertex& start, Vertex& end, float t) {
    auto position = start.position + (end.position - start.position) * t;
    auto attributes =
        InterpAttributes(start.attributes, end.attributes, Lerp<float>, t);
    return Vertex{position, attributes};
}

// 透视矫正需要用1/z
void VertexRhwInit(Vertex& vertex) {
    float rhw_z = 1.0 / vertex.position.z;
    vertex.position.z = rhw_z;
    AttributesForeach(vertex.attributes,
                      [=](float value) { return value * rhw_z; });
}

Attributes InterpAttributes(Attributes& attr1, Attributes& attr2,
                            std::function<float(float, float, float)> f,
                            float t) {
    Attributes attributes = Attributes();
    // for (const auto& [key, value] : attr1.varyingFloat) {
    //     attributes.varyingFloat[key] = f(value, attr2.varyingFloat[key], t);
    // }
    // for (const auto& [key, v1] : attr1.varyingVec2) {
    //     auto v2 = attr2.varyingVec2[key];
    //     attributes.varyingVec2[key] = Vec2{f(v1.x, v2.x, t), f(v1.y, v2.y,
    //     t)};
    // }
    // for (const auto& [key, v1] : attr1.varyingVec3) {
    //     auto v2 = attr2.varyingVec3[key];
    //     attributes.varyingVec3[key] =
    //         Vec3{f(v1.x, v2.x, t), f(v1.y, v2.y, t), f(v1.z, v2.z, t)};
    // }
    // for (const auto& [key, v1] : attr1.varyingVec4) {
    //     auto v2 = attr2.varyingVec4[key];
    //     attributes.varyingVec4[key] = Vec4{f(v1.x, v2.x, t), f(v1.y, v2.y,
    //     t),
    //                                        f(v1.z, v2.z, t), f(v1.w, v2.w,
    //                                        t)};
    // }

    // varyingFloat
    for (int index = 0; index < MAX_ATTRIBUTES_NUM; index++) {
        attributes.varyingFloat[index] =
            f(attr1.varyingFloat[index], attr2.varyingFloat[index], t);
    }

    // varyingVec2
    for (int index = 0; index < MAX_ATTRIBUTES_NUM; index++) {
        auto v1 = attr1.varyingVec2[index];
        auto v2 = attr2.varyingVec2[index];
        attributes.varyingVec2[index] =
            Vec2{f(v1.x, v2.x, t), f(v1.y, v2.y, t)};
    }

    // varyingVec3
    for (int index = 0; index < MAX_ATTRIBUTES_NUM; index++) {
        auto v1 = attr1.varyingVec3[index];
        auto v2 = attr2.varyingVec3[index];
        attributes.varyingVec3[index] =
            Vec3{f(v1.x, v2.x, t), f(v1.y, v2.y, t), f(v1.z, v2.z, t)};
    }

    // varyingVec4
    for (int index = 0; index < MAX_ATTRIBUTES_NUM; index++) {
        auto v1 = attr1.varyingVec4[index];
        auto v2 = attr2.varyingVec4[index];
        attributes.varyingVec4[index] =
            Vec4{f(v1.x, v2.x, t), f(v1.y, v2.y, t), f(v1.z, v2.z, t),
                 f(v1.w, v2.w, t)};
    }
    return attributes;
}

void AttributesForeach(Attributes& attr, std::function<float(float)> f) {
    // for (const auto& [key, value] : attr.varyingFloat) {
    //     attr.varyingFloat[key] = f(value);
    // }
    // for (const auto& [key, value] : attr.varyingVec2) {
    //     attr.varyingVec2[key] = Vec2{f(value.x), f(value.y)};
    // }
    // for (const auto& [key, value] : attr.varyingVec3) {
    //     attr.varyingVec3[key] = Vec3{f(value.x), f(value.y), f(value.z)};
    // }
    // for (const auto& [key, value] : attr.varyingVec4) {
    //     attr.varyingVec4[key] =
    //         Vec4{f(value.x), f(value.y), f(value.z), f(value.w)};
    // }

    // varyingFloat
    for (int index = 0; index < MAX_ATTRIBUTES_NUM; index++) {
        attr.varyingFloat[index] = f(attr.varyingFloat[index]);
    }

    // varyingVec2
    for (int index = 0; index < MAX_ATTRIBUTES_NUM; index++) {
        auto v = attr.varyingVec2[index];
        attr.varyingVec2[index] = Vec2{f(v.x), f(v.y)};
    }

    // varyingVec3
    for (int index = 0; index < MAX_ATTRIBUTES_NUM; index++) {
        auto v = attr.varyingVec3[index];
        attr.varyingVec3[index] = Vec3{f(v.x), f(v.y), f(v.z)};
    }

    // varyingVec4
    for (int index = 0; index < MAX_ATTRIBUTES_NUM; index++) {
        auto v = attr.varyingVec4[index];
        attr.varyingVec4[index] = Vec4{f(v.x), f(v.y), f(v.z), f(v.w)};
    }
}

using VertexChanging =
    std::function<Vertex(Vertex&, Uniforms&, TextureStorage&)>;
using PixelShading =
    std::function<Vec4(Attributes&, Uniforms&, TextureStorage&)>;

class Shader {
   public:
    VertexChanging vertexChanging;
    PixelShading pixelShading;

    Uniforms uniforms;

    Shader()
        : vertexChanging([](Vertex& vertex, Uniforms&, TextureStorage&) {
              return vertex;
          }),
          pixelShading([](Attributes&, Uniforms&, TextureStorage&) {
              return Vec4::Zero;
          }),
          uniforms(Uniforms()) {}

    Vertex CallVertexChanging(Vertex& vertex, Uniforms& uniforms,
                              TextureStorage& texture_storage) {
        return vertexChanging(vertex, uniforms, texture_storage);
    }

    Vec4 CallPixelShading(Attributes& attributes, Uniforms& uniforms,
                          TextureStorage& texture_storage) {
        return pixelShading(attributes, uniforms, texture_storage);
    }
};
